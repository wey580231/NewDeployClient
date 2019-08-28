#include <fstream>
#include "Header.h"
#include "MyTime.h"
#include "md5.h"
#include "socket.h"
#include "global.h"
#include "RUtil.h"

long long int processLen;			//进度偏移量
char tailBuf[TAILBUFSIZE];			//存放粘连部分的buffer

extern char* UTF8ToUniCode(const char* utf8, int srclen, int &len);

void setRequestFlag(char * dest,const char * flag){
	memset(dest, 0, 16);
	strcpy(dest, flag);
}

bool createDir(char * path){
	char filepath[256] = { 0 };
	int filepathlen = 0;
	if (RUtil::find_last_of(path, filepath, filepathlen)){
		if (!RUtil::existedDir(filepath)){
			return (RUtil::creatDir(filepath) == 0);
		}else{
			return true;
		}
	}
	return false;
}

//部署文件
void DeployFiles()
{
	RSocket tsocket;

	if (!tsocket.createSocket(RSocket::R_TCP)){
		RUtil::printError("create deploy socket failed!");
		return;
	}

	if (!tsocket.bind(NULL, DEPLOYPORT)){
		RUtil::printError("bind deploy socket failed!");
		return;
	}

	if (!tsocket.listen()){
		RUtil::printError("listen deploy socket failed!");
		return;
	}

	int tcp_nodelay = 1;
	if (tsocket.setSockopt(SO_KEEPALIVE, (char*)&tcp_nodelay, sizeof(int)) != 0){
		RUtil::printError("set deploy socket keepalive failed!");
		return;
	}

	if (tsocket.setSockopt(TCP_NODELAY, (char*)&tcp_nodelay, sizeof(int), IPPROTO_TCP) != 0){
		RUtil::printError("set deploy socket nodelay failed!");
		return;
	}

	bool recvFlag;
	char requestFlag[16];
	MyTime m_time;
	
	while (g_progFlag)
	{
		RSocket tclient = tsocket.accept();
		if (!tclient.isValid())
			continue;

		// 接受文件传输数据
		int RecvLen;
		char recvbuf[REVBUFSIZE] = ""; // 用于将接收的转换UTF8格式

		while (1)
		{
			memset(recvbuf, 0, sizeof(recvbuf));
			RecvLen = tclient.recv(recvbuf, REVBUFSIZE);

			if (RecvLen <= 0)
			{
				int errorCode = tclient.getLastError();

				if (errorCode == EWOULDBLOCK || errorCode == ETIMEDOUT)
					continue;
				else {
					RUtil::printError("deploy client recv name error,code [%d]",errorCode);
					break;
				}
			}

			RUtil::printError(m_time.getTime().c_str());

			//判断是否部署结束
			int serialflag = 0;
			memcpy((char*)&serialflag, recvbuf, 4);

			if (serialflag == -1) {
				RUtil::printError("deploy end!");
				break;
			}

			long long size = 0;			//待部署文件长度
			memcpy((char*)&size, (recvbuf + SERINUMLENGTH + PATHLENGTH + MD5LENGTH), 8);

			char path[PATHLENGTH];
			char md5[MD5LENGTH];
			memset(path, 0, PATHLENGTH);
			memset(md5, 0, MD5LENGTH);
			memcpy(md5, (recvbuf + PATHLENGTH + SERINUMLENGTH), MD5LENGTH);

			// 转换为UTF-8格式
#ifdef WIN32
//			char *convertBuf = UTF8ToUniCode(recvbuf, RecvLen, RecvLen);
//			memcpy(recvbuf, convertBuf, RecvLen);
//			delete convertBuf;
#endif
			memcpy(path, (recvbuf + SERINUMLENGTH), PATHLENGTH);

			RUtil::printError("Start Deploy>>>>>> %s", path);

			if (size == 0)
			{
				if (createDir(path)){
					std::ofstream ofs(path, ios::binary);
					ofs.close();
				}
				setRequestFlag(requestFlag, "c");
				tclient.send((const char*)requestFlag, strlen(requestFlag));
				continue;
			}
			
			DeployPackageEntity package(0, path, md5);
			FILE* pFile = fopen(package.targetPath, "r");

			//如果当前文件不存在，则通知服务器开始发送当前文件的报文
			if (NULL == pFile)
			{
				setRequestFlag(requestFlag,"t");
				tclient.send((const char*)requestFlag, strlen(requestFlag));

				recvFlag = true;
			}
			else
			{		
				MD5_CTX mdvalue;
				char md5temp[34];
				memset(md5temp, 0, 34);
				mdvalue.GetFileMd5(md5temp, package.targetPath);

				fclose(pFile);

				//如果存在，MD5也一致，则通知服务器部署下一个文件
				if (0 == strcmp(md5temp, package.MD5Value))
				{
					setRequestFlag(requestFlag, "f");
					tclient.send((const char*)requestFlag, strlen(requestFlag));

					recvFlag = false;
					continue;
				}
				else
				{
					//如果文件存在，MD5值不一致，则重新部署该文件
					setRequestFlag(requestFlag, "t");
					tclient.send((const char*)requestFlag, strlen(requestFlag));
					recvFlag = true;
				}
			}
		
			if (!createDir(path))
				continue;

			std::ofstream ofs;
			ofs.open(path, ios::binary);

			static int recveddatanum = 0;			//已接收的数据包长度
			static int tailDataLength = 0;			//粘包的长度	
			bool deployEnd = false;					//当前文件部署结束标志位
			memset(tailBuf, 0, TAILBUFSIZE);

			if (recvFlag == true)
			{
				char recvdatabuf[REVBUFSIZE] = { 0 };

				while (1)
				{
					static int t_iRecvLen;

					memset(recvdatabuf, 0, sizeof(recvdatabuf));
					
					t_iRecvLen = tclient.recv(recvdatabuf, REVBUFSIZE);

					if (t_iRecvLen <= 0)
					{
						int errorCode = tclient.getLastError();

						if (errorCode == EWOULDBLOCK || errorCode == ETIMEDOUT)
							continue;
						else {
							RUtil::printError("deploy client recv content error,code [%d]",errorCode);
							break;
						}
					}

					bool hasTailBuff = false;
					char * newBuff = NULL;
					int newBuffLen;
					
					//判断粘连buf中是否存在东西，存在则将其中的东西与收到的新的recvdatabuf进行拼接
					if (tailDataLength != 0) {
						newBuffLen = t_iRecvLen + tailDataLength;
						newBuff = (char *)malloc(newBuffLen);
						memcpy(newBuff, tailBuf, tailDataLength);
						memcpy(newBuff + tailDataLength, recvdatabuf, t_iRecvLen);
						hasTailBuff = true;
					}
					else {
						newBuff = recvdatabuf;
						newBuffLen = t_iRecvLen;
					}

					processLen = 0;
					do {
						//即将解析的buf剩余长度是否足以解析一个报文头，足够解析则先解析一个头部
						if (newBuffLen - processLen >= sizeof(HeadPacket)) 
						{
							HeadPacket head;
							memcpy((char*)&head, newBuff + processLen, sizeof(head));
							//解析完头部剩余的buf长度是否足够解析一个单步长度，足够则解析，不够则将其保留下来继续去接收新的报文
							if (newBuffLen - processLen - sizeof(head) >= head.dataSize) 
							{
								processLen += sizeof(head);

								//可解析一次
								char* array_data = newBuff + processLen;
								processLen += head.dataSize;
								
								//记录已接收的数据总长度
								recveddatanum += head.dataSize;
								//如果已收的报文字节长度等于报文的总长度，则该文件接收完毕，通知服务器部署下一个路径
								if (recveddatanum == head.totalSize) {
									ofs.write(array_data, head.dataSize);
									ofs.close();
									ofs.clear();
									//通知服务器该文件已经部署结束，开始传输下一个路径
									setRequestFlag(requestFlag, "c");
									int realSendLen = tclient.send((const char*)requestFlag, strlen(requestFlag));

									if (realSendLen <= 0) {
										RUtil::printError("deploy client Send request error");
										break;
									}

									tailDataLength = 0;
									recveddatanum = 0;
									deployEnd = true;
									break;
								}
								else {
									ofs.write(array_data, head.dataSize);
								}
								//如果偏移量和解析报文的总长度相等，则粘连buf中的内容已处理完毕
								if (processLen == newBuffLen) {
									tailDataLength = 0;
									break;
								}
							}
							else {
								memset(tailBuf, 0, TAILBUFSIZE);
								int leftLen = newBuffLen - processLen;
								memcpy(tailBuf, newBuff + processLen, leftLen);
								tailDataLength = leftLen;
								break;
							}
						}
						else {
							memset(tailBuf, 0, TAILBUFSIZE);
							int leftLen = newBuffLen - processLen;
							memcpy(tailBuf, newBuff + processLen, leftLen);
							tailDataLength = leftLen;
							break;
						}
					} while (1);

					//如果此时解析的buf不等于空并且标志位为true则需要释放开辟的buf内存
					if (newBuff != NULL && hasTailBuff)
						free(newBuff);

					//如果当前文件的部署结束标志为true，则跳出当前的while循环，去接收新的部署路径
					if (deployEnd == true) {
						break;
					}
				}
			}
		}
	}
	
	tsocket.closeSocket();
}