#ifndef _STRINGZ_H__
#define _STRINGZ_H__

#include <iostream>
#include <vector>

/*  C++  去除左右空格  */
int trim_z(std::string& s);

/*  C++  去掉字符串首部空格  */
int ltrim_z(char* s);

/*  C++  去掉字符串尾部空格  */
int rtrim_z(char* s);

/*  C++  去掉字符串收尾空格  */
int trim_z(char* s);

std::vector<char*> split_z(char* pch,char* sp);

/*  去掉右侧（一个或多个）换行符\n  */
int rtrim_n_z(char* s);

/*  去掉左侧（一个或多个）换行符\n  */
int ltrim_n_z(char* s);

/*  去掉左侧、右侧（一个或多个）换行符\n  */
int trim_n_z(char* s);

#endif // _STRINGZ_H__