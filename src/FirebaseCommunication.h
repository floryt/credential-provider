#pragma once
#include <iostream>
#include "Logger.h"
#include "HTTPSclient.h"

enum EXIT_TYPE
{
	default,

	//-----------trying to connect:---------
	cant_connect_to_server,
	connection_to_server_succeeded,

	//---------trying to authenticate:------
	authentication_succeeded,
	access_denied,
	bad_request,

	time_out

};

class FirebaseCommunication
{
public:
	FirebaseCommunication(std::string path);
	EXIT_TYPE TryToConnect();
	EXIT_TYPE TryToAuthenticate(LPCWSTR username, bool is_guest, LPCWSTR* recived_message);
	~FirebaseCommunication();
	Logger* GetLog();
	void ThreadWait(std::string req_type, char* data, std::string &packet_buffer, bool& is_Error); //POST/GET, json, a poiner to buffer
	std::wstring s2ws(const std::string& s); //string to LPCWSTR

private:
	Logger* _log;
	bool cv;
	HTTPclient* http;


};
