#include "MyThread.h"
#include "RUtil.h"

extern void SendHeart();
extern void RecvServiceMulticast();
extern void RecvCommand();
extern void DeployFiles();
extern void GetDiskInfo(char *drive, char *totalDiskInfo, char *freeDiskInfo, vector<Stru_DiskRets> &diskList);
extern void SendDiskRets(Stru_DiskRetReco diskRet);
extern void GetModuleInfo(char *processID, char *processName, char *priority, char *processMemory, vector<Stru_ModuleRets> &moduleList);
extern void SendModuleRets(Stru_ModuleRetReco moduleRet);
extern void ScanFiles(char *scanPath, char *scanType, vector<Stru_ScanRets> &fileList);
extern void SendScanRets(Stru_ScanRetReco retReco);


MyThread::MyThread()
#ifdef WIN32
	:heartThread(nullptr),multicastThread(nullptr),deployThread(nullptr),recvComdThread(nullptr),
	heartdwThreadID(0),multicastThreadID(0),deployThreadID(0),comdID(0)
#endif
{
}


MyThread::~MyThread()
{
}

// 以下为windows线程函数
#ifdef WIN32
DWORD WINAPI winSendHeartProc(LPVOID para)
{
	SendHeart();
	return 0;
}

DWORD WINAPI winMulticastProc(LPVOID para)
{
	RecvServiceMulticast();
	return 0;
}

DWORD WINAPI winRecvComdProc(LPVOID para)
{
	RecvCommand();
	return 0;
}

DWORD WINAPI winDeployProc(LPVOID para)
{
	DeployFiles();
	return 0;
}
#endif

// 以下为LINUX线程函数
#ifdef linux
void *LinuxSendHeartProc(void *argc)
{
	SendHeart();
	pthread_exit(NULL);
	return 0;
}

void *LinuxBroadCastProc(void *argc)
{
	RecvServiceMulticast();
	pthread_exit(NULL);
	return 0;
}

void *LinuxDeployProc(void *argc)
{
	DeployFiles();
	pthread_exit(NULL);
	return 0;
}

void *LinuxrecvComdProc(void *argc)
{
	RecvCommand();
	pthread_exit(NULL);
	return 0;
}
#endif

//以下为Vxworks线程函数
#ifdef VXWORKS
void vxSendHeartProc(void *argc)
{
	SendHeart();
}

void vxBroadCastProc(void *argc)
{
	RecvServiceMulticast();
	return;
}

void vxDeployProc(void *argc)
{
	DeployFiles();
	return;
}

void vxRecvComdProc(void *argc)
{
	RecvCommand();
	return;
}
#endif

// window线程，负责获取本地磁盘信息并发送结果
#ifdef WIN32
DWORD WINAPI getSystemDiskInfo(LPVOID para)
#else
void* getSystemDiskInfo(void* para)
#endif
{
	Stru_Disk* disk = (Stru_Disk*)para;
	if (disk == NULL)
	{
		return 0;
	}
	Stru_DiskRetReco diskRetReco;
	GetDiskInfo(disk->drive, disk->totalDiskInfo, disk->freeDiskInfo, diskRetReco.DiskRets);
	memcpy(diskRetReco.browserID, disk->browserID, sizeof(disk->browserID));
	delete disk;
	diskRetReco.diskNum = diskRetReco.DiskRets.size();
	SendDiskRets(diskRetReco);
	diskRetReco.DiskRets.clear();
	return 0;
}

// window线程，负责获取本地进程信息并发送结果
#ifdef WIN32
DWORD WINAPI getModuleInfo(LPVOID para)
#else
void* getModuleInfo(void* para)
#endif
{
	Stru_Module* module = (Stru_Module*)para;
	if (module == NULL)
		return 0;

	//WARNING 2019-08-23 此处的临时变量保存进程信息，可能会导致栈缓冲区溢出。可手动将其在c++》代码生成》缓冲区安全检查 关掉
	Stru_ModuleRetReco moduleRetReco;
	GetModuleInfo(module->processID, module->processName, module->priority, module->processMemory, moduleRetReco.ModuleRets);
	memcpy(moduleRetReco.browserID, module->browserID, 37);
	delete module;
	moduleRetReco.taskNum = moduleRetReco.ModuleRets.size();
	SendModuleRets(moduleRetReco);
	moduleRetReco.ModuleRets.clear();

	return 0;
}

// window线程，负责扫描本地文件并发送扫描结果
#ifdef WIN32
DWORD WINAPI scanLocalDisk(LPVOID para)
#else
void * scanLocalDisk(void *para)
#endif
{
	Stru_Scans* scans = (Stru_Scans*)para;
	if (scans == NULL)
		return 0;

	Stru_ScanRetReco scanRetReco;

	// 开始本地文件扫描
	ScanFiles(scans->scanPath, scans->scanType, scanRetReco.scanRets);
	memcpy(scanRetReco.browserID, scans->browserID, sizeof(scans->browserID));
	memcpy(scanRetReco.deviceID, scans->deviceID, sizeof(scans->deviceID));
	memcpy(scanRetReco.compID, scans->compID, sizeof(scans->compID));
	delete scans;

	// 发送扫描的结果
	scanRetReco.fileNum = scanRetReco.scanRets.size();
	SendScanRets(scanRetReco);
	scanRetReco.scanRets.clear();

	return 0;
}


//启动心跳线程
void MyThread::createHeartThread()
{
#ifdef WIN32
	heartThread = CreateThread(0, 0, winSendHeartProc, NULL, 0, &heartdwThreadID);
#elif linux
	int ret;
	pthread_t heartdwThreadID;
	ret = pthread_create(&heartdwThreadID, NULL, LinuxSendHeartProc , NULL);
#elif VXWORKS
	int ret = taskSpawn("heartThead", 145, 0, 0x200000, (FUNCPTR)vxSendHeartProc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	RUtil::printError("heart thread %d ",ret);
#endif
}

//启动组播线程
void MyThread::createMulticastThread()
{
#ifdef WIN32
	multicastThread = CreateThread(0, 0, winMulticastProc, NULL, 0, &multicastThreadID);
#elif linux
	int ret;
	pthread_t broadcastThreadID;
	ret = pthread_create(&broadcastThreadID, NULL, LinuxBroadCastProc , NULL);
#elif VXWORKS
	int ret = taskSpawn("multicastThread", 150, 0, 0x200000, (FUNCPTR)vxBroadCastProc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	RUtil::printError("multicast thread %d ",ret);
#endif
}

//启动部署线程
void MyThread::createDeployThread()
{
#ifdef WIN32
	deployThread = CreateThread(0, 0, winDeployProc, NULL, 0, &deployThreadID);
#elif linux
	int ret;
	pthread_t deployThreadID;
	ret = pthread_create(&deployThreadID, NULL, LinuxDeployProc, NULL);
#elif VXWORKS
	int ret = taskSpawn("deployThread", 145, 0x01000000, 0x200000, (FUNCPTR)vxDeployProc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	RUtil::printError("deploy thread %d ",ret);
#endif
}

//启动命令线程
void MyThread::createRecvOrderThread()
{
#ifdef WIN32
	recvComdThread = CreateThread(0, 0, winRecvComdProc, NULL, 0, &comdID);
#elif linux
	int ret;
	pthread_t comdID;
	ret = pthread_create(&comdID, NULL, LinuxrecvComdProc, NULL);
#elif VXWORKS
	int ret = taskSpawn("recvOrderThread", 145, 0x01000000, 0x200000, (FUNCPTR)vxRecvComdProc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	RUtil::printError("recvOrder thread %d ",ret);
#endif
}

//启动磁盘扫描线程
void MyThread::FoundDiskThread(Stru_Disk disk)
{
	Stru_Disk *diskbuf = new Stru_Disk();
	memcpy(diskbuf, &disk, sizeof(Stru_Disk));
#ifdef WIN32
	HANDLE diskThread = NULL;
	DWORD diskThreadID = 0;
	diskThread = CreateThread(0, 0, getSystemDiskInfo, diskbuf, 0, &diskThreadID);
#elif linux
	pthread_t diskThreadID;
	pthread_create(&diskThreadID, NULL, getSystemDiskInfo, (void*)diskbuf);
#elif VXWORKS
	taskSpawn("diskThread", 145, 0x01000000, 0x200000, (FUNCPTR)getSystemDiskInfo, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}

//启动进程扫描线程
void MyThread::FoundModuleProxy(Stru_Module module)
{
	Stru_Module *modulebuf = new Stru_Module();
	memcpy(modulebuf, &module, sizeof(Stru_Module));
#ifdef WIN32
	HANDLE moduleThread = NULL;
	DWORD moduleThreadID = 0;
	moduleThread = CreateThread(0, 0, getModuleInfo, (LPVOID)modulebuf, 0, &moduleThreadID);
#elif linux
	pthread_t moduleThreadID;
	pthread_create(&moduleThreadID, NULL, getModuleInfo, (void*)modulebuf);
#elif VXWORKS
	taskSpawn("moduleThread", 145, 0x01000000, 0x200000, (FUNCPTR)getModuleInfo, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}

//启动文件扫描线程
void MyThread::FonudScanFiles(Stru_Scans scans)
{
	Stru_Scans *scanbuf = new Stru_Scans();
	memcpy(scanbuf, &scans, sizeof(Stru_Scans));
#ifdef WIN32
	HANDLE scanThread = NULL;
	DWORD scanThreadID = 0;
	scanThread = CreateThread(0, 0, scanLocalDisk, (LPVOID)scanbuf, 0, &scanThreadID);
#elif linux
	pthread_t scanThreadID;
	pthread_create(&scanThreadID, NULL, scanLocalDisk, (void*)scanbuf);
#elif VXWORKS
	taskSpawn("scanThread", 145, 0x01000000, 0x200000, (FUNCPTR)scanLocalDisk, (int)scanbuf, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}