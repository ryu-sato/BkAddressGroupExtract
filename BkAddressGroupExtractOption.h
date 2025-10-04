#ifndef __BK_ADDRESS_GROUP_EXTRACT_OPTION_H__
#define __BK_ADDRESS_GROUP_EXTRACT_OPTION_H__

#include <windows.h>
#include <windowsx.h>

enum ADDRESSFORMAT { AF_ADDRESS = 0,
                     AF_NAME_AND_ADDRESS = 1,
                     AF_ADDRESS_AND_NAME = 2,
                     AF_POISON};
extern int g_nEnableExtractAddressGroup;
extern enum ADDRESSFORMAT g_extractAddressFormat;

BOOL CALLBACK OptionDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam , LPARAM lParam);

#endif __BK_ADDRESS_GROUP_EXTRACT_OPTION_H__