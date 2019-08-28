#include "Header.h"
#include "global.h"
#include "socket.h"
#include "RUtil.h"

bool AnalysisFile()
{
	bool rState;
	FILE *file;
	char ln[80];
#ifdef WIN32
	fopen_s(&file, "returnpingdata.txt", "r");
#elif linux
	//fopen_s(&file,"w");
	return false;
#elif VXWORKS
	/*char filePath[128];
	memset(filePath,0,128);
	string path = "/ata0a/deployClient/NewDeployClient/returnpingdata.txt";
	strcpy(filePath,path.c_str());
	FILE *fp = fopen(filePath,"w+");
	if(fp == NULL)
		return false;*/
#endif

	fgets(ln, 80, file);//读入空行，舍弃
	fgets(ln, 80, file);//读入ping信息，舍弃
	fgets(ln, 80, file);//读入ping对象返回值，用来分析

	string data = ln;
	int iPos = data.find("=");
	data = data.substr(iPos + 1, 3);
	int  n = atoi(data.c_str());
	rState = n > 0;
	fclose(file);
	return rState;
}

void RecvServiceMulticast()
{
	int r;

	RSocket tsocket;
	if (!tsocket.createSocket(RSocket::R_UDP)){
		RUtil::printError("create serviceBroadcast socket error!");
		return;
	}

    int t_iReuseAddr = 1;
    if(tsocket.setSockopt(SO_REUSEADDR,(char *)&t_iReuseAddr,sizeof(t_iReuseAddr))!= 0){
        RUtil::printError("serviceBroadcast reuseaddr error!");
        return;
    }

#ifdef linux
    if(tsocket.setSockopt(SO_REUSEPORT,(char *)&t_iReuseAddr,sizeof(t_iReuseAddr))!= 0){
        RUtil::printError("serviceBroadcast reuseport error!");
        return;
    }
#endif

	if (!tsocket.bind(hostIP, BOARDCASTPORT)){
		RUtil::printError("bind serviceBroadcast socket error!");
		return;
	}
	
	if (!tsocket.joinGroup(hostIP, "224.10.10.15")){
		RUtil::printError("join group error!");
		return;
	}

	if (tsocket.setRecvTimeOut(10000) != 0){
		RUtil::printError("set serviceBroadcast recv timeout error!");
		return;
	}

    socklen_t len = sizeof(sockaddr_in);
	char t_remoteIp[20] = {0};
	unsigned short t_usRemotePort = 0;
	char sign[5];
	char recvline[20];
	
	while (g_progFlag)
	{
		memset(recvline, 0, 20);
		
		r = tsocket.recvFrom(recvline, sizeof(recvline), t_remoteIp, t_usRemotePort);
		
		if (r <= 0)
			continue;

		memset(sign, 0, 5);
		// 获得报文标识
		memcpy(sign, recvline, 4);
		if (strcmp(sign, "S101") == 0)
		{
			// 获得服务器的IP地址
			G_ServerIP.clear();
			G_ServerIP.append(recvline + 4, r - 4);
			
			RUtil::printError("recv server ip:%s", G_ServerIP.c_str());
			
#ifdef WIN32
			break;
			/*
				char frontstr[100] = "cmd /c ping ";
				strncat(frontstr, G_ServerIP.c_str(), sizeof(G_ServerIP.length()));
				char afterstr[100] = " -n 1 -w 1000 >returnpingdata.txt";
				strncat(frontstr, afterstr, sizeof(afterstr));

				WinExec((char*)frontstr, SW_HIDE);
				Sleep(1000);
				bool returndata = AnalysisFile();
				if (returndata == false)
				{
					G_ServerIP.clear();
					RUtil::printError("ping server %s error", G_ServerIP.c_str());
				}
				else if (returndata == true)
				{
					break;
				}
			*/
#elif linux
            //NOTE 2019-08-23 linux只ping服务器,不检测是否可以ping通（无意义）
			char command[80] = {0};
			string tmp = " ping ";
			strncat(command,tmp.c_str(),strlen(tmp.c_str()));
			strncat(command,G_ServerIP.c_str(),G_ServerIP.size());
			tmp = " -c4 > `pwd`/returnpingdata.txt ";
			strncat(command,tmp.c_str(),strlen(tmp.c_str()));

			if(popen(command,"r") == NULL){
				RUtil::printError("exec command [%s] error.",command);
				break;
			}
#elif VXWORKS
			
#endif
		}
	}

	tsocket.closeSocket();
}