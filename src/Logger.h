#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <chrono>
#include <iomanip>
#include <time.h>

class Logger
{
public:
	Logger(std::string path);
	~Logger();

	void Write(std::string foo_name, std::string data);

	std::_Timeobj<char, const tm*> timeStamp();


private:
	std::fstream _log;
};
