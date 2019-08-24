#include "Header.h"
#include "MyTime.h"
#include "MyThread.h"
#include "socket.h"
#include "global.h"
#include "RUtil.h"


// 接收服务器发送的命令信息，包括扫描，部署指令
void RecvCommand()
{
	RSocket tsocket;
	if (!tsocket.createSocket(RSocket::R_UDP)){
		RUtil::printError("create command socket error!");
		return;
	}

	if (!tsocket.bind(nullptr, COMMAND)){
		RUtil::printError("bind command socket error!");
		return;
	}

	MyTime m_time;
	MyThread m_thread;

	char sign[5];				//接收报文标识
	char command[1];			//命令类型
	char recvline[382];			//接收数据缓冲区
	unsigned short t_usRemotePort;
	while (g_progFlag)
	{
		memset(recvline, 0, sizeof(recvline));

		int t_iRecvLen = tsocket.recvFrom(recvline, sizeof(recvline), nullptr, t_usRemotePort);
		if (t_iRecvLen <= 0)
			continue;

		memset(sign, 0, 5);
		// 获得报文标识
		memcpy(sign, recvline, 4);
		memcpy(command, recvline + 4, 1);

		if (strcmp(sign, "S102") == 0) // 表示全盘扫描
		{
			m_time.getTime();

			RUtil::printError("recv command scan:S102");

			Stru_Scans scans;
			int index = 4;
			memcpy(scans.browserID, recvline + index, sizeof(scans.browserID));
			// 获得申请ID，该信息是在返回扫描结果时有效，对本软件无意义
			index += sizeof(scans.browserID);
			memcpy(scans.deviceID, recvline + index, sizeof(scans.deviceID));
			// 获得设备ID，该信息是在返回扫描结果时有效，对本软件无意义
			index += sizeof(scans.deviceID);
			memcpy(scans.compID, recvline + index, sizeof(scans.compID));
			index += sizeof(scans.compID);
			// 获得扫描路径
			memcpy(scans.scanPath, recvline + index, sizeof(scans.scanPath));
			printf("the scans Path:%s\n", scans.scanPath);
			m_thread.FonudScanFiles(scans);
		}
        else if (strcmp(sign, "S103") == 0) // 表示根据指定类型扫描
		{
			m_time.getTime();

			RUtil::printError("recv command scan:S103");

			Stru_Scans scans;
			int index = 4;
			memcpy(scans.browserID, recvline + index, sizeof(scans.browserID));
			// 获得申请ID，该信息是在返回扫描结果时有效，对本软件无意义
			index += sizeof(scans.browserID);
			memcpy(scans.deviceID, recvline + index, sizeof(scans.deviceID));
			// 获得设备ID，该信息是在返回扫描结果时有效，对本软件无意义
			index += sizeof(scans.deviceID);
			memcpy(scans.compID, recvline + index, sizeof(scans.compID));
			index += sizeof(scans.compID);
			memcpy(scans.scanType, recvline + index, sizeof(scans.scanType));
			index += sizeof(scans.scanType);
			memcpy(scans.scanPath, recvline + index, sizeof(scans.scanPath));
			m_thread.FonudScanFiles(scans);
		}
		else if (strcmp(sign, "S105") == 0)   // 表示根据指定类型扫描
		{
			m_time.getTime();

			RUtil::printError("recv command scan:S105");

			Stru_Command com;
			memset(com.command, 0, sizeof(com.command));
			memset(com.sign, 0, sizeof(com.sign));
			int index = 4;
			// 获取查询类型
			memcpy(com.command, command, strlen(command));
			index += sizeof(com.command);

			Stru_Module module;
			int dex = 5;
			memcpy(module.browserID, recvline + dex, sizeof(module.browserID));
			dex += sizeof(module.browserID);
			memcpy(module.processID, recvline + dex, sizeof(module.processID));
			dex += sizeof(module.processID);
			memcpy(module.processName, recvline + dex, sizeof(module.processName));
			dex += sizeof(module.processName);
			memcpy(module.priority, recvline + dex, sizeof(module.priority));
			dex += sizeof(module.priority);
			memcpy(module.processMemory, recvline + dex, sizeof(module.processMemory));
			dex += sizeof(module.processMemory);
			m_thread.FoundModuleProxy(module);

		}
		else if (strcmp(sign, "S106") == 0)
		{
			m_time.getTime();

			RUtil::printError("recv command scan:S106");
			Stru_Command com;
			memset(com.command, 0, sizeof(com.command));
			memset(com.sign, 0, sizeof(com.sign));
			int index = 4;
			// 获取查询类型
			memcpy(com.command, command, strlen(command));
			index += sizeof(com.command);

			Stru_Disk disk;
			int dex = 5;
			memcpy(&disk, recvline + dex, sizeof(Stru_Disk));
			m_thread.FoundDiskThread(disk);
		}
	}
}

