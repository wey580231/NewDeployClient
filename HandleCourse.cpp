/*!
*  @brief     获取系统进程信息
*  @details   获取当前操作系统的进程信息，以tcp客户端形式，将数据发送至服务器。
*  @author    wey
*  @version   1.0
*  @date      2019.08.22
*  @warning
*  @copyright NanJing RenGu.
*  @note
*/
#include "Header.h"
#include "MyTime.h"
#include "RUtil.h"
#include "global.h"
#include "socket.h"

MyTime m_time;

#ifdef WIN32
//通过进程ID获取进程句柄
HANDLE GetProcessHandle(int nID)
{
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, nID);
}


int GetProcessMemory(HANDLE handle)
{
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	//printf("pid=%d,bytes=%d,kb=%d\n", GetCurrentProcessId(), pmc.WorkingSetSize, int(pmc.WorkingSetSize / 1024));
	return pmc.WorkingSetSize / 1024;
}
#endif


/*!
* @brief 获取当前进程的名称、状态
* @date 2019-08-23
* @warning 【1】若出现 Stack around the variable 'a_memory' was corrupted.错误，则按照【https://www.cnblogs.com/flysnail/archive/2011/09/21/2184114.html】设置项目工程属性
*          【2】若在release下出现_security_cookie的错误，说明vs对缓冲区进行溢出检查，这时在属性-》属性-》c++ -》代码生成：缓冲区安全检查 改为 否 (/GS-)
*/
void GetModuleInfo(char *processID, char *processName, char *priority, char *processMemory, vector<Stru_ModuleRets> &moduleList)
{
#ifdef WIN32
	Stru_ModuleRets moduleRets;
	FILE* fp;
	char szTest[1024];
	string temp;
	int subscript = 0;
	int count = 1;
	int temp_count = 0;
	memset(szTest, 0, 1024);
	fp = _popen("tasklist /FO CSV /NH", "r");
	while ((fgets(szTest, sizeof(szTest) - 1, fp)) != NULL)
	{
		if (strlen(szTest) != 0)
		{
			szTest[1023] = '\0';    
			temp = szTest;
			while (1)
			{
				++subscript;
				if (szTest[subscript] == '"')
				{
					count++;
					if (count == 2)
					{
						//映像名称
						memset(moduleRets.processName, 0, sizeof(moduleRets.processName));
						strcpy(moduleRets.processName, temp.substr(1, (subscript - 1)).c_str());
						temp_count = subscript;
					}
					else if (count == 4)
					{
						//PID
						memset(moduleRets.processID, 0, sizeof(moduleRets.processID));
						strcpy(moduleRets.processID, temp.substr((temp_count + 3), (subscript - temp_count - 3)).c_str());
						memset(moduleRets.priority, 0, 8);
					}
					else if (count == 8)
					{
						temp_count = subscript;
					}
					else if (count == 10)
					{
						//内存使用
						memset(moduleRets.processMemory, 0, sizeof(moduleRets.processMemory));
						int pid = atoi(moduleRets.processID);
						int c_memory = 0;
						HANDLE handle = GetProcessHandle(pid);
						if ((unsigned int)handle == 0) {
							break;
						}
						else
						{
							c_memory = GetProcessMemory(handle);
							char a_memory[8] = { 0 };
							_itoa(c_memory, a_memory, 8);
							strcpy(moduleRets.processMemory,a_memory);
						}
						moduleList.push_back(moduleRets);
						subscript = 0;
						count = 1;
						temp_count = 0;
						break;
					}
				}
				else if (subscript > 1023)
				{
					subscript = 0;
					count = 1;
					temp_count = 0;
					break;
				}
			}
		}
	}
	_pclose(fp);
#elif linux
	Stru_ModuleRets moduleRets;
	FILE* fp;
	char szTest[1024];
	string temp;
	fp = popen("ps -aux", "r");
	while ((fgets(szTest, sizeof(szTest) - 1, fp)) != NULL)
	{
		if (strlen(szTest) != 1)
		{
			memset(szTest, 0, sizeof(szTest));
			fgets(szTest, sizeof(szTest) - 1, fp);
			szTest[1023] = '\0';   
			temp = szTest;
			memset(moduleRets.processName, 0, 128);
			strcpy(moduleRets.processName, temp.substr(0, 8).c_str());
			memset(moduleRets.processID, 0, 5);
			strcpy(moduleRets.processID, temp.substr(11, 4).c_str());

			memset(moduleRets.priority, 0, 8);
			moduleList.push_back(moduleRets);
		}
	}
	pclose(fp);
#elif VXWORKS
	
#endif
	
}

// 通过tcp返回进程结果
void SendModuleRets(Stru_ModuleRetReco moduleRet)
{
	RSocket tclient;
	if (!tclient.createSocket(RSocket::R_TCP)){
		RUtil::printError("create send disk socket error!");
		return;
	}

	if (g_progFlag)
	{
		if (!G_ServerIP.empty())
		{
			if (tclient.connect(G_ServerIP.c_str(), SCANRETS)){
				int len = sizeof(moduleRet)+moduleRet.taskNum * (5 + 128 + 8 + 8);
				
				char *modulebuf = new char[len];
				memset(modulebuf, 0, len);
				int index = 0;
				
				memcpy(modulebuf, moduleRet.sign, sizeof(moduleRet.sign));						//报文标识
				index += sizeof(moduleRet.sign);
				
				memcpy(modulebuf + index, moduleRet.browserID, sizeof(moduleRet.browserID));	// 申请ID号
				index += sizeof(moduleRet.browserID);
				
				for (int i = 0; i < moduleRet.taskNum; i++)
				{
					memcpy(modulebuf + index, moduleRet.ModuleRets[i].processID, sizeof(moduleRet.ModuleRets[i].processID));
					index += sizeof(moduleRet.ModuleRets[i].processID);
					memcpy(modulebuf + index, moduleRet.ModuleRets[i].processName, sizeof(moduleRet.ModuleRets[i].processName));
					index += sizeof(moduleRet.ModuleRets[i].processName);
					memcpy(modulebuf + index, moduleRet.ModuleRets[i].priority, sizeof(moduleRet.ModuleRets[i].priority));
					index += sizeof(moduleRet.ModuleRets[i].priority);
					memcpy(modulebuf + index, moduleRet.ModuleRets[i].processMemory, sizeof(moduleRet.ModuleRets[i].processMemory));
					index += sizeof(moduleRet.ModuleRets[i].processMemory);
				}
				int ret = tclient.send(modulebuf, len); 

				if (ret < 0)
					RUtil::printError("send moduleRets info failed!");
				else
					RUtil::printError("send moduleRets info success!");

				delete modulebuf;
			}
			else{
				RUtil::printError("disk client connect error!");
			}
		}
	}
	tclient.closeSocket();
}


