#ifndef RUTIL_H_2019_08_20
#define RUTIL_H_2019_08_20

#include "Header.h"

namespace RUtil{

	extern void printError(const char * format,...);

	extern bool existedDir(const char * filepath);

	//获得当前文件的文件路径 返回值  返回参数中文件的路径  其中filepath不含最后的\\信息
	extern bool find_last_of(char* absfile, char *filepath, int &len);

	// 创建文件夹
	extern int creatDir(char *pDir);

#ifdef linux
	extern string executeBashCommand(string commd);
#endif

}

#endif //RUTIL_H_2019_08_20
