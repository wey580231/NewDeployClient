#ifndef MY_LOG_2019_08_19
#define MY_LOG_2019_08_19

class MyLog
{
public:
	MyLog();
	~MyLog();

	//创建日志文件
	void FoundLog(char*,char[]);
	//分析配置文件
	void AnalyzeConfig(char[]);
	//日志记录函数
	void WriteLog(const char*,char*,char[]);
	
	//转换文件名称
	int my_mkdir(char*);
	
	//source内容以,分隔，查找innerbuf是否在buffer内容中，如果是返回对应的位置，否则返回-1值
	int findIndex(char *source, char *innerbuf);
	//判断文件filename是否为type类型 多个后缀类型以,分隔
	bool decFileName(char *fileName, char *fileType);

};

#endif //MY_LOG_2019_08_19
