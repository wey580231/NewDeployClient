#include "MyTime.h"

#include <iostream>

//localtime函数被检测出64位移植性的编译错误，在此忽略此编译错误
#pragma warning(disable : 4996)

MyDate::MyDate() {
	time_t ttime;
	time(&ttime);

	struct tm * ttm = localtime(&ttime);
	m_iYear = 1900 + ttm->tm_year;
	m_iMonth = ttm->tm_mon + 1;
	m_iDay = ttm->tm_mday;
}

MyDate::MyDate(int year, int month, int day):m_iYear(year),m_iMonth(month),m_iDay(day) {

}


bool MyDate::operator==(const MyDate &rhs) const {
	return m_iYear == rhs.m_iYear &&
		   m_iMonth == rhs.m_iMonth &&
		   m_iDay == rhs.m_iDay;
}

bool MyDate::operator!=(const MyDate &rhs) const {
	return !(rhs == *this);
}

MyDate &MyDate::operator=(MyDate &date) {
	this->m_iYear = date.m_iYear;
	this->m_iMonth = date.m_iMonth;
	this->m_iDay = date.m_iDay;
	return *this;
}

void MyDate::setYear(int year) {
	m_iYear = year;
}

int MyDate::getYear() {
	return m_iYear;
}

void MyDate::setMonth(int month) {
	m_iMonth = month;
}

int MyDate::getMonth() {
	return m_iMonth;
}

void MyDate::setDay(int day) {
	m_iDay = day;
}

int MyDate::getDay() {
	return m_iDay;
}

MyDate::MyDate(MyDate &date) {
	this->m_iYear = date.m_iYear;
	this->m_iMonth = date.m_iMonth;
	this->m_iDay = date.m_iDay;
}


MyTime::MyTime()
{
	time_t ttime;
	time(&ttime);

	struct tm * ttm = localtime(&ttime);
	m_iHour = ttm->tm_hour;
	m_iMinute = ttm->tm_min;
	m_iSecond = ttm->tm_sec;
}


MyTime::MyTime(int hour, int min, int sec):m_iHour(hour),m_iMinute(min),m_iSecond(sec) {

}

MyTime::MyTime(MyTime &time) {
	this->m_iHour = time.m_iHour;
	this->m_iMinute = time.m_iMinute;
	this->m_iSecond = time.m_iSecond;
}


MyTime &MyTime::operator=(MyTime &time) {
	this->m_iHour = time.m_iHour;
	this->m_iMinute = time.m_iMinute;
	this->m_iSecond = time.m_iSecond;
	return *this;
}

void MyTime::setHour(int hour) {
	m_iHour = hour;
}

int MyTime::getHour() {
	return m_iHour;
}

void MyTime::setMinute(int minute) {
	m_iMinute = minute;
}

int MyTime::getMinute() {
	return m_iMinute;
}

void MyTime::setSecond(int second) {
	m_iSecond = second;
}

int MyTime::getSecond() {
	return m_iSecond;
}


void MyTime::printTime()
{
	printf("%d:%d:%d",m_iHour,m_iMinute,m_iSecond);
}

// 返回系统时间的名称
string MyTime::getTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H-%M-%S", localtime(&timep));
	return tmp;
}

MyDateTime::MyDateTime():m_date(),m_time() {

}

MyDateTime::MyDateTime(MyDate date, MyTime time):m_date(date),m_time(time) {

}

MyDate MyDateTime::getDate() {
	return m_date;
}

MyTime MyDateTime::getTime() {
	return m_time;
}

string MyDateTime::toString() {

	char buf[64] = {0};
	sprintf(buf,"%d-%d-%d %d:%d:%d",m_date.getYear(),m_date.getMonth(),m_date.getDay(),m_time.getHour(),m_time.getMinute(),m_time.getSecond());
	return string(buf);
}