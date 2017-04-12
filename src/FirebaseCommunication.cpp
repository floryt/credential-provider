#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <windows.h>
#include <thread>
#include "FirebaseCommunication.h"
#include <atlbase.h>
#include <atlconv.h>


FirebaseCommunication::FirebaseCommunication(std::string path)
{
	_log = new Logger(path);
	_log->Write("Constracture", "--------------------------------------------------");
	http = new HTTPclient(_log);
	cv = false;

}

EXIT_TYPE FirebaseCommunication::TryToConnect()
{
	//post - "ping" to check if server is a avalible

	EXIT_TYPE to_return;

	_log->Write("TryToConnect", "attempting to connect");

	//------------------------------ping------------------------------

	//setting the data
	std::string answer = "_"; //so if the request failed we will se "_" in the log
	std::string type = "GET";
	char* data = ""; //there is no data for GET
	bool isError = false;


	//initial CV
	cv = false;


	//starting thread
	std::thread tempt(&FirebaseCommunication::ThreadWait, this, type, data, std::ref(answer), std::ref(isError));
	tempt.detach();


	//waiting for response 
	while (cv == false)
	{
		_log->Write("TryToConnect", "no response yet");
		Sleep(1000);
	}

	//---after reciving response
	std::string ans(answer);
	_log->Write("TryToConnect", "recived: " + ans);

	if (isError)
	{
		_log->Write("TryToConnect", "could'nt connect");
		to_return = cant_connect_to_server;
	}
	else
	{
		
		//processig the data
		if (strcmp(answer.c_str(), "OK") == 0)
		{
			_log->Write("TryToConnect", "connection succeeded");
			to_return = connection_to_server_succeeded;
		}
		else
		{
			_log->Write("TryToConnect", "connection failed");
			to_return = cant_connect_to_server;
		}
	}

	


	return to_return;
}

EXIT_TYPE FirebaseCommunication::TryToAuthenticate(LPCWSTR username, bool is_guest, LPCWSTR* recived_message) //recived_message: a pointer to enter the message from admin
{
	//post with json

	EXIT_TYPE to_return;

	(*recived_message) = L"_";

	_log->Write("TryToAuthenticate", "attempting to Authenticate");

	//------------------------------post------------------------------

	//setting the data
	std::string answer = "__";
	cv = false;
	std::string type = "POST";
	bool isError = false;
	char* data = _strdup(http->createJson(username, is_guest)); //"{\"username\":\"Steven\",\"computerUID\":\"123456789\",\"guest\":false}"; //from const char* to char*
	std::string tempd(data);
	_log->Write("TryToAuthenticate", "created json: " + tempd + "length: "+ std::to_string(tempd.length()));
	
	if (tempd.length() > 0) //to prevent exception
	{
		if (data[0] == ' ') //fixing bug
		{
			data[0] = '{';
			_log->Write("TryToAuthenticate", "[FIXED] fixed json: " + std::string(data));
		}
	}
	else
	{
		_log->Write("TryToAuthenticate", "[ERROR] json is empty");
	}
	


	//initial CV
	cv = false;


	//starting thread
	std::thread tempt(&FirebaseCommunication::ThreadWait, this, type, strdup(data), std::ref(answer), std::ref(isError));
	tempt.detach();


	//waiting for response 
	while (cv == false)
	{
		_log->Write("TryToAuthenticate", "no response yet");
		Sleep(1000);
	}


	//---after reciving response
	std::string ans(answer);
	_log->Write("TryToAuthenticate", "got response: " + ans);


	if (isError) //it could be a timeout for example
	{
		if (strcmp((answer.substr(0, 11)).c_str(), "12002") == 0)
		{
			to_return = time_out;
			_log->Write("TryToAuthenticate", "detedcet timeout");
		}
		else
		{
			to_return = bad_request; //it's not a bad request but it will be handled the same way
			_log->Write("TryToAuthenticate", "detedcet some error in request");
		}
	}
	else
	{

		
		//processig the data
		if (strcmp((answer.substr(0, 11)).c_str(), "Bad Request") == 0)
		{
			_log->Write("TryToConnect", "connection failed: Bad request");
			to_return = bad_request;
		}
		else //we got an actual data
		{
			std::string* message = new std::string();
			(*message) = "";
			bool isAccess = http->parseJSON((char*)ans.c_str(), message);

			//--parsing the message
			std::wstring stemp = s2ws(*message);
			LPCWSTR result = stemp.c_str();
			(*recived_message) = result;
			_log->Write("TryToAuthenticate", "message from admin: " + (*message));

			if (isAccess) //access = true
			{

				_log->Write("TryToAuthenticate", "connection succeeded");
				to_return = authentication_succeeded;
			}
			else
			{
				_log->Write("TryToAuthenticate", "access denied");
				to_return = access_denied;
			}

		}
	}


	return to_return;
}

std::wstring FirebaseCommunication::s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

FirebaseCommunication::~FirebaseCommunication()
{
	_log->Write("~FirebaseCommunication", "distractor");
	delete _log;
	delete http;
}


Logger* FirebaseCommunication::GetLog()
{
	return _log;
}

void FirebaseCommunication::ThreadWait(std::string req_type, char* data, std::string &packet_buffer, bool& is_Error)
{
	char* ans;
	bool isError = false;
	if (req_type == "GET")
	{
		ans = http->GET(isError);
		std::string str(ans);
		packet_buffer = ans;
	}
	else if (req_type == "POST")
	{
		ans = http->POST(data, isError);
		std::string str(ans);
		packet_buffer = ans;
	}
	else //not a valid request
	{
		packet_buffer = "";
	}

	is_Error = isError;

	cv = true;
}

