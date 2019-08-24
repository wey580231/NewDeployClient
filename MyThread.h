#ifndef MY_THREAD_2019_08_20
#define MY_THREAD_2019_08_20

#include "Header.h"

class MyThread
{
public:
	MyThread();
	~MyThread();

	//创建心跳线程
	void createHeartThread();
	//创建广播线程
	void createMulticastThread();
	//创建文件部署线程
	void createDeployThread();
	//创建命令接收线程
	void createRecvOrderThread();
	//创建扫描磁盘线程
	void FoundDiskThread(Stru_Disk);
	//创建扫描进程线程
	void FoundModuleProxy(Stru_Module);
	//创建文件扫描线程
	void FonudScanFiles(Stru_Scans);

#ifdef WIN32
	HANDLE heartThread;
	HANDLE multicastThread;
	HANDLE deployThread;
	HANDLE recvComdThread;

	DWORD heartdwThreadID;
	DWORD multicastThreadID;
	DWORD deployThreadID;
	DWORD comdID;
#endif
};

#endif //MY_THREAD_2019_08_20