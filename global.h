#ifndef NEWDEPLOYCLIENT_GLOBAL_H
#define NEWDEPLOYCLIENT_GLOBAL_H

extern char filelogName[128];	// 存放本次启动软件记录日志的名称
extern char *LogFloder;		    // 存放日志的路径
extern char serverIP[15];	    // 存放服务器的IP地址
extern char hostIP[15];			// 存放本机IP地址
extern bool g_deployFlag;		// 部署标志位
extern bool g_progFlag;			// 程序运行标志，true表示正常运行，false表示退出

#endif //NEWDEPLOYCLIENT_GLOBAL_H
