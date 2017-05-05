#include "HTTPSclient.h"
#include "jsonParser.h"
#include <atlbase.h>
#include <atlconv.h>
#include <string>

HTTPclient::HTTPclient(Logger* log, ConfigParser* config)
{
	_log = log;
	_config = config;
}

//if returned nullpter - error
char* HTTPclient::GET(bool& isError, char* firebase_function)
{
	//------config variebles---
	std::string text;
	std::wstring stemp;
	LPCWSTR config_result;
	//-------------------------

	isError = false;

	//***********getting text from config*********
	text = _config->GetVal("tryingToConnect");
	stemp = std::wstring(text.begin(), text.end()); //CASTING: string to lpcwstr
	config_result = stemp.c_str();
	//********************************************

	HINTERNET hIntSession = InternetOpen(config_result, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

	//---------timeout:

	//***********getting text from config*********
	text = _config->GetVal("getTimeOut");
	//********************************************

	DWORD rec_timeout = 1000 * stoi(text);					
	InternetSetOption(hIntSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &rec_timeout, sizeof(rec_timeout));
	//-------------------------

	//***********getting text from config*********
	text = _config->GetVal("firebasePath");
	stemp = std::wstring(text.begin(), text.end()); //CASTING: string to lpcwstr
	config_result = stemp.c_str();
	//********************************************

	HINTERNET hHttpSession = InternetConnect(hIntSession, config_result, INTERNET_DEFAULT_HTTPS_PORT, _T(""), _T(""), INTERNET_SERVICE_HTTP, 0, 0);

	//CASTING: char* to LPWSTR
	wchar_t wtext[50];
	mbstowcs(wtext, firebase_function, strlen(firebase_function) + 1);//Plus null
	LPCWSTR function = wtext;

	HINTERNET hHttpRequest = HttpOpenRequest(
		hHttpSession,
		_T("GET"),
		function,
		HTTP_VERSION, 0, 0, INTERNET_FLAG_SECURE, 0);


	//---for get:
	TCHAR* szHeaders = _T("Content-Type: text/html\nMySpecialHeder: credentialProvider");
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

//firebase_function example: "/DllCommunication"
char* HTTPclient::POST(char* json, bool& isError, char* firebase_function)
{
	//------config variebles---
	std::string text;
	std::wstring stemp;
	LPCWSTR config_result;
	//-------------------------

	isError = false;


	//***********getting text from config*********
	text = _config->GetVal("tryingToConnect");
	stemp = std::wstring(text.begin(), text.end()); //CASTING: string to lpcwstr
	config_result = stemp.c_str();
	//********************************************

	HINTERNET hIntSession = InternetOpen(config_result, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

	//---------timeout:

	//***********getting text from config*********
	text = _config->GetVal("postTimeOut");
	//********************************************
	DWORD rec_timeout = 1000 * stoi(text);
	InternetSetOption(hIntSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &rec_timeout, sizeof(rec_timeout));

	//-------------------------


	//***********getting text from config*********
	text = _config->GetVal("firebasePath");
	stemp = std::wstring(text.begin(), text.end()); //CASTING: string to lpcwstr
	config_result = stemp.c_str();
	//********************************************

	HINTERNET hHttpSession = InternetConnect(hIntSession, config_result, INTERNET_DEFAULT_HTTPS_PORT, _T(""), _T(""), INTERNET_SERVICE_HTTP, 0, 0);

	//CASTING: char* to LPWSTR
	wchar_t wtext[50];
	mbstowcs(wtext, firebase_function, strlen(firebase_function) + 1);//Plus null
	LPCWSTR function = wtext;

	HINTERNET hHttpRequest = HttpOpenRequest(
		hHttpSession,
		_T("POST"),
		function,
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

const char* HTTPclient::createJson(LPCWSTR user_email)
{
	//"{\"username\":\"Steven\",\"computerUID\":\"123456789\",\"guest\":false}";

	//TODO - make json modular.
	std::unordered_map<std::string, std::string> map_json;
	map_json.insert(std::pair<std::string, std::string>("\"email\"","\"" + std::string(CT2A(user_email)) + "\""));  //CASTING: lpswtsr to string
	std::string uid= "\"";
	uid += "ii5tekf43attq9prnma2ik8s8c";
	uid += "\"";
	map_json.insert(std::pair<std::string, std::string>("\"computerUID\"",uid)); //TODO: add identifier
	//map_json.insert(std::pair<std::string, std::string>("\"isGuest\"", is_guest ? "true" : "false"));

	return jsonParser::CreateJson(map_json).c_str();


}

bool HTTPclient::parseJSON(char* json, std::string* message)
{


	bool to_return = false;


	//{"access":true,"message":"Custom massage from admin"};
	std::string json_s(json);
	std::string from_admin = "";

	std::map<std::string, std::string> map_json= jsonParser::ParseJson(json);

	std::map<std::string, std::string>::iterator it;

	it = map_json.find("access"); //TODO: config here the file
	if (it != map_json.end())
	{
		if (it->second.compare("true") == 0)
		{
			to_return = true;
		}
	}

	it = map_json.find("message"); //TODO: config here
	if (it != map_json.end())
	{
		from_admin += it->second;
	}


	*message = from_admin;


	return to_return;

}