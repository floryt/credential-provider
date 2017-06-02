#pragma once
#include <iostream>
#include "dbugLog.h"
#include "HTTPSclient.h"
#include "config.h"

enum EXIT_TYPE
{
	default,

	//--------trying to connect:---------
	cant_connect_to_server,
	connection_to_server_succeeded,

	//---------trying to authenticate:------
	authentication_succeeded,
	access_denied,
	bad_request,

	time_out

};

enum POST_STEP
{
	obtain_user_identity,
	obtain_admin_permission
};

class FirebaseCommunication
{
public:
	FirebaseCommunication();
	EXIT_TYPE TryToConnect(LPCWSTR username);
	EXIT_TYPE AuthenticationPost(LPCWSTR username, LPCWSTR* recived_message, POST_STEP step);
	EXIT_TYPE AuthenticationPostMock(LPCWSTR username, LPCWSTR* recived_message, POST_STEP step);
	EXIT_TYPE TryToConnectMock();
	~FirebaseCommunication();
	void ThreadWait(std::string req_type, char* data, std::string &packet_buffer, bool& is_Error); //POST/GET, json, a poiner to buffer
	std::wstring s2ws(const std::string& s); //string to LPCWSTR

private:
	bool cv;
	HTTPclient* http;


};
