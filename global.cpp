#include "global.h"

char filelogName[128];					// 存放本次启动软件记录日志的名称
char *LogFloder = (char*)"RGLOGS/";		// 存放日志的路径
string G_ServerIP;						// 服务器IP地址
char hostIP[15] = "0.0.0.0";			// 存放本机IP地址
bool g_deployFlag;			            // 部署标志位
bool g_progFlag;			            // 程序运行标志，true表示正常运行，false表示退出
