#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <windows.h>
#include <thread>
#include "FirebaseCommunication.h"
#include <atlbase.h>
#include <atlconv.h>
#include <vector>


FirebaseCommunication::FirebaseCommunication(Logger* log, ConfigParser* config)
{
	_log = log;
	_config = config;
	http = new HTTPclient(_log, _config);
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

	//***********getting text from config*********
	std::string firebase_function = _config->GetVal("GETcheckConnectionFooName");
	//********************************************

	char* data = ""; //there is no data for GET
	bool isError = false;


	//initial CV
	cv = false;


	//starting thread
	std::thread tempt(&FirebaseCommunication::ThreadWait, this, firebase_function, data, std::ref(answer), std::ref(isError));
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

EXIT_TYPE FirebaseCommunication::AuthenticationPostMock(LPCWSTR username, LPCWSTR* recived_message, POST_STEP step)
{
	*recived_message = L"my message";
	return authentication_succeeded;
}

EXIT_TYPE FirebaseCommunication::TryToConnectMock()
{
	return connection_to_server_succeeded;
}

//works for all types of requests we need here
EXIT_TYPE FirebaseCommunication::AuthenticationPost(LPCWSTR username, LPCWSTR* recived_message, POST_STEP step) //recived_message: a pointer to enter the message from admin
{
	//post with json

	EXIT_TYPE to_return;

	(*recived_message) = L"_";

	//no need of config - log purpose only.
	std::string mes = "no step";
	if (step == obtain_user_identity)
	{
		mes = "obtainIdentityVerification";
	}
	else if (step == obtain_admin_permission)
	{
		mes = "obtainAdminPermission";
	}
	_log->Write("AuthenticationPost", "attempting to Authenticate: step " + mes);

	//------------------------------post------------------------------

	//setting the data
	std::string answer = "__";
	cv = false;

	std::string firebase_function = "";
	if (step == obtain_user_identity)
	{
		//***********getting text from config*********
		firebase_function = _config->GetVal("POSTobtainUserIdentityFooName");
		//********************************************
	}
	else if (step == obtain_admin_permission)
	{
		//***********getting text from config*********
		firebase_function = _config->GetVal("POSTobtainAdminPermissionFooName");
		//********************************************
	}

	bool isError = false;
	char* data = _strdup(http->createJson(username)); //"{\"username\":\"Steven\",\"computerUID\":\"123456789\"}"; //from const char* to char*
	std::string tempd(data);
	_log->Write("AuthenticationPost", "created json: " + tempd + "length: "+ std::to_string(tempd.length()));
	
	if (tempd.length() > 0) //to prevent exception
	{
		if (data[0] == ' ') //fixing bug
		{
			data[0] = '{';
			_log->Write("AuthenticationPost", "[FIXED] fixed json: " + std::string(data));
		}
	}
	else
	{
		_log->Write("AuthenticationPost", "[ERROR] json is empty");
	}
	


	//initial CV
	cv = false;


	//starting thread
	std::thread tempt(&FirebaseCommunication::ThreadWait, this, firebase_function, strdup(data), std::ref(answer), std::ref(isError));
	tempt.detach();


	//waiting for response 
	while (cv == false)
	{
		_log->Write("AuthenticationPost", "no response yet");
		Sleep(1000);
	}


	//---after reciving response
	std::string ans(answer);
	_log->Write("AuthenticationPost", "got response: " + ans);


	if (isError) //it could be a timeout for example
	{
		if (strcmp((answer.substr(0, 11)).c_str(), "12002") == 0)
		{
			to_return = time_out;
			_log->Write("AuthenticationPost", "detedcet timeout");
		}
		else
		{
			to_return = bad_request; //it's not a bad request but it will be handled the same way
			_log->Write("AuthenticationPost", "detedcet some error in request");
		}
	}
	else
	{

		
		//processig the data
		if (strcmp((answer.substr(0, 11)).c_str(), "Bad Request") == 0)
		{
			_log->Write("AuthenticationPost", "connection failed: Bad request");
			to_return = bad_request;
		}
		else //we got an actual data
		{
			std::string* message = new std::string();
			(*message) = "";
			bool isAccess = http->parseJSON((char*)ans.c_str(), message);

			//--parsing the message
			/*std::wstring stemp = s2ws(*message);
			LPCWSTR result = stemp.c_str();*/
			if (message->size() == 2) //bug fix - message is ""
			{
				(*message) = ":)"; //recived no messgae. TODO- update field in struct
				_log->Write("AuthenticationPost", "detected message bug - recived empty message");
			}
			std::wstring stemp = std::wstring(message->begin(), message->end()); //CASTING: string to lpcwstr
			LPCWSTR result = stemp.c_str();
			(*recived_message) = result;
			_log->Write("AuthenticationPost", "message from admin: " + (*message));

			if (isAccess) //access = true
			{

				_log->Write("AuthenticationPost", "step succeeded");
				to_return = authentication_succeeded;
			}
			else
			{
				_log->Write("AuthenticationPost", "access denied");
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
	if (req_type == "/connectivityCheck") //TODO: send struct and return struct
	{
		ans = http->GET(isError, _strdup(req_type.c_str()));  //CASTING: string to char*
		std::string str(ans);
		packet_buffer = ans;
	}
	else if (req_type == "/obtainIdentityVerification" || req_type == "/obtainAdminPermission" || req_type == "/DLLmock") //TODO: get from config
	{
		std::vector<char> cstr(req_type.c_str(), req_type.c_str() + req_type.size() + 1);
		ans = http->POST(data, isError, _strdup(req_type.c_str()));
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

