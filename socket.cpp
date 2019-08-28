#include "socket.h"

#include <cstdio>
#include <iostream>
#include <cstring>
#include "RUtil.h"

using namespace std;

RSocket::RSocket():socktype(R_NONE)
{
    sockFd = 0;
    socketPort = 0;
    errorCode = 0;
    socketValid = false;
    blockAble = false;
    memset(socketIp,0,sizeof(socketIp));
}

RSocket::RSocket(const RSocket &rsock)
{
    this->sockFd = rsock.sockFd;
    this->socketPort = rsock.socketPort;
    this->errorCode = rsock.errorCode;
    this->socketValid = rsock.socketValid;
    this->blockAble = rsock.blockAble;
    memset(socketIp,0,sizeof(socketIp));
    memcpy(this->socketIp,rsock.socketIp,sizeof(socketIp));
}

bool RSocket::createSocket(SocketType socktype)
{
#ifdef WIN32
    static bool isInit = false;
    if(!isInit)
    {
        isInit = true;
        WSADATA ws;
        if(WSAStartup(MAKEWORD(2,2),&ws) != 0)
        {
			RUtil::printError("Init windows socket failed!");
            return false;
        }
    }
#endif

    sockFd = socket(AF_INET,socktype,0);
    if(sockFd == -1)
    {
        errorCode = getErrorCode();
		RUtil::printError("Create socket failed! [ErrorCode:%d]", errorCode);
        return false;
    }

    socketValid = true;
    return true;
}

bool RSocket::bind(const char *ip, unsigned short port)
{
    if(!isValid())
        return false;

    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(port);
	if (ip == NULL)
		localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		localAddr.sin_addr.s_addr = inet_addr(ip);

    int ret = ::bind(sockFd,(sockaddr*)&localAddr,sizeof(localAddr));
    if(ret == -1)
    {
        closeSocket();
        errorCode = getErrorCode();
		RUtil::printError("Bind socket error [%s:%d] [ErrorCode:%d]", ip, port, errorCode);
        return false;
    }

	if (ip != NULL)
		strcpy(socketIp,ip);

    socketPort = port;

    return true;
}

/*!
 * @brief 开启socket监听
 * @param[in] backlog 同一时刻最大允许连接数量
 * @return 是否监听成功
 */
bool RSocket::listen(int backlog)
{
    if(!isValid())
        return false;

    int ret = ::listen(sockFd,backlog);
    if(ret == -1)
    {
        closeSocket();
        errorCode = getErrorCode();
		RUtil::printError("Listen socket error! [ErrorCode:%d]", errorCode);
        return false;
    }

	RUtil::printError("Listen socket success!");

    return true;
}

bool RSocket::closeSocket()
{
    if(isValid())
    {
        if(closesocket(sockFd) == 0)
        {
            socketValid = false;
            sockFd = 0;
            return true;
        }
#ifdef WIN32
		RUtil::printError("Close socket error [ErrorCode:%d]!", GetLastError());
#endif
    }
    return false;
}

/*!
 * @brief 作为server端接收客户端连接
 * @param[in] 无
 * @return 返回接收的socket描述信息
 */
RSocket RSocket::accept()
{
    RSocket tmpSocket;
    if(!isValid())
        return tmpSocket;

    sockaddr_in clientAddr;
    int len = sizeof(clientAddr);
#ifdef VXWORKS
    int clientSocket = ::accept(sockFd,(sockaddr*)&clientAddr,(int*)&len);
#else
    int clientSocket = ::accept(sockFd,(sockaddr*)&clientAddr,(socklen_t*)&len);
#endif
    if(clientSocket == -1)
    {
#ifdef WIN32
		RUtil::printError("Accept failed [ErrorCode:%d]!", GetLastError());
#endif
        return tmpSocket;
    }

    strcpy(tmpSocket.socketIp,inet_ntoa(clientAddr.sin_addr));
    tmpSocket.socketValid = true;
    tmpSocket.socketPort= ntohs(clientAddr.sin_port);
    tmpSocket.sockFd = clientSocket;
    tmpSocket.socktype = R_TCP;

	RUtil::printError("Recv socket [%s:%d]", tmpSocket.socketIp, tmpSocket.socketPort);

    return tmpSocket;
}

/*!
 * @brief 接收数据，并将结果保存至缓冲区
 * @param[out] buff 保存并返回接收的数据
 * @param[in] length 缓冲区数据长度
 * @return 实际接收数据的长度
 */
int RSocket::recv(char *buff, int length)
{
    if(!isValid() || buff == NULL)
        return -1;

    size_t buffLen = strlen(buff);
    memset(buff,0,buffLen);

    int ret = ::recv(sockFd,buff,length,0);

	if (ret < 0){
		errorCode = getErrorCode();
	}

    return ret;
}

/*!
 * @brief 发送一定长度数据
 * @param[in] buff 待发送数据的缓冲区
 * @param[in] length 待发送的长度
 * @return 是否发送成功
 */
int RSocket::send(const char *buff, const int length)
{
    if(!isValid() || buff == NULL)
        return -1;

    int sendLen = 0;
    while(sendLen != length)
    {
        int ret = ::send(sockFd,buff+sendLen,length-sendLen,0);
        if (ret <= 0)
        {
			if (ret < 0){
				errorCode = getErrorCode();
			}
            sendLen = -1;
            break;
        }
        sendLen += ret;
    }
    return sendLen;
}

/*!
 * @brief 连接socket
 * @param[in] remoteIp 远程IP
 * @param[in] remotePort 远程端口
 * @param[in] teimeouts 超时时间
 * @return 连接是否成功
 */
bool RSocket::connect(const char *remoteIp, const unsigned short remotePort, int timeouts)
{
    if(!isValid())
        return false;

    sockaddr_in remoteAddr;
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(remotePort);
    remoteAddr.sin_addr.s_addr = inet_addr(remoteIp);

    setBlock(false);

    if(::connect(sockFd,(sockaddr*)&remoteAddr,sizeof(remoteAddr)) != 0)
    {
        fd_set set;
#ifdef VXWORKS
        memset((char *)&set,0,sizeof(fd_set));
#else
        FD_ZERO(&set);
#endif
        FD_SET(sockFd,&set);
        timeval tm;
        tm.tv_sec = timeouts;
        tm.tv_usec = 0;
        if(select(sockFd+1,0,&set,0,&tm) <= 0)
        {
			RUtil::printError("Connect timeout or error [%s:%d] %s", remoteIp, remotePort, strerror(errno));
            return false;
        }
    }

    setBlock(true);
	RUtil::printError("connect %s:%d success!", remoteIp, remotePort);
    return true;
}

/*!
 * @brief 设置socket是否为阻塞模式
 * @param[in] flag 状态
 * @return 返回设置的是否成功设置
 */
bool RSocket::setBlock(bool flag)
{
    if(!isValid())
        return false;

#if defined(WIN32)
    unsigned long ul = 0;
    if(!flag)
    {
        ul = 1;
    }
    ioctlsocket(sockFd,FIONBIO,&ul);
#else
    int flags = fcntl(sockFd,F_GETFL,0);
    if(flags<0)
        return false;

    if(flag)
        flags = flags&~O_NONBLOCK;
    else
        flags = flags|O_NONBLOCK;

    if(fcntl(sockFd,F_SETFL,flags)!=0)
        return false;
#endif
    return true;
}

/*!
 * @brief UDP数据接收
 * @param[in] buff 接收的数据缓冲区
 * @param[in] length 接收的数据长度
 * @param[in] senderIp 发送端ip地址
 * @param[in] senderPort 发送端端口号
 * @return 实际接收数据的长度
 */
int RSocket::recvFrom(char *buff, int length, char *senderIp, unsigned short &senderPort)
{
    if(!isValid())
        return -1;

    socklen_t fromlen = sizeof(sockaddr);
#ifdef VXWORKS
    int recvLen = ::recvfrom(sockFd,buff,length,0,(sockaddr *)&m_lastRecvAddr,(int *)&fromlen);
#else
    int recvLen = ::recvfrom(sockFd,buff,length,0,(sockaddr *)&m_lastRecvAddr,&fromlen);
#endif

    if(recvLen < 0){
        errorCode = getErrorCode();
#ifdef WIN32
		if (errorCode != WSAETIMEDOUT && errorCode != WSAEWOULDBLOCK){
			RUtil::printError("Recv socket error [ErrorCode:%d]!\n", GetLastError());
		}
#elif linux
        if(errorCode != ETIMEDOUT && errorCode != EWOULDBLOCK){
			RUtil::printError("Recv socket error [ErrorCode:%d]!\n",errorCode);
        }
#endif
        return -1;
    }

    if(senderIp != NULL){
        strcpy(senderIp,inet_ntoa(m_lastRecvAddr.sin_addr));
    }

    senderPort = ntohs(m_lastRecvAddr.sin_port);

    return recvLen;
}

/*!
 * @brief UDP数据发送
 * @param[in] buff char* 待发送数据缓冲区
 * @param[in] length int 待发送数据长度
 * @param[in] dest char* 目的地址
 * @param[in] port int   目的端口号
 * @return 实际发送数据长度
 */
int RSocket::sendTo(const char *buff, const int length, const char *dest, const int port)
{
    if(!isValid())
        return -1;

    sockaddr_in raddr;
    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(port);
    raddr.sin_addr.s_addr = inet_addr(dest);

    int sendLen = 0;
    while(sendLen != length)
    {
#ifdef VXWORKS
    	int ret = ::sendto(sockFd,const_cast<char *>(buff),length,0,(sockaddr*)&raddr,sizeof(sockaddr_in));
#else
    	int ret = ::sendto(sockFd,buff,length,0,(sockaddr*)&raddr,sizeof(sockaddr_in));
#endif
        if (ret <= 0)
        {
            sendLen = -1;
			RUtil::printError("send to %s [ErrorCode:%d]!", dest, getErrorCode());
            break;
        }
        sendLen += ret;
    }

    return sendLen;
}

/*!
 *  @brief UDP广播发送至指定端口
 */
int RSocket::broadcast(const char *buff, const int length,const char * broadcastIp, const int port)
{
    if(!isValid())
        return -1;

    sockaddr_in raddr;
    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(port);
//    raddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    raddr.sin_addr.s_addr = inet_addr(broadcastIp);

    int sendLen = 0;
    while(sendLen != length)
    {
#ifdef VXWORKS
    	int ret = ::sendto(sockFd,const_cast<char *>(buff),length,0,(sockaddr*)&raddr,sizeof(sockaddr_in));
#else
    	int ret = ::sendto(sockFd,buff,length,0,(sockaddr*)&raddr,sizeof(sockaddr_in));
#endif
        if (ret <= 0)
        {
            sendLen = -1;
            break;
        }
        sendLen += ret;
    }

    return sendLen;
}

bool RSocket::joinGroup(const char * localIp, const char * groupIp)
{
	if (!isValid())
		return false;

	ip_mreq mreq;
	mreq.imr_interface.s_addr = inet_addr(localIp);    //本地某一网络设备接口的IP地址。
	mreq.imr_multiaddr.s_addr = inet_addr(groupIp);   //组播组的IP地址。
	int ret = setsockopt(sockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
	
	return ret == 0;
}

int RSocket::setSendBuff(int bytes)
{
   return setSockopt(SO_SNDBUF,(char *)&bytes,sizeof(bytes));
}

int RSocket::setRecvBuff(int bytes)
{
    return setSockopt(SO_RCVBUF,(char *)&bytes,sizeof(bytes));
}

int RSocket::setRecvTimeOut(int millsecond)
{
#ifdef WIN32
	return setSockopt(SO_RCVTIMEO, (char *)&millsecond, sizeof(millsecond));
#else
	struct timeval timeout = {millsecond / 1000,0};   
	return setSockopt(SO_RCVTIMEO,(char *)&timeout, sizeof(struct timeval));
#endif
}

int RSocket::setSendTimeOut(int millsecond)
{
#ifdef WIN32
	return setSockopt(SO_SNDTIMEO, (char *)&millsecond, sizeof(millsecond));
#else
	struct timeval timeout = { millsecond / 1000, 0 };
	return setSockopt(SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
#endif
}

int RSocket::setSockopt(int optname, const char *optval, int optlen,int level)
{
    if(!isValid())
        return -1;

#ifdef VXWORKS
	return setsockopt(sockFd, level, optname,const_cast<char *>(optval), optlen);
#else
	return setsockopt(sockFd, level, optname, optval, optlen);
#endif
}

int RSocket::getSockopt(int optname, char *optval, socklen_t optlen, int level)
{
    if(!isValid())
        return -1;
#ifdef VXWORKS
	return getsockopt(sockFd, level, optname, const_cast<char *>(optval), (int *)&optlen);
#else
	return setsockopt(sockFd, level, optname, optval, optlen);
#endif
}

int RSocket::getSendBuff()
{
    int sendBuff = 0;
    int result = getSockopt(SO_SNDBUF,(char *)&sendBuff,sizeof(sendBuff));
    if(result < 0)
        return -1;
    return sendBuff;
}

int RSocket::getRecvBuff()
{
    int sendBuff = 0;
    int result = getSockopt(SO_RCVBUF,(char *)&sendBuff,sizeof(sendBuff));
    if(result < 0)
        return -1;
    return sendBuff;
}

int RSocket::getSendTimeout()
{
    int sendBuff = 0;
    int result = getSockopt(SO_SNDTIMEO,(char *)&sendBuff,sizeof(sendBuff));
    if(result < 0)
        return -1;
    return sendBuff;
}

int RSocket::getRecvTimeOut()
{
    int sendBuff = 0;
    int result = getSockopt(SO_RCVTIMEO,(char *)&sendBuff,sizeof(sendBuff));
    if(result < 0)
        return -1;
    return sendBuff;
}

int RSocket::getLastError()
{
    return errorCode;
}

int RSocket::getErrorCode()
{
#ifdef WIN32
    return WSAGetLastError();
#elif defined(linux)
    return errno;
#endif
}
