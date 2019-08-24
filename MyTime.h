#ifndef	MY_TIME_H_2019_08_19
#define MY_TIME_H_2019_08_19

#include "Header.h"

class MyDate{
public:
	explicit MyDate();
	explicit MyDate(int year,int month,int day);
	MyDate(MyDate & date);
	MyDate & operator= (MyDate &date);

	bool operator==(const MyDate &rhs) const;

	bool operator!=(const MyDate &rhs) const;

	void setYear(int year);
	int getYear();

	void setMonth(int month);
	int getMonth();

	void setDay(int day);
	int getDay();

private:
	int m_iYear;
	int m_iMonth;
	int m_iDay;
};

class MyTime
{
public:
	explicit MyTime();
	explicit MyTime(int hour,int min,int sec);
	MyTime(MyTime & time);
	MyTime& operator= (MyTime & time);

	void setHour(int hour);
	int getHour();

	void setMinute(int minute);
	int getMinute();

	void setSecond(int second);
	int getSecond();

	void printTime();

	string getTime();

private:
	int m_iHour;
	int m_iMinute;
	int m_iSecond;
};

class MyDateTime
{
public:
	explicit MyDateTime();
	explicit MyDateTime(MyDate date,MyTime time);

	MyDate getDate();
	MyTime getTime();

	string toString();

private:
	MyDate m_date;
	MyTime m_time;

};

#endif //MY_TIME_H_2019_08_19
