/*!
*  @brief     磁盘信息扫描
*  @details   获取当前操作系统的磁盘信息，以tcp客户端形式，将数据发送至服务器。
*  @author    wey
*  @version   1.0
*  @date      2019.08.22
*  @warning
*  @copyright NanJing RenGu.
*  @note
*/

#include "Header.h"
#include "RUtil.h"
#include "global.h"
#include "socket.h"

char* UnicodeToUTF8(const char* src, int srclen, int &len);

#ifdef WIN32
string TCHAR2STRING(TCHAR *STR)
{
	int iLen = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, NULL);
	char* chRtn = new char[iLen * sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
	string str(chRtn);
	return str;
}
#endif

void GetDiskInfo(char *drive, char *totalDiskInfo, char *freeDiskInfo, vector<Stru_DiskRets> &diskList)
{
#ifdef WIN32
	Stru_DiskRets diskRets;
	int driveCount = 0;
	char szDriveInfo[16 + 1] = { 0 };
	DWORD driveInfo = GetLogicalDrives();
	_itoa_s(driveInfo, szDriveInfo, 2);
	while (driveInfo) {
		if (driveInfo & 1) {
			driveCount++;
		}
		driveInfo >>= 1;
	}
	int driveStrLen = GetLogicalDriveStrings(0, NULL);
	TCHAR *driveStr = new TCHAR[driveStrLen];
	GetLogicalDriveStrings(driveStrLen, driveStr);
	TCHAR *lpDriveStr = driveStr;
	for (int i = 0; i < driveCount; i++)
	{
		ULARGE_INTEGER freeBytesAvailableToCaller;
		ULARGE_INTEGER totalNumberOfBytes;
		ULARGE_INTEGER totalNumberOfFreeBytes;
		GetDiskFreeSpaceEx(lpDriveStr, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes);
		float total = GB(totalNumberOfBytes);
		float free = GB(freeBytesAvailableToCaller);
		string tmp;
		tmp = "";

		string dump;
		dump = TCHAR2STRING(lpDriveStr);
		const char *c_s = dump.c_str();
		int drivelens = 0;
		char *drive = (char*)UnicodeToUTF8(c_s, strlen(c_s), drivelens);
		memcpy(diskRets.drive, drive, sizeof(drive));
		cout << "盘符:" << drive << " ";

		stringstream tt;
		tt << total;
		tt >> tmp;
		tt.str("");
		const char* temp = tmp.c_str();
		memcpy(diskRets.totalDiskInfo, temp, sizeof(temp));
		cout << "总大小:" << temp << " ";
		tmp = "";
		temp = NULL;

		stringstream ff;
		ff << free;
		ff >> tmp;
		tt.str("");
		temp = tmp.c_str();
		memcpy(diskRets.freeDiskInfo, temp, sizeof(temp));
		cout << "剩余大小:" << temp << endl;
		tmp = "";
		temp = NULL;

		diskList.push_back(diskRets);
		lpDriveStr += 4;
	}
#endif

#ifdef linux
	Stru_DiskRets Rets;
	FILE * fp;
	char a[80], d[80], e[80], f[80], buf[256];
	double c, b;
	fp = popen("df", "r");
	double dev_total = 0, dev_used = 0;
	DEV_MEM  *dev = (DEV_MEM *)malloc(sizeof(DEV_MEM));

	int i = 0;
	while (!feof(fp))
	{
		fgets(buf, 256, fp); // leave out \n
		fscanf(fp, "%s %lf %lf %s %s %s", a, &b, &c, d, e, f);
		if (i == 7)
		{
			dev_total += b;
			dev_used += c;
		}
		else
		{

		}
		i++;
	}
	dev->total = dev_total / 1024 / 1024;
	dev->used_rate = (dev_total - dev_used) / 1024 / 1024;
	pclose(fp);

	string tmp = "";
	const char* temp = NULL;

	strcpy(Rets.drive, ".host:/");

	stringstream ttt;
	ttt << dev->total;
	ttt >> tmp;
	ttt.str("");
	temp = tmp.c_str();
	memcpy(Rets.totalDiskInfo, temp, strlen(temp));
	tmp = "";
	temp = NULL;

	stringstream fff;
	fff << dev->used_rate;
	fff >> tmp;
	fff.str("");
	temp = tmp.c_str();
	memcpy(Rets.freeDiskInfo, temp, strlen(temp));
	tmp = "";
	temp = NULL;

	diskList.push_back(Rets);
#endif
}

// 通过tcp返回硬盘结果
void SendDiskRets(Stru_DiskRetReco diskRet)
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
				int len = sizeof(diskRet)+diskRet.diskNum * (32 + 12 + 12);
				
				char *diskbuf = new char[len];
				memset(diskbuf, 0, len);
				
				int index = 0;
				memcpy(diskbuf, diskRet.sign, sizeof(diskRet.sign));						//报文标识
				index += sizeof(diskRet.sign);
				
				memcpy(diskbuf + index, diskRet.browserID, sizeof(diskRet.browserID));		// 申请ID号
				index += sizeof(diskRet.browserID);
				
				for (int i = 0; i < diskRet.diskNum; i++)
				{
					memcpy(diskbuf + index, diskRet.DiskRets[i].drive, sizeof(diskRet.DiskRets[i].drive));
					index += sizeof(diskRet.DiskRets[i].drive);
					memcpy(diskbuf + index, diskRet.DiskRets[i].totalDiskInfo, sizeof(diskRet.DiskRets[i].totalDiskInfo));
					index += sizeof(diskRet.DiskRets[i].totalDiskInfo);
					memcpy(diskbuf + index, diskRet.DiskRets[i].freeDiskInfo, sizeof(diskRet.DiskRets[i].freeDiskInfo));
					index += sizeof(diskRet.DiskRets[i].freeDiskInfo);
				}
				
				int ret = tclient.send(diskbuf, len);

				if (ret < 0)
					RUtil::printError("send disk info failed!");
				else
					RUtil::printError("send disk info success!");

				delete diskbuf;
			}
			else{
				RUtil::printError("disk client connect error!");
			}
		}
	}

	tclient.closeSocket();
}