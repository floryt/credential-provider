#pragma once
#include <tchar.h>
#include <iostream>
//#include <winsock2.h>
#include <windows.h>
#include <WinInet.h>
#include <cstdlib>
#include "Logger.h"

#pragma comment(lib,"ws2_32.lib")

class HTTPclient
{
public:
	HTTPclient(Logger* log);
	
	char* GET(bool& isError);
	char* POST(char* json, bool& isError);
	const char* createJson(LPCWSTR username, bool is_guest);
	bool parseJSON(char* json, std::string* message); //the json to parse, a pointer to return the message

private:
	Logger* _log;

};