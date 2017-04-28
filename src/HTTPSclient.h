#pragma once
#include <tchar.h>
#include <iostream>
//#include <winsock2.h>
#include <windows.h>
#include <WinInet.h>
#include <cstdlib>
#include "Logger.h"
#include <map>

#pragma comment(lib,"ws2_32.lib")

class HTTPclient
{
public:
	HTTPclient(Logger* log);
	
	char* GET(bool& isError, char* firebase_function);
	char* POST(char* json, bool& isError, char* firebase_function);
	const char* createJson(LPCWSTR user_email);
	bool parseJSON(char* json, std::string* message); //the json to parse, a pointer to return the message

private:
	Logger* _log;
};