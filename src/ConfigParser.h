#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "Logger.h"

typedef std::map<std::string, std::string> map_config;

class ConfigParser
{
public:
	ConfigParser(std::string path, Logger* log);
	~ConfigParser();
	bool IsOpen();
	void Parse();
	std::string GetVal(std::string key);


private:
	std::ifstream _file;
	map_config* _map;
	Logger* _log;
};
