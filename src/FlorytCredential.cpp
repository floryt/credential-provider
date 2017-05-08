//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#ifndef WIN32_NO_STATUS

#include "Logger.h"
#include <thread> 
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif
#include <unknwn.h>
#include "FlorytCredential.h"
#include "guid.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include "Logger.h"
#include <atlbase.h>
#include <atlconv.h>


FlorytCredential::FlorytCredential() :
	_cRef(1),
	_pCredProvCredentialEvents(nullptr),
	_pszUserSid(nullptr),
	_pszQualifiedUserName(nullptr),
	_fIsLocalUser(false),
	_fChecked(false),
	_fShowControls(false),
	_dwComboIndex(0)
{
	DllAddRef();

	ZeroMemory(_rgCredProvFieldDescriptors, sizeof(_rgCredProvFieldDescriptors));
	ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
	ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));
}

FlorytCredential::~FlorytCredential()
{
	if (_rgFieldStrings[SFI_PASSWORD])
	{
		size_t lenPassword = wcslen(_rgFieldStrings[SFI_PASSWORD]);
		SecureZeroMemory(_rgFieldStrings[SFI_PASSWORD], lenPassword * sizeof(*_rgFieldStrings[SFI_PASSWORD]));
	}
	for (int i = 0; i < ARRAYSIZE(_rgFieldStrings); i++)
	{
		CoTaskMemFree(_rgFieldStrings[i]);
		CoTaskMemFree(_rgCredProvFieldDescriptors[i].pszLabel);
	}
	CoTaskMemFree(_pszUserSid);
	CoTaskMemFree(_pszQualifiedUserName);
	DllRelease();
}

// Initializes one credential with the field information passed in.
// Set the value of the SFI_LARGE_TEXT field to pwzUsername.
HRESULT FlorytCredential::Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	_In_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR const *rgcpfd,
	_In_ FIELD_STATE_PAIR const *rgfsp,
	_In_ ICredentialProviderUser *pcpUser)
{
	HRESULT hr = S_OK;
	_cpus = cpus;

	GUID guidProvider;
	pcpUser->GetProviderID(&guidProvider);
	_fIsLocalUser = (guidProvider == Identity_LocalUserProvider);

	// Copy the field descriptors for each field. This is useful if you want to vary the field
	// descriptors based on what Usage scenario the credential was created for.
	for (DWORD i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(_rgCredProvFieldDescriptors); i++)
	{
		_rgFieldStatePairs[i] = rgfsp[i];
		hr = FieldDescriptorCopy(rgcpfd[i], &_rgCredProvFieldDescriptors[i]);
	}

	//BAR: the texts shown in the fileds
	// Initialize the String value of all the fields.
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Floryt", &_rgFieldStrings[SFI_APP_NAME]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"", &_rgFieldStrings[SFI_EMAIL]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"", &_rgFieldStrings[SFI_PASSWORD]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Submit", &_rgFieldStrings[SFI_SUBMIT_BUTTON]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Click here to connect", &_rgFieldStrings[SFI_CONNECT]);
	}
	if (SUCCEEDED(hr))
	{
		hr = pcpUser->GetStringValue(PKEY_Identity_QualifiedUserName, &_pszQualifiedUserName);
	}
	if (SUCCEEDED(hr))
	{
		PWSTR pszUserName;
		pcpUser->GetStringValue(PKEY_Identity_UserName, &pszUserName); //BAR: getting the username
		if (pszUserName != nullptr)
		{
			wchar_t szString[256];
			StringCchPrintf(szString, ARRAYSIZE(szString), L"%s", pszUserName);
			hr = SHStrDupW(szString, &_rgFieldStrings[SFI_USERNAME]); //BAR: put the username in one of the fileds
			CoTaskMemFree(pszUserName);
		}
		else
		{
			hr = SHStrDupW(L"User Name is NULL", &_rgFieldStrings[SFI_USERNAME]);
		}
	}
	if (SUCCEEDED(hr))
	{
		PWSTR pszLogonStatus;
		pcpUser->GetStringValue(PKEY_Identity_LogonStatusString, &pszLogonStatus);
		if (pszLogonStatus != nullptr)
		{
			wchar_t szString[256];
			StringCchPrintf(szString, ARRAYSIZE(szString), L"Logon Status: %s", pszLogonStatus);
			hr = SHStrDupW(szString, &_rgFieldStrings[SFI_LOGONSTATUS_TEXT]);
			CoTaskMemFree(pszLogonStatus);
		}
		else
		{
			hr = SHStrDupW(L"Logon Status is NULL", &_rgFieldStrings[SFI_LOGONSTATUS_TEXT]);
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = pcpUser->GetSid(&_pszUserSid);
	}

	return hr;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of anything.
HRESULT FlorytCredential::Advise(_In_ ICredentialProviderCredentialEvents *pcpce)
{
	if (_pCredProvCredentialEvents != nullptr)
	{
		_pCredProvCredentialEvents->Release();
	}
	return pcpce->QueryInterface(IID_PPV_ARGS(&_pCredProvCredentialEvents));
	
}

// LogonUI calls this to tell us to release the callback.
HRESULT FlorytCredential::UnAdvise()
{
	if (_pCredProvCredentialEvents)
	{
		_pCredProvCredentialEvents->Release();
	}
	_pCredProvCredentialEvents = nullptr;
	return S_OK;
}

// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the
// field definitions. But if you want to do something
// more complicated, like change the contents of a field when the tile is
// selected, you would do it here.
HRESULT FlorytCredential::SetSelected(_Out_ BOOL *pbAutoLogon)
{
	*pbAutoLogon = FALSE;
	return S_OK;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. The most common thing to do here (which we do below)
// is to clear out the password field.
HRESULT FlorytCredential::SetDeselected()
{
	HRESULT hr = S_OK;
	if (_rgFieldStrings[SFI_PASSWORD])
	{
		size_t lenPassword = wcslen(_rgFieldStrings[SFI_PASSWORD]);
		SecureZeroMemory(_rgFieldStrings[SFI_PASSWORD], lenPassword * sizeof(*_rgFieldStrings[SFI_PASSWORD]));

		CoTaskMemFree(_rgFieldStrings[SFI_PASSWORD]);
		hr = SHStrDupW(L"", &_rgFieldStrings[SFI_PASSWORD]);

		if (SUCCEEDED(hr) && _pCredProvCredentialEvents)
		{
			_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, _rgFieldStrings[SFI_PASSWORD]);
		}
	}

	return hr;
}

// Get info for a particular field of a tile. Called by logonUI to get information
// to display the tile.
HRESULT FlorytCredential::GetFieldState(DWORD dwFieldID,
	_Out_ CREDENTIAL_PROVIDER_FIELD_STATE *pcpfs,
	_Out_ CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE *pcpfis)
{
	HRESULT hr;

	// Validate our parameters.
	if ((dwFieldID < ARRAYSIZE(_rgFieldStatePairs)))
	{
		*pcpfs = _rgFieldStatePairs[dwFieldID].cpfs;
		*pcpfis = _rgFieldStatePairs[dwFieldID].cpfis;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}
	return hr;
}

// Sets ppwsz to the string value of the field at the index dwFieldID
HRESULT FlorytCredential::GetStringValue(DWORD dwFieldID, _Outptr_result_nullonfailure_ PWSTR *ppwsz)
{
	HRESULT hr;
	*ppwsz = nullptr;

	// Check to make sure dwFieldID is a legitimate index
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors))
	{
		// Make a copy of the string and return that. The caller
		// is responsible for freeing it.
		hr = SHStrDupW(_rgFieldStrings[dwFieldID], ppwsz);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Get the image to show in the user tile
HRESULT FlorytCredential::GetBitmapValue(DWORD dwFieldID, _Outptr_result_nullonfailure_ HBITMAP *phbmp)
{
	HRESULT hr = S_OK;
	*phbmp = nullptr;

	if ((SFI_TILEIMAGE == dwFieldID))
	{
		HBITMAP hbmp = LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_TILE_IMAGE));
		if (hbmp != nullptr)
		{
			hr = S_OK;
			*phbmp = hbmp;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets pdwAdjacentTo to the index of the field the submit button should be
// adjacent to. We recommend that the submit button is placed next to the last
// field which the user is required to enter information in. Optional fields
// should be below the submit button.
HRESULT FlorytCredential::GetSubmitButtonValue(DWORD dwFieldID, _Out_ DWORD *pdwAdjacentTo)
{
	HRESULT hr;

	if (SFI_SUBMIT_BUTTON == dwFieldID)
	{
		// pdwAdjacentTo is a pointer to the fieldID you want the submit button to
		// appear next to.
		*pdwAdjacentTo = SFI_PASSWORD;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}
	return hr;
}

// Sets the value of a field which can accept a string as a value.
// This is called on each keystroke when a user types into an edit field
HRESULT FlorytCredential::SetStringValue(DWORD dwFieldID, _In_ PCWSTR pwz)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
		(CPFT_EDIT_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft ||
			CPFT_PASSWORD_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		PWSTR *ppwszStored = &_rgFieldStrings[dwFieldID];
		CoTaskMemFree(*ppwszStored);
		hr = SHStrDupW(pwz, ppwszStored);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Not used in our case, but must be due to interface demmands
HRESULT FlorytCredential::GetCheckboxValue(DWORD dwFieldID, _Out_ BOOL *pbChecked, _Outptr_result_nullonfailure_ PWSTR *ppwszLabel)
{
	HRESULT hr = S_OK;

	return hr;
}

// Sets whether the specified checkbox is checked or not.
HRESULT FlorytCredential::SetCheckboxValue(DWORD dwFieldID, BOOL bChecked)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
		(CPFT_CHECKBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		_fChecked = bChecked;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Not used in our case, but must be due to interface demmands
HRESULT FlorytCredential::GetComboBoxValueCount(DWORD dwFieldID, _Out_ DWORD *pcItems, _Deref_out_range_(< , *pcItems) _Out_ DWORD *pdwSelectedItem)
{
	HRESULT hr = S_OK;

	return hr;
}


// Not used in our case, but must be due to interface demmands
HRESULT FlorytCredential::GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, _Outptr_result_nullonfailure_ PWSTR *ppwszItem)
{
	HRESULT hr = S_OK;

	return hr;
}

// Called when the user changes the selected item in the combobox.
HRESULT FlorytCredential::SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
		(CPFT_COMBOBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		_dwComboIndex = dwSelectedItem;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}


void FlorytCredential::display_dynamic(SAMPLE_FIELD_ID field_id)
{
	_pCredProvCredentialEvents->BeginFieldUpdates();

	_pCredProvCredentialEvents->SetFieldState(nullptr, field_id, CPFS_DISPLAY_IN_SELECTED_TILE);

	_pCredProvCredentialEvents->EndFieldUpdates();
}

void FlorytCredential::hide_dynamic(SAMPLE_FIELD_ID field_id)
{
	_pCredProvCredentialEvents->BeginFieldUpdates();

	_pCredProvCredentialEvents->SetFieldState(nullptr, field_id, CPFS_HIDDEN);

	_pCredProvCredentialEvents->EndFieldUpdates();
}

void FlorytCredential::change_label_text(LPCWSTR text)
{
	_pCredProvCredentialEvents->BeginFieldUpdates();

	_pCredProvCredentialEvents->SetFieldString(nullptr, SFI_LOGONSTATUS_TEXT, text);

	_pCredProvCredentialEvents->EndFieldUpdates();
}

bool FlorytCredential::post_step(POST_STEP step, FirebaseCommunication* server) //true - continue steps. false - stop.
{
	bool to_return = false;
	EXIT_TYPE exit = default;


	LPCWSTR* recived_message = new LPCWSTR();
	exit = server->AuthenticationPost(_rgFieldStrings[SFI_EMAIL], recived_message, step);

	if (exit == access_denied)
	{
		//change_label_text(_rgFieldStrings[SFI_EDIT_TEXT]);
		change_label_text(*recived_message);
		display_dynamic(SFI_CONNECT);
		display_dynamic(SFI_EMAIL);

	}
	else if (exit == authentication_succeeded)
	{
		//---------OK to move to next step-------
		to_return = true;

		if (step == obtain_admin_permission) //because step1 does'nt need to change anything yet (it will be changed in the next step
		{
			change_label_text(*recived_message); //final step - need to change.
		}
		
	}
	else if (exit == bad_request)
	{
		//***********getting text from config*********
		std::string text = _config->GetVal("badRequest");
		std::wstring stemp = std::wstring(text.begin(), text.end()); //CASTING: string to lpcwstr
		LPCWSTR result = stemp.c_str();
		//********************************************
		change_label_text(result);

		display_dynamic(SFI_CONNECT);
		display_dynamic(SFI_EMAIL);


	}
	else if (exit == time_out)
	{
		change_label_text(L"the request timed out");
		display_dynamic(SFI_CONNECT);
		display_dynamic(SFI_EMAIL);

	}
	return to_return;
}


bool FlorytCredential::connection_to_server()
{
	//-----------------------------------------------------------------------
	
	Logger* log = new Logger("C:\\Users\\User\\Desktop\\log.txt"); //TODO- path from registry
	log->Write("connection_to_server", "--------------------------------------------------");

	_config = new ConfigParser("C:\\Users\\User\\Desktop\\config.txt", log);
	_config->Parse();

	bool to_return = false;
	bool hr = false;

	FirebaseCommunication* server = new FirebaseCommunication(log, _config);

	hide_dynamic(SFI_CONNECT);
	hide_dynamic(SFI_EMAIL);
	display_dynamic(SFI_LOGONSTATUS_TEXT);
	

	EXIT_TYPE exit = default;

	//***********getting text from config*********
	std::string text = _config->GetVal("tryingToConnect");
	std::wstring stemp = std::wstring(text.begin(), text.end()); //CASTING: string to lpcwstr
	LPCWSTR result = stemp.c_str();
	//********************************************

	change_label_text(result);



	exit = server->TryToConnect();

	if (exit == cant_connect_to_server)
	{
		display_dynamic(SFI_LOGONSTATUS_TEXT);

		//***********getting text from config*********
		std::string text = _config->GetVal("connectionToServerFailed");
		std::wstring stemp = std::wstring(text.begin(), text.end()); //CASTING: string to lpcwstr
		LPCWSTR result = stemp.c_str();
		//********************************************
		change_label_text(result);

		display_dynamic(SFI_CONNECT);
		display_dynamic(SFI_EMAIL);
		
	}
	else if (exit == connection_to_server_succeeded)
	{

		//***********getting text from config*********
		std::string text = _config->GetVal("requestUserToTypeYes");
		std::wstring stemp = std::wstring(text.begin(), text.end()); //CASTING: string to lpcwstr
		LPCWSTR result = stemp.c_str();
		//********************************************

		change_label_text(result);

		hr = post_step(obtain_user_identity, server);
		if(hr)
		{
			//***********getting text from config*********
			std::string text = _config->GetVal("requestAdminPermission");
			std::wstring stemp = std::wstring(text.begin(), text.end()); //CASTING: string to lpcwstr
			LPCWSTR result = stemp.c_str();
			//********************************************
			change_label_text(result);

			hr = post_step(obtain_admin_permission, server);
			if(hr)
			{
				//---------proccess succeeded-------

				//~~~getting username~~~
				PWSTR field_username = _rgFieldStrings[SFI_USERNAME];
				char str_username[256];
				wcstombs(str_username, field_username, 256);
				log->Write("connecting_to_server", "username: " + std::string(str_username));


				//~~~generating password~~~
				std::string password = "12345"; //TODO: do random


				//~~~changing password~~~
				change_password(log, str_username, password);
				CoTaskMemFree(field_username);


				//~~~putting password in field~~~
				std::wstring stemp = std::wstring(password.begin(), password.end()); //CASTING: string to lpcwstr
				LPCWSTR result = stemp.c_str();
				_pCredProvCredentialEvents->BeginFieldUpdates();
				_pCredProvCredentialEvents->SetFieldString(nullptr, SFI_PASSWORD, result);
				_pCredProvCredentialEvents->EndFieldUpdates();


				to_return = true;
			}
		}
	}

	delete server;

	delete _config;

	return to_return;

}



// Called when the user clicks a command link.
HRESULT FlorytCredential::CommandLinkClicked(DWORD dwFieldID)
{
	HRESULT hr = S_OK;

	CREDENTIAL_PROVIDER_FIELD_STATE cpfsShow = CPFS_HIDDEN;

	// Validate parameter.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
		(CPFT_COMMAND_LINK == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		HWND hwndOwner = nullptr;
		switch (dwFieldID)
		{
		case SFI_CONNECT:
			if (connection_to_server()) //if the connection has succeeded and the user has been authenticated
			{
				//show other fileds
				_pCredProvCredentialEvents->BeginFieldUpdates();
				cpfsShow = CPFS_DISPLAY_IN_SELECTED_TILE; //status SHOW
				
				_pCredProvCredentialEvents->SetFieldState(nullptr, SFI_PASSWORD, cpfsShow);
				_pCredProvCredentialEvents->SetFieldState(nullptr, SFI_SUBMIT_BUTTON, cpfsShow);

				_pCredProvCredentialEvents->SetFieldState(nullptr, SFI_CONNECT, CPFS_HIDDEN); //hiding the first authentication

				//_pCredProvCredentialEvents->SetFieldString(nullptr, SFI_HIDECONTROLS_LINK, _fShowControls ? L"Hide additional controls" : L"Show additional controls");

				//_pCredProvCredentialEvents->SetFieldString(nullptr, SFI_PASSWORD, L"hello"); //changing to password

				_pCredProvCredentialEvents->EndFieldUpdates();
				_fShowControls = !_fShowControls; //BAR: changing the state for next time



				SendEnter();
			}
			break;


		default:
			hr = E_INVALIDARG;
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Collect the username and password into a serialized credential for the correct usage scenario
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials
// back to the system to log on.
HRESULT FlorytCredential::GetSerialization(_Out_ CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE *pcpgsr,
	_Out_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpcs,
	_Outptr_result_maybenull_ PWSTR *ppwszOptionalStatusText,
	_Out_ CREDENTIAL_PROVIDER_STATUS_ICON *pcpsiOptionalStatusIcon)
{


	//BAR: we get a pointer to an object from winlogon who calls the foo. we then arrange the username and passwords as requested (hashing the passwords),
	//	   and requesting the LSA to load an authentication package. this "package" (actually a dll) will determine if to log the user into the system.
	HRESULT hr = E_UNEXPECTED;
	*pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
	*ppwszOptionalStatusText = nullptr;
	*pcpsiOptionalStatusIcon = CPSI_NONE;
	ZeroMemory(pcpcs, sizeof(*pcpcs));

	// For local user, the domain and user name can be split from _pszQualifiedUserName (domain\username).
	// CredPackAuthenticationBuffer() cannot be used because it won't work with unlock scenario.
	if (_fIsLocalUser)
	{
		/*BAR: a definiation from MSDN about authentication packages
		authentication package:
		A DLL that encapsulates the authentication logic used to determine whether to permit a user to log on.
		LSA authenticates a user logon by sending the request to an authentication package.
		The authentication package then examines the logon information and either authenticates or rejects the user logon attempt.
		*/

		PWSTR pwzProtectedPassword;
		hr = ProtectIfNecessaryAndCopyPassword(_rgFieldStrings[SFI_PASSWORD], _cpus, &pwzProtectedPassword); //BAR: copy string from passwordBox
		if (SUCCEEDED(hr))
		{
			PWSTR pszDomain;
			PWSTR pszUsername;
			hr = SplitDomainAndUsername(_pszQualifiedUserName, &pszDomain, &pszUsername);
			if (SUCCEEDED(hr))
			{
				KERB_INTERACTIVE_UNLOCK_LOGON kiul;
				hr = KerbInteractiveUnlockLogonInit(pszDomain, pszUsername, pwzProtectedPassword, _cpus, &kiul); //BAR: pack into kiul
				if (SUCCEEDED(hr))
				{
					// We use KERB_INTERACTIVE_UNLOCK_LOGON in both unlock and logon scenarios.  It contains a
					// KERB_INTERACTIVE_LOGON to hold the creds plus a LUID that is filled in for us by Winlogon
					// as necessary.
					hr = KerbInteractiveUnlockLogonPack(kiul, &pcpcs->rgbSerialization, &pcpcs->cbSerialization); //BAR: re-arrange the strings (?)
					if (SUCCEEDED(hr))
					{
						ULONG ulAuthPackage;
						hr = RetrieveNegotiateAuthPackage(&ulAuthPackage); //BAR: requesting the LSA to build a package (to load a dll) anf gets the identifier of it. this package will determine whether to login the user or no9t, open logon session
						if (SUCCEEDED(hr))
						{
							pcpcs->ulAuthenticationPackage = ulAuthPackage;
							pcpcs->clsidCredentialProvider = CLSID_CSample;
							// At this point the credential has created the serialized credential used for logon
							// By setting this to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
							// that we have all the information we need and it should attempt to submit the
							// serialized credential.
							*pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED; //BAR: here the CP ends.
						}
					}
				}
				CoTaskMemFree(pszDomain);
				CoTaskMemFree(pszUsername);
			}
			CoTaskMemFree(pwzProtectedPassword);
		}
	}
	else
	{
		DWORD dwAuthFlags = CRED_PACK_PROTECTED_CREDENTIALS | CRED_PACK_ID_PROVIDER_CREDENTIALS;

		// First get the size of the authentication buffer to allocate
		if (!CredPackAuthenticationBuffer(dwAuthFlags, _pszQualifiedUserName, const_cast<PWSTR>(_rgFieldStrings[SFI_PASSWORD]), nullptr, &pcpcs->cbSerialization) &&
			(GetLastError() == ERROR_INSUFFICIENT_BUFFER))
		{
			pcpcs->rgbSerialization = static_cast<byte *>(CoTaskMemAlloc(pcpcs->cbSerialization));
			if (pcpcs->rgbSerialization != nullptr)
			{
				hr = S_OK;

				// Retrieve the authentication buffer
				if (CredPackAuthenticationBuffer(dwAuthFlags, _pszQualifiedUserName, const_cast<PWSTR>(_rgFieldStrings[SFI_PASSWORD]), pcpcs->rgbSerialization, &pcpcs->cbSerialization))
				{
					ULONG ulAuthPackage;
					hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
					if (SUCCEEDED(hr))
					{
						pcpcs->ulAuthenticationPackage = ulAuthPackage;
						pcpcs->clsidCredentialProvider = CLSID_CSample;

						// At this point the credential has created the serialized credential used for logon
						// By setting this to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
						// that we have all the information we need and it should attempt to submit the
						// serialized credential.
						*pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
					}
				}
				else
				{
					hr = HRESULT_FROM_WIN32(GetLastError());
					if (SUCCEEDED(hr))
					{
						hr = E_FAIL;
					}
				}

				if (FAILED(hr))
				{
					CoTaskMemFree(pcpcs->rgbSerialization);
				}
			}
			else
			{
				hr = E_OUTOFMEMORY;
			}
		}
	}
	return hr;
}

struct REPORT_RESULT_STATUS_INFO
{
	NTSTATUS ntsStatus;
	NTSTATUS ntsSubstatus;
	PWSTR     pwzMessage;
	CREDENTIAL_PROVIDER_STATUS_ICON cpsi;
};

static const REPORT_RESULT_STATUS_INFO s_rgLogonStatusInfo[] =
{
	{ STATUS_LOGON_FAILURE, STATUS_SUCCESS, L"Incorrect password or username. Stop the brute-force!", CPSI_ERROR, },
	{ STATUS_ACCOUNT_RESTRICTION, STATUS_ACCOUNT_DISABLED, L"The account is disabled.", CPSI_WARNING },
};

// ReportResult is completely optional.  Its purpose is to allow a credential to customize the string
// and the icon displayed in the case of a logon failure.  For example, we have chosen to
// customize the error shown in the case of bad username/password and in the case of the account
// being disabled.
HRESULT FlorytCredential::ReportResult(NTSTATUS ntsStatus,
	NTSTATUS ntsSubstatus,
	_Outptr_result_maybenull_ PWSTR *ppwszOptionalStatusText,
	_Out_ CREDENTIAL_PROVIDER_STATUS_ICON *pcpsiOptionalStatusIcon)
{
	*ppwszOptionalStatusText = nullptr;
	*pcpsiOptionalStatusIcon = CPSI_NONE;

	DWORD dwStatusInfo = (DWORD)-1;

	// Look for a match on status and substatus.
	for (DWORD i = 0; i < ARRAYSIZE(s_rgLogonStatusInfo); i++)
	{
		if (s_rgLogonStatusInfo[i].ntsStatus == ntsStatus && s_rgLogonStatusInfo[i].ntsSubstatus == ntsSubstatus)
		{
			dwStatusInfo = i;
			break;
		}
	}

	if ((DWORD)-1 != dwStatusInfo)
	{
		if (SUCCEEDED(SHStrDupW(s_rgLogonStatusInfo[dwStatusInfo].pwzMessage, ppwszOptionalStatusText)))
		{
			*pcpsiOptionalStatusIcon = s_rgLogonStatusInfo[dwStatusInfo].cpsi;
		}
	}

	// If we failed the logon, try to erase the password field.
	if (FAILED(HRESULT_FROM_NT(ntsStatus)))
	{
		if (_pCredProvCredentialEvents)
		{
			_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, L"");
		}
	}

	// Since nullptr is a valid value for *ppwszOptionalStatusText and *pcpsiOptionalStatusIcon
	// this function can't fail.
	return S_OK;
}

// Gets the SID of the user corresponding to the credential.
HRESULT FlorytCredential::GetUserSid(_Outptr_result_nullonfailure_ PWSTR *ppszSid)
{
	*ppszSid = nullptr;
	HRESULT hr = E_UNEXPECTED;
	if (_pszUserSid != nullptr)
	{
		hr = SHStrDupW(_pszUserSid, ppszSid);
	}
	// Return S_FALSE with a null SID in ppszSid for the
	// credential to be associated with an empty user tile.

	return hr;
}

// GetFieldOptions to enable the password reveal button and touch keyboard auto-invoke in the password field.
HRESULT FlorytCredential::GetFieldOptions(DWORD dwFieldID,
	_Out_ CREDENTIAL_PROVIDER_CREDENTIAL_FIELD_OPTIONS *pcpcfo)
{
	*pcpcfo = CPCFO_NONE;

	if (dwFieldID == SFI_PASSWORD)
	{
		*pcpcfo = CPCFO_ENABLE_PASSWORD_REVEAL;
	}
	return S_OK;
}