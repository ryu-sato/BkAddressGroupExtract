#pragma once

#include "AddressBook.h"
#include "PlugInSDK/BeckyAPI.h"
#include "PlugInSDK/BkCommon.h"
#include "resource.h"

extern WNDPROC g_lpfnDefaultComposeWindowProc;

LRESULT CALLBACK HookComposeWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int ExtractGroupAddress(const TCHAR *szGroupAddress, TCHAR *szBuffer, size_t nBufferLen,
						enum ADDRESSFORMAT);
bool IsAddressGroup(const TCHAR *szAddress, CBeckyAPI bkapi);
void ExtractGroupAddressInHeader(HWND hWnd, TCHAR *lpHeaderName);

