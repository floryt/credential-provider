#include "HTTPSclient.h"
#include <atlbase.h>
#include <atlconv.h>
#include <string>

HTTPclient::HTTPclient(Logger* log)
{
	_log = log;
}

//if returned nullpter - error
char* HTTPclient::GET(bool& isError)
{
	isError = false;
	HINTERNET hIntSession = InternetOpen(_T("MyApp"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

	//---------timeout:
	DWORD rec_timeout = 1000 * 3;					// TODO: change time.
	InternetSetOption(hIntSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &rec_timeout, sizeof(rec_timeout));
	//-------------------------

	HINTERNET hHttpSession = InternetConnect(hIntSession, _T("us-central1-fir-communication-bce3d.cloudfunctions.net"), INTERNET_DEFAULT_HTTPS_PORT, _T(""), _T(""), INTERNET_SERVICE_HTTP, 0, 0);

	HINTERNET hHttpRequest = HttpOpenRequest(
		hHttpSession,
		_T("GET"),
		_T("/DllCommunication"),
		HTTP_VERSION, 0, 0, INTERNET_FLAG_SECURE, 0);


	//---for get:
	TCHAR* szHeaders = _T("Content-Type: text/html\nMySpecialHeder: whatever");
	CHAR szReq[1024] = "";

	CHAR szBuffer[1025];

	if (!HttpSendRequest(hHttpRequest, szHeaders, _tcslen(szHeaders), szReq, strlen(szReq))) {
		DWORD dwErr = GetLastError();
		
		/// handle error
		_itoa_s(dwErr, szBuffer, 10);
		isError = true;
		if (dwErr == 12002)
		{
			_log->Write("HTTPclient: GET", "[NO_CONNECTION] request timed out");
			
		}
		else
		{
			_log->Write("HTTPclient: GET", "[NO_CONNECTION] error in http request");
		}

	}
	else
	{

		DWORD dwRead = 0;
		while (::InternetReadFile(hHttpRequest, szBuffer, sizeof(szBuffer) - 1, &dwRead) && dwRead) {
			szBuffer[dwRead] = 0;
			OutputDebugStringA(szBuffer);
			dwRead = 0;
		}
	}



	InternetCloseHandle(hHttpRequest);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hIntSession);

	return szBuffer;
}

char* HTTPclient::POST(char* json, bool& isError)
{
	isError = false;

	HINTERNET hIntSession = InternetOpen(_T("MyApp"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

	//---------timeout:
	DWORD rec_timeout = 1000 * 10; // TODO: change time.
	InternetSetOption(hIntSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &rec_timeout, sizeof(rec_timeout));
	//-------------------------

	HINTERNET hHttpSession = InternetConnect(hIntSession, _T("us-central1-fir-communication-bce3d.cloudfunctions.net"), INTERNET_DEFAULT_HTTPS_PORT, _T(""), _T(""), INTERNET_SERVICE_HTTP, 0, 0);

	HINTERNET hHttpRequest = HttpOpenRequest(
		hHttpSession,
		_T("POST"),
		_T("/DllCommunication"),
		HTTP_VERSION, 0, 0, INTERNET_FLAG_SECURE, 0);


	//---for post:
	TCHAR* szHeaders = _T("Content-Type: application/json\r\n");
	CHAR* szReq = json;// "{\"username\":\"Steven\",\"computerUID\":\"123456789\",\"guest\":false}";

	_log->Write("HTTPclient: POST", std::string(json));

	CHAR szBuffer[1025];

	if (!HttpSendRequest(hHttpRequest, szHeaders, _tcslen(szHeaders), szReq, strlen(szReq))) {
		DWORD dwErr = GetLastError();

		/// handle error
		isError = true;
		_itoa_s(dwErr, szBuffer,10);
		if (dwErr == 12002)
		{
			_log->Write("HTTPclient: POST", "[NO_RESPONSE] request timed out");

		}
		else
		{
			_log->Write("HTTPclient: POST", "[NO_RESPONSE] error in http request");
		}

	}
	else
	{
		DWORD dwRead = 0;
		while (::InternetReadFile(hHttpRequest, szBuffer, sizeof(szBuffer) - 1, &dwRead) && dwRead) {
			szBuffer[dwRead] = 0;
			OutputDebugStringA(szBuffer);
			dwRead = 0;
		}
	}


	InternetCloseHandle(hHttpRequest);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hIntSession);

	return szBuffer;
}

const char* HTTPclient::createJson(LPCWSTR username, bool is_guest)
{
	//"{\"username\":\"Steven\",\"computerUID\":\"123456789\",\"guest\":false}";
	std::string json = "";
	json += "{\"username\":"; //{\"username\:"

	json += "\"";
	json += CT2A(username);
	json += "\",\"computerUID\":\"";

	json += std::to_string(123456789); //TODO: add identifier

	json += "\",\"guest\":";

	json += is_guest ? "true" : "false";

	json += "}";

	_log->Write("HTTPclient::createJson", json);

	return json.c_str();

}

bool HTTPclient::parseJSON(char* json, std::string* message)
{
	bool to_return = false;

	//{"access":true,"message":"Custom massage from admin"};
	std::string json_s(json);
	std::string from_admin = "";


	if (json_s.length() >= 14) //so there won't be exception
	{
		if (strcmp((json_s.substr(0, 14)).c_str(), "{\"access\":true") == 0)
		{
			to_return = true;
		}
	}
	int i = 0;
	if (!to_return)
	{
		from_admin += "access denied: ";
		i = 27;
	}
	else
	{
		from_admin += "enter password, ";
		i = 26;
	}
	

	for (; i < json_s.length() && json_s[i] != '\"'; i++)
	{
		from_admin += json_s[i];
	}

	*message = from_admin;


	//*message = json_s;
	return to_return;

}