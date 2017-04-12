#include "Logger.h"
#include <iostream>
#include <ctime>
#include <time.h>
#include <chrono>
#include <iomanip>
#pragma warning(disable : 4996)


Logger::Logger(std::string path)
{
	_log.open(path, std::fstream::in | std::fstream::out | std::fstream::app);
}

Logger::~Logger()
{
	_log.close();
}

void Logger::Write(std::string foo_name, std::string data)
{
	clock_t mili = clock();
	_log << timeStamp() << "."<<mili<< " | " << foo_name << " | " << data << std::endl;
}

std::_Timeobj<char, const tm*> Logger::timeStamp()
{
	std::string to_return = "";

	// current date/time based on current system
	time_t ltime;
	struct tm *Tm;
	ltime = time(NULL);
	Tm = localtime(&ltime);
	return std::put_time(Tm, "%c");
}