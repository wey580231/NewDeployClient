#include "RUtil.h"

#include <cstdio>
#include <cstdarg>
#include <iostream>

#include "Header.h"

#ifdef DVXWORK

#endif

namespace RUtil{

void printError(const char * format, ...){
	char tbuff[1024] = { 0 };

	va_list list;
	va_start(list,format);
	vsnprintf(tbuff, 1024, format, list);
	va_end(list);

#ifdef VXWORKS
	logMsg(tbuff, 0, 0, 0, 0, 0, 0);
#else
	std::cout << tbuff << std::endl;
#endif

}


/*!
*  @brief 获取指定路径是否存在
*  @param[in]  filepath 待测试路径信息
*  @return true:路径存在;false:路径不存在
*/
bool existedDir(const char * filepath)
{
	return access(filepath,0) == 0;
}

// 获得当前文件的文件路径 返回值  返回参数中文件的路径  其中filepath不含最后的\\信息
bool find_last_of(char* absfile, char *filepath, int &len)
{
	int findindex = -1;
	for (int i = 0; i < (int)strlen(absfile); i++)
	{
		if (absfile[i] == '\\' || absfile[i] == '/') // 找到\\字符串了
		{
			findindex = i;
			continue;
		}
	}
	if (findindex == -1)
		return false;
	else
	{
		if (filepath == NULL)
		{
			filepath = new char[strlen(absfile)];
			memset(filepath, 0, strlen(absfile));
		}
		memcpy(filepath, absfile, findindex);
		return true;
	}
	return false;
}


int my_mkdir(char *pszDir)
{
#ifdef WIN32
	return mkdir(pszDir);
#endif
#ifdef linux
	return mkdir(pszDir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
#endif
#ifdef DVXWORK
	return mkdir(pszDir);
#endif
}


// 创建文件夹
int creatDir(char *pDir)
{
	if (NULL == pDir)
		return 0;

	int i = 0;
	int iRet;
	int iLen;
	char* pszDir;

	//pszDir = strdup(pDir);
	pszDir = new char[strlen(pDir) + 1];
	memset(pszDir, 0, strlen(pDir) + 1);
	memcpy(pszDir, pDir, strlen(pDir));
	iLen = strlen(pszDir);

	// 遍历依次查找每一级的目录文件
	for (i = 0; i < iLen; i++)
	{
		if ((pszDir[i] == '\\' || pszDir[i] == '/') && i != 0)
		{
			pszDir[i] = '\0';

			// 判断该目录是存在
			iRet = access(pszDir, 0);
			if (iRet != 0)
			{
				//printf("the %s is not exist\n",pszDir);
				iRet = my_mkdir(pszDir);
				if (iRet != 0)
				{
					//printf("Create %s file is failed\n",pszDir);
					return -1;
				}
			}
			else
			{
				//printf("the %s is exist\n",pszDir);
			}
			pszDir[i] = '/';
		}
	}

	iRet = my_mkdir(pszDir);
	free(pszDir);
	return iRet;
}

}