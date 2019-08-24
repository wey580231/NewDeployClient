#include "MyLog.h"
#include "Header.h"
#include "MyTime.h"
#include "RUtil.h"

MyLog::MyLog()
{
}


MyLog::~MyLog()
{
}

void MyLog::FoundLog(char* sz_floder,char sz_path[])
{
	MyDateTime tDatetime;
	string logName = string(sz_floder) + "log-" + tDatetime.toString() + ".txt";
	strcpy(sz_path, logName.c_str());
}


void MyLog::AnalyzeConfig(char sz_config[])
{
	FILE* fp = fopen("config.txt", "r");
	if (fp == NULL)
	{
		char hostName[255];
		gethostname(hostName, sizeof(hostName));
		struct hostent *p = gethostbyname(hostName);
		int i = 0;
		while (p->h_addr_list[i] != NULL)
		{
			printf("hostname: %s\n", p->h_name);
			printf("ip: %s\n", inet_ntoa(*(struct in_addr*)p->h_addr_list[i]));
			i++;
		}
	}
	else
	{
		char ln[80];
		fgets(ln, 80, fp);
		string data = ln;
		const char* ip;
		int iPos = data.find("=");
		data = data.substr(iPos + 1, 14);//截取字符串返回字节数
		int  n = atoi(data.c_str());
		if (n != 192)
		{
			char hostName[255];
			gethostname(hostName, sizeof(hostName));
			struct hostent *p = gethostbyname(hostName);
			int i = 0;
			while (p->h_addr_list[i] != NULL)
			{
				printf("hostname: %s\n", p->h_name);
				printf("ip: %s\n", inet_ntoa(*(struct in_addr*)p->h_addr_list[i]));
				i++;
			}
		}
		else
		{
			ip = data.c_str();
			cout << "local_IP: " << ip << endl;
			memcpy(sz_config, ip, 15);
		}
		fclose(fp);
	}
}

void MyLog::WriteLog(const char *buf, char* sz_floder, char sz_config[])
{
#ifdef WRITELOG
	if (buf != NULL)
	{
		RUtil::creatDir(sz_floder);
		FILE *fp = fopen(sz_config, "a+");
		if (fp == NULL)
			return;

		MyDateTime tDatetime;
		string time = tDatetime.toString() + "\t";
		fwrite(time.c_str(), 1, time.size(), fp);
		fwrite(buf, 1, strlen(buf), fp);
		fclose(fp);
	}
#endif
}

// source内容以,分隔，查找innerbuf是否在buffer内容中，如果是返回对应的位置，否则返回-1值
int MyLog::findIndex(char *source, char *innerbuf)
{
	string buffer(source);
	// 首先按照,对buffer进行分隔，放到链表当中
	vector<string> sourcelist;
	size_t pos = 0, found = 0;
	while (found != string::npos)
	{
		found = buffer.find(",", pos);
		sourcelist.push_back(string(buffer, pos, found - pos));
		pos = found + 1;
	}
	// 分析innerbuf是否在source中
	for (int i = 0; i < (int)sourcelist.size(); i++)
	{
		if (strcmp(sourcelist[i].c_str(), innerbuf) == 0)
		{
			// 找到对应的内容
			return i + 1;
		}
	}
	return -1;
}


// 判断文件filename是否为type类型 多个后缀类型以,分隔
bool MyLog::decFileName(char *fileName, char *fileType)
{
	if (fileName == 0 || fileType == 0)
		return false;
	int len = strlen(fileName);
	for (int i = 0; i < len; i++)
	{
		if (fileName[i] == '.'&&i != (len - 1))
		{
			char type[40];
			memcpy(&type, &fileName[i + 1], len - i + 1);
			if (findIndex(fileType, type) > 0)
				return true;
		}
	}
	return false;
}