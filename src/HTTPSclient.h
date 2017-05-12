#pragma once
#include <tchar.h>
#include <iostream>
//#include <winsock2.h>
#include <windows.h>
#include <WinInet.h>
#include <cstdlib>
#include "dbugLog.h"
#include <map>
#include "config.h"

#pragma comment(lib,"ws2_32.lib")

class HTTPclient
{
public:
	HTTPclient();
	
	char* GET(bool& isError, char* firebase_function);
	char* POST(char* json, bool& isError, char* firebase_function);
	const char* createJson(LPCWSTR user_email);
	bool parseJSON(char* json, std::string* message); //the json to parse, a pointer to return the message

};