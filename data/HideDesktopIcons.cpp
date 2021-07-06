/*

I decided to write this small code segment when investigating ransomware attacks. Although this isn't really novel, or difficult,
it is always fun to see COM implementations that aren't pain on the eyes. Anyway, the code segment below hides the Desktop icons 
from the users so an attacker could (in theory) present a background image with a ransom note. 

The code contains a bit of generic code e.g. the error handling I implement often (FAILURE goto statement), as well as
generic code for releasing our COM objects.


This doesn't have an accompanied paper because I am too tired to write. I fucking hate writing.

1luv
                                                                                                                  
                                           88  88                                                                 
                                           88  88                                                                 
                                           88  88                                                                 
,adPPYba,  88,dPYba,,adPYba,    ,adPPYba,  88  88  8b       d8                          8b       d8  8b,     ,d8  
I8[    ""  88P'   "88"    "8a  a8P_____88  88  88  `8b     d8'                          `8b     d8'   `Y8, ,8P'   
 `"Y8ba,   88      88      88  8PP"""""""  88  88   `8b   d8'                            `8b   d8'      )888(     
aa    ]8I  88      88      88  "8b,   ,aa  88  88    `8b,d8'                              `8b,d8'     ,d8" "8b,   
`"YbbdP"'  88      88      88   `"Ybbd8"'  88  88      Y88'                                 "8"      8P'     `Y8  
                                                       d8'                                                        
                                                      d8'     888888888888  888888888888                          

*/




#include <Windows.h>
#include <shlobj.h>
#include <atlcomcli.h>
#include <stdio.h>

DWORD InterfaceQueryDesktopView(REFIID Riid, PVOID* Pp);
DWORD Win32FromHResult(HRESULT Result);

int main(VOID)
{
	DWORD dwError = ERROR_SUCCESS;
	HRESULT Result = S_OK;
	IFolderView2* FolderView2 = NULL;

	if (CoInitialize(NULL) != S_OK)
		goto FAILURE;

	dwError = InterfaceQueryDesktopView(IID_PPV_ARGS(&FolderView2));
	if (dwError != ERROR_SUCCESS)
		goto FAILURE;

	dwError = 0;
	Result = FolderView2->GetCurrentFolderFlags(&dwError);
	if (!SUCCEEDED(Result))
		goto FAILURE;

	Result = FolderView2->SetCurrentFolderFlags(FWF_NOICONS, dwError ^ FWF_NOICONS);
	if (!SUCCEEDED(Result))
		goto FAILURE;

	if(FolderView2)
		FolderView2->Release();

	CoUninitialize();

	return ERROR_SUCCESS;

FAILURE:

	dwError = Win32FromHResult(Result);

	if (FolderView2)
		FolderView2->Release();

	CoUninitialize();

	return dwError;
}

DWORD InterfaceQueryDesktopView(REFIID Riid, PVOID* Pp)
{
	DWORD dwError = ERROR_SUCCESS;
	HRESULT Result = S_OK;
	IShellWindows* ShellWindows = NULL;
	IShellBrowser* Browser = NULL;
	IShellView* View = NULL;
	CComVariant Desktop(CSIDL_DESKTOP); //initialize
	CComVariant IDisposeObject;
	IDispatch* Dispatch = NULL;
	IServiceProvider* Provider = NULL;

	Result = CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_IShellWindows, (VOID**)(&ShellWindows));
	if (!SUCCEEDED(Result))
		return Win32FromHResult(Result);

	dwError = 0;
	Result = ShellWindows->FindWindowSW(&Desktop, &IDisposeObject, SWC_DESKTOP, (PLONG)&dwError, SWFO_NEEDDISPATCH, &Dispatch);
	if (!SUCCEEDED(Result))
		goto FAILURE;
	else
		dwError = ERROR_SUCCESS;

	Result = Dispatch->QueryInterface(IID_IServiceProvider, (VOID**)&Provider);
	if (!SUCCEEDED(Result))
		goto FAILURE;

	Result = Provider->QueryService(SID_STopLevelBrowser, IID_PPV_ARGS(&Browser));
	if (!SUCCEEDED(Result))
		goto FAILURE;

	Result = Browser->QueryActiveShellView(&View);
	if (!SUCCEEDED(Result))
		goto FAILURE;

	Result = View->QueryInterface(Riid, Pp);
	if (!SUCCEEDED(Result))
		goto FAILURE;
	
	if (Provider)
		Provider->Release();

	if (Browser)
		Browser->Release();

	if (View)
		View->Release();
	
	if(ShellWindows)
		ShellWindows->Release();

	return ERROR_SUCCESS;

FAILURE:

	dwError = Win32FromHResult(Result);

	if (Provider)
		Provider->Release();

	if (Browser)
		Browser->Release();

	if (View)
		View->Release();

	if (ShellWindows)
		ShellWindows->Release();

	return dwError;
}

DWORD Win32FromHResult(HRESULT Result)
{
	if ((Result & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0))
		return HRESULT_CODE(Result);

	if (Result == S_OK)
		return ERROR_SUCCESS;

	return ERROR_CAN_NOT_COMPLETE;
}