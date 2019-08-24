#include "Header.h"
#include "global.h"
#include "socket.h"
#include "RUtil.h"

double dwStreamIn;
double dwStreamOut;

//获取CPU参数
long deax;
long debx;
long decx;
long dedx;

#ifdef WIN32

void initCPU(long veax)
{
	__asm
	{
		mov eax, veax
		cpuid
		mov deax, eax
		mov debx, ebx
		mov decx, ecx
		mov dedx, edx
	}
}
long getCpuFreq()
{
	int start, over;
	__asm
	{
		RDTSC
		mov start, eax
	}
	Sleep(50);
	__asm
	{
		RDTSC
		mov over, eax
	}
	return (over - start) / 50000;;
}
string getManufactureID()
{
	char manuID[25];
	memset(manuID, 0, sizeof(manuID));
	initCPU(0);
	memcpy(manuID + 0, &debx, 4);
	memcpy(manuID + 4, &dedx, 4);
	memcpy(manuID + 8, &decx, 4);
	return manuID;
}
string getCpuType()
{
	const long id = 0x80000002;
	char cpuType[49];
	memset(cpuType, 0, sizeof(cpuType));
	for (long t = 0; t < 3; t++)
	{
		initCPU(id + t);
		memcpy(cpuType + 16 * t + 0, &deax, 4);
		memcpy(cpuType + 16 * t + 4, &debx, 4);
		memcpy(cpuType + 16 * t + 8, &decx, 4);
		memcpy(cpuType + 16 * t + 12, &dedx, 4);
	}
	return cpuType;
}

long long CompareFileTime(FILETIME time1, FILETIME time2)
{
	long long a = time1.dwHighDateTime << 32 | time1.dwLowDateTime;
	long long b = time2.dwHighDateTime << 32 | time2.dwLowDateTime;
	return   (b - a);
}

int getcpuUsage()
{
	int cpu;
	HANDLE hEvent;
	BOOL res;
	FILETIME preidleTime;
	FILETIME prekernelTime;
	FILETIME preuserTime;
	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;

	res = GetSystemTimes(&idleTime, &kernelTime, &userTime);
	preidleTime = idleTime;
	prekernelTime = kernelTime;
	preuserTime = userTime;
	hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);  // 初始值为 nonsignaled ，并且每次触发后自动设置为nonsignaled 
	WaitForSingleObject(hEvent, 1000);
	res = GetSystemTimes(&idleTime, &kernelTime, &userTime);
	long long idle = CompareFileTime(preidleTime, idleTime);
	long long kernel = CompareFileTime(prekernelTime, kernelTime);
	long long user = CompareFileTime(preuserTime, userTime);
	cpu = (kernel + user - idle) * 100 / (kernel + user) + (rand() % 5);
	return cpu;
}

double getMemorytotalInfo()
{
	string memory_info;
	MEMORYSTATUSEX statusex;
	statusex.dwLength = sizeof(statusex);
	if (GlobalMemoryStatusEx(&statusex))
	{
		unsigned long long total = 0, remain_total = 0, avl = 0, remain_avl = 0;
		double decimal_total = 0, decimal_avl = 0;
		remain_total = statusex.ullTotalPhys % GBYTES;
		total = statusex.ullTotalPhys / GBYTES;
		avl = statusex.ullAvailPhys / GBYTES;
		remain_avl = statusex.ullAvailPhys % GBYTES;
		if (remain_total > 0)
		{
			decimal_total = (remain_total / MBYTES) / DKBYTES;
		}
		/*if(remain_avl > 0)
		{
		decimal_avl = (remain_avl / MBYTES) / DKBYTES;
		}*/
		decimal_total += (double)total;
		//decimal_avl += (double)avl;
		return decimal_total * 1024;
	}
	return -1;
}

double getMemoryavlInfo()
{
	string memory_info;
	MEMORYSTATUSEX statusex;
	statusex.dwLength = sizeof(statusex);
	if (GlobalMemoryStatusEx(&statusex))
	{
		unsigned long long total = 0, remain_total = 0, avl = 0, remain_avl = 0;
		double decimal_total = 0, decimal_avl = 0;
		remain_total = statusex.ullTotalPhys % GBYTES;
		total = statusex.ullTotalPhys / GBYTES;
		avl = statusex.ullAvailPhys / GBYTES;
		remain_avl = statusex.ullAvailPhys % GBYTES;
		if (remain_avl > 0)
		{
			decimal_avl = (remain_avl / MBYTES) / DKBYTES;
		}
		decimal_avl += (double)avl;
		return decimal_avl * 1024;
	}
	return -1;
}


double upstreamSpeed()
{
	FILE* fp;
	char szTest[1024];
	string temp;
	int count = 0;
	double tempStreamOut1 = 0;
	double tempStreamOut2 = 0;
	fp = _popen("netsh interface ipv4 show sub", "r");
	memset(szTest, 0, 1024);
	while (fgets(szTest, sizeof(szTest) - 1, fp) != NULL)
	{
		if (strlen(szTest) != 1)
		{
			szTest[1023] = '\0';   //数组补齐，要不会报越界错误
			temp = szTest;
			temp = temp.substr(39, 11);
			memset(szTest, 0, 1024);
			count++;
			if (count >= 3)
			{
				int a = atoi(temp.c_str());
				tempStreamOut1 += a;
			}
		}
	}
	_pclose(fp);

	Sleep(100);

	memset(szTest, 0, 1024);
	temp = "";
	count = 0;
	fp = _popen("netsh interface ipv4 show sub", "r");
	while (fgets(szTest, sizeof(szTest) - 1, fp) != NULL)
	{
		if (strlen(szTest) != 1)
		{
			szTest[1023] = '\0';   //数组补齐，要不会报越界错误
			temp = szTest;
			temp = temp.substr(39, 11);
			memset(szTest, 0, 1024);
			count++;
			if (count >= 3)
			{
				int a = atoi(temp.c_str());
				tempStreamOut2 += a;
			}

		}
	}
	_pclose(fp);

	dwStreamOut = (tempStreamOut2 - tempStreamOut1) / 1024 / 0.1;
	return dwStreamOut;
}


double downstreamSpeed()
{
	FILE* fp;
	char szTest[1024];
	string temp;
	int count = 0;
	double tempStreamIn1 = 0;
	double tempStreamIn2 = 0;
	fp = _popen("netsh interface ipv4 show sub", "r");
	memset(szTest, 0, 1024);
	while (fgets(szTest, sizeof(szTest) - 1, fp) != NULL)
	{
		if (strlen(szTest) != 1)
		{
			szTest[1023] = '\0';    
			temp = szTest;
			temp = temp.substr(25, 11);
			memset(szTest, 0, 1024);
			count++;
			if (count >= 3)
			{
				int b = atoi(temp.c_str());
				tempStreamIn1 += b;
			}

		}
	}
	_pclose(fp);
	Sleep(100);

	memset(szTest, 0, 1024);
	temp = "";
	count = 0;
	fp = _popen("netsh interface ipv4 show sub", "r");
	while (fgets(szTest, sizeof(szTest) - 1, fp) != NULL)
	{
		if (strlen(szTest) != 1)
		{
			szTest[1023] = '\0'; 
			temp = szTest;
			temp = temp.substr(25, 11);
			memset(szTest, 0, 1024);
			count++;
			if (count >= 3)
			{
				int b = atoi(temp.c_str());
				tempStreamIn2 += b;
			}

		}
	}
	_pclose(fp);
	dwStreamIn = (tempStreamIn2 - tempStreamIn1) / 1024 / 0.1;
	return dwStreamIn;
}
#endif


void SendHeart() {
	RSocket tsocket;
	if (!tsocket.createSocket(RSocket::R_UDP)){
		RUtil::printError("create udp socket error!");
		return;
	}

	if (!tsocket.bind(nullptr, HEARTPORT)){
		RUtil::printError("bind socket error!");
		return;
	}
    
    Stru_Heart heartbuf;

    while (true && g_progFlag) {

		if (strcmp(serverIP, "0.0.0.0") != 0) // 判断当前是否已获知服务器的IP地址
		{
#ifdef WIN32
            //memset((char *)&heartbuf,0,sizeof(Stru_Heart));
            string tmp;
            tmp = "";
            tmp = getCpuType();
            const char* temp = tmp.c_str();
            memcpy(heartbuf.cpuName, temp, strlen(temp));		//CPU名称

            tmp = "";
            temp = NULL;

            long midvalue = getCpuFreq();
            memset(heartbuf.cpuFreq, 0, 6);
            stringstream ss;
            ss << midvalue;
            ss >> tmp;
            ss.str("");
            temp = tmp.c_str();
            memcpy(heartbuf.cpuFreq, temp, strlen(temp));		//CPU主频

            tmp = "";
            temp = NULL;

			int preUsa;
            int usa = getcpuUsage();
            memset(heartbuf.cpuUsage, 0, 4);
            if (usa < 0)
            {
                preUsa = 0 + (rand() % 5);
                stringstream sss;
                sss << preUsa;
                sss >> tmp;
                sss.str("");
                temp = tmp.c_str();
                memcpy(heartbuf.cpuUsage, temp, strlen(temp));

                tmp = "";
                temp = NULL;

            }
            else if (usa > 100)
            {
                preUsa = 100 - (rand() % 5);
                stringstream sss;
                sss << preUsa;
                sss >> tmp;
                sss.str("");
                temp = tmp.c_str();
                memcpy(heartbuf.cpuUsage, temp, strlen(temp));

                tmp = "";
                temp = NULL;
            }
            else if (usa >= 0 && usa <= 100)
            {
                preUsa = usa;
                stringstream sss;
                sss << usa;
                sss >> tmp;
                sss.str("");
                temp = tmp.c_str();
                memcpy(heartbuf.cpuUsage, temp, strlen(temp));					//CPU利用率
                tmp = "";
                temp = NULL;
            }

            double total = getMemorytotalInfo();
            memset(heartbuf.decimal_total, 0, 6);
            stringstream tt;
            tt << total;
            tt >> tmp;
            tt.str("");
            temp = tmp.c_str();
            memcpy(heartbuf.decimal_total, temp, strlen(temp));					//RAM总大小

            tmp = "";
            temp = NULL;

            double avl = getMemoryavlInfo();
            memset(heartbuf.decimal_avl, 0, 6);
            stringstream aa;
            aa << avl;
            aa >> tmp;
            aa.str("");
            temp = tmp.c_str();
            memcpy(heartbuf.decimal_avl, temp, strlen(temp));				//RAM的剩余大小

            tmp = "";
            temp = NULL;

            double upspeed = upstreamSpeed();
            memset(heartbuf.upstreamSpeed, 0, 8);
            stringstream uu;
            uu << upspeed;
            uu >> tmp;
            uu.str("");
            temp = tmp.c_str();
            memcpy(heartbuf.upstreamSpeed, temp, strlen(temp));

            tmp = "";
            temp = NULL;


            double downspeed = downstreamSpeed();
            memset(heartbuf.downstreamSpeed, 0, 8);
            stringstream dd;
            dd << downspeed;
            dd >> tmp;
            dd.str("");
            temp = tmp.c_str();
            memcpy(heartbuf.downstreamSpeed, temp, strlen(temp));

            tmp = "";
            temp = NULL;
#else
			strcpy(heartbuf.cpuName, "0");
			strcpy(heartbuf.cpuFreq, "0");
			strcpy(heartbuf.cpuUsage, "0");
			strcpy(heartbuf.decimal_total, "0");
			strcpy(heartbuf.decimal_avl, "0");
#endif
			int ret = tsocket.sendTo((char *)&heartbuf, sizeof(heartbuf), serverIP, HEARTPORT);
			if (ret == -1)
			{
				RUtil::printError("send heart error [%d:%d]",serverIP,HEARTPORT);
			}

#ifdef WIN32
			Sleep(500);
#elif linux
			usleep(500000);
#elif DVXWORK
			sleep(1 / 2);
#endif
        }
    }

	tsocket.closeSocket();
}