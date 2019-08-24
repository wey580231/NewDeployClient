#include "Header.h"
#include "MyTime.h"
#include "MyLog.h"
#include "MyThread.h"
#include "global.h"

int main()
{
#ifdef WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	MyTime m_mytime;
	cout<<m_mytime.getTime()<<endl;
#endif

	//启动在日志文件夹中创建一个日志文件
	MyLog m_mylog;
	m_mylog.FoundLog(LogFloder, filelogName);
    //启动时解析配置文件中的本地IP
	m_mylog.AnalyzeConfig(hostIP);

    g_progFlag = true;
    g_deployFlag = true;

    MyThread m_thread;
	m_thread.createHeartThread();
	m_thread.createMulticastThread();
	m_thread.createDeployThread();
	m_thread.createRecvOrderThread();

#ifdef WIN32
	WaitForSingleObject(m_thread.heartThread, INFINITE); // 等待线程结束
#else
	getchar();
#endif

	return 0;
}
