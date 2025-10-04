////////////////////////////////////////////////////////////////////////////////////
// Template file for plugin.
//
// You can modify and redistribute this file without any permission.
//
// Note:
// Create a sub folder under "PlugInSDK" folder. e.g. "PlugInSDK\MyProject\" and
// place your project files there.

#include <windows.h>
#include <tchar.h>
#include <string>
#include <fstream>
#include "BkAddressGroupExtract.h"
#include "BkAddressGroupExtractOption.h"

#define BK_IDC_COMPWND_TO		0x0000292F
#define BK_IDC_COMPWND_CC		0x00002930
#define BK_IDC_COMPWND_BCC		0x00002931
#define BK_IDC_COMPWND_SUBJECT	0x00002932
#define BK_IDC_COMPWND_REPLY_TO	0x00002926
#define BK_IDC_COMPWND_SENDER	0x00002927

WNDPROC g_lpfnDefaultComposeWindowProc;
CBeckyAPI bka; // You can have only one instance in a project.
HINSTANCE g_hInstance = NULL;
char szIni[_MAX_PATH+2]; // Ini file to save your plugin settings.

/////////////////////////////////////////////////////////////////////////////
// DLL entry point
BOOL APIENTRY DllMain( HANDLE hModule, 
					   DWORD  ul_reason_for_call, 
					   LPVOID lpReserved
					 )
{
	g_hInstance = (HINSTANCE)hModule;
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				if (!bka.InitAPI()) {
					return FALSE;
				}
				GetModuleFileName((HINSTANCE)hModule, szIni, _MAX_PATH);
				LPSTR lpExt = strrchr(szIni, '.');
				if (lpExt) {
					strcpy(lpExt, ".ini");
				} else {
					// just in case
					strcat(szIni, ".ini");
				}
			}
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks from Becky!

#ifdef __cplusplus
extern "C"{
#endif

////////////////////////////////////////////////////////////////////////
// Called when the program is started and the main window is created.
int WINAPI BKC_OnStart()
{
	/*
	Since BKC_OnStart is called after Becky!'s main window is
	created, at least BKC_OnMenuInit with BKC_MENU_MAIN is called
	before BKC_OnStart. So, do not assume BKC_OnStart is called
	prior to any other callback.
	*/
	g_nEnableExtractAddressGroup =
		GetPrivateProfileInt(_T("Option"), _T("ENABLE_EXTRACT_ADDRESS_GROUP"), 1, szIni) % 2;
	g_extractAddressFormat = (ADDRESSFORMAT)
		(GetPrivateProfileInt(_T("Option"), _T("EXTRACT_ADDRESS_FORMAT"), AF_ADDRESS, szIni) % AF_POISON);

	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when the main window is closing.
int WINAPI BKC_OnExit()
{
	// Return -1 if you don't want to quit.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when menu is intialized.
int WINAPI BKC_OnMenuInit(HWND hWnd, HMENU hMenu, int nType)
{
	switch (nType) {
	case BKC_MENU_MAIN:
		{
			/* Sample of adding menu items
			HMENU hSubMenu = GetSubMenu(hMenu, 4);
			// Define CmdProc as "void WINAPI CmdProc(HWND, LPARAM)"
			UINT nID = bka.RegisterCommand("Information about this Command", nType,CmdProc);
			AppendMenu(hSubMenu, MF_STRING, nID, "&Menu item");
			*/
			/* If needed, you can register the command UI update callback.
			// Define CmdUIProc as "UINT WINAPI CmdUIProc(HWND, LPARAM)"
			bka.RegisterUICallback(nID, CmdUIProc);
			*/
		}
		break;
	case BKC_MENU_LISTVIEW:
		break;
	case BKC_MENU_TREEVIEW:
		break;
	case BKC_MENU_MSGVIEW:
		break;
	case BKC_MENU_MSGEDIT:
		break;
	case BKC_MENU_TASKTRAY:
		break;
	case BKC_MENU_COMPOSE:
		break;
	case BKC_MENU_COMPEDIT:
		break;
	case BKC_MENU_COMPREF:
		break;
	default:
		break;
	}
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a folder is opened.
int WINAPI BKC_OnOpenFolder(LPCTSTR lpFolderID)
{
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a mail is selected.
int WINAPI BKC_OnOpenMail(LPCTSTR lpMailID)
{
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called every minute.
int WINAPI BKC_OnEveryMinute()
{
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a compose windows is opened.
int WINAPI BKC_OnOpenCompose(HWND hWnd, int nMode/* See COMPOSE_MODE_* in BeckyApi.h */)
{
	switch (nMode) {
	case COMPOSE_MODE_COMPOSE1:   // 新規メッセージの作成
		if (g_nEnableExtractAddressGroup != 0) {
			ExtractGroupAddressInHeader(hWnd, _T("To"));
			ExtractGroupAddressInHeader(hWnd, _T("Cc"));
			ExtractGroupAddressInHeader(hWnd, _T("Bcc"));
		}
		break;

	case COMPOSE_MODE_COMPOSE2:   // 返信アドレスへのメール作成
	case COMPOSE_MODE_COMPOSE3:   // 選択アドレスへのメール作成
	case COMPOSE_MODE_TEMPLATE:   // テンプレートの作成／編集
	case COMPOSE_MODE_REPLY1:     // 返信
	case COMPOSE_MODE_REPLY2:     // 全員へ返信
	case COMPOSE_MODE_REPLY3:     // 選択されたアドレスへ返信
	case COMPOSE_MODE_FORWARD1:   // 転送
	case COMPOSE_MODE_FORWARD2:   // リダイレクト転送
	case COMPOSE_MODE_FORWARD3:   // 添付ファイルとして転送
		break;
	}

	g_lpfnDefaultComposeWindowProc = (WNDPROC) GetWindowLong(hWnd, GWL_WNDPROC);
	SetWindowLong(hWnd, GWL_WNDPROC, (LONG) HookComposeWindowProc);

	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when the composing message is saved.
int WINAPI BKC_OnOutgoing(HWND hWnd, int nMode/* 0:SaveToOutbox, 1:SaveToDraft, 2:SaveToReminder*/) 
{
	if (g_nEnableExtractAddressGroup != 0) {
		ExtractGroupAddressInHeader(hWnd, _T("To"));
		ExtractGroupAddressInHeader(hWnd, _T("Cc"));
		ExtractGroupAddressInHeader(hWnd, _T("Bcc"));
	}

	// Return -1 if you do not want to send it yet.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a key is pressed.
int WINAPI BKC_OnKeyDispatch(HWND hWnd, int nKey/* virtual key code */, int nShift/* Shift state. 0x40=Shift, 0x20=Ctrl, 0x60=Shift+Ctrl, 0xfe=Alt*/)
{
	// Return TRUE if you want to suppress subsequent command associated to this key.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a message is retrieved and saved to a folder
int WINAPI BKC_OnRetrieve(LPCTSTR lpMessage/* Message source*/, LPCTSTR lpMailID/* Mail ID*/)
{
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when a message is spooled
int WINAPI BKC_OnSend(LPCTSTR lpMessage/* Message source */)
{
	// Return BKC_ONSEND_PROCESSED, if you have processed this message
	// and don't need Becky! to send it.
	// Becky! will move this message to Sent box when the sending
	// operation is done.
	// CAUTION: You are responsible for the destination of this
	// message if you return BKC_ONSEND_PROCESSED.

	// Return BKC_ONSEND_ERROR, if you want to cancel the sending operation.
	// You are responsible for displaying an error message.

	// Return 0 to proceed the sending operation.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when all messages are retrieved
int WINAPI BKC_OnFinishRetrieve(int nNumber/* Number of messages*/)
{
	// Always return 0.
	return 0;
}


////////////////////////////////////////////////////////////////////////
// Called when plug-in setup is needed.
int WINAPI BKC_OnPlugInSetup(HWND hWnd)
{
	INT_PTR ret = DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_OPTION), hWnd, (DLGPROC) OptionDialogProc);
	if (ret == IDOK) {
		TCHAR extractAddressFormat[32] = "";
		WritePrivateProfileString(_T("Option"), _T("ENABLE_EXTRACT_ADDRESS_GROUP"),
			(g_nEnableExtractAddressGroup != 0 ? _T("1") : _T("0")), szIni);
		_itoa_s(g_extractAddressFormat, extractAddressFormat, sizeof(extractAddressFormat));
		WritePrivateProfileString(_T("Option"), _T("EXTRACT_ADDRESS_FORMAT"),
			extractAddressFormat, szIni);
	}
	// Return nonzero if you have processed.
	// return 1;
	return 1;
}


////////////////////////////////////////////////////////////////////////
// Called when plug-in information is being retrieved.
typedef struct tagBKPLUGININFO
{
	char szPlugInName[80]; // Name of the plug-in
	char szVendor[80]; // Name of the vendor
	char szVersion[80]; // Version string
	char szDescription[256]; // Short description about this plugin
} BKPLUGININFO, *LPBKPLUGININFO;

int WINAPI BKC_OnPlugInInfo(LPBKPLUGININFO lpPlugInInfo)
{
	/* You MUST specify at least szPlugInName and szVendor.
	   otherwise Becky! will silently ignore your plug-in.
	*/
	strcpy(lpPlugInInfo->szPlugInName, "Becky! Address Group Extract");
	strcpy(lpPlugInInfo->szVendor, "tatsurou");
	strcpy(lpPlugInInfo->szVersion, "1.0");
	strcpy(lpPlugInInfo->szDescription, "Extract Address Group to Mail Address List");
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when drag and drop operation occurs.
int WINAPI BKC_OnDragDrop(LPCSTR lpTgt, LPCSTR lpSrc, int nCount, int dropEffect)
{
	/*
	lpTgt:	A folder ID of the target folder.
			You can assume it is a root mailbox, if the string
			contains only one '\' character.
	lpSrc:	Either a folder ID or mail IDs. Multiple mail IDs are
			separated by '\n' (0x0a).
			You can assume it is a folder ID, if the string
			doesn't contain '?' character.
	nCount:	Number of items to be dropped.
			It can be more than one, if you drop mail items.
	dropEffect: Type of drag and drop operation
			1: Copy
			2: Move
			4: Link (Used for filtering setup in Becky!)
	*/
	// If you want to cancel the default drag and drop action,
	// return -1;
	// Do not assume the default action (copy, move, etc.) is always
	// processed, because other plug-ins might cancel the operation.
	return 0;
}


////////////////////////////////////////////////////////////////////////
// Called when a message was retrieved and about to be filtered.
int WINAPI BKC_OnBeforeFilter2(LPCSTR lpMessage, LPCSTR lpMailBox, int* lpnAction, char** lppParam)
{
	/*
    lpMessage: A complete message source, which ends with
    "<CRLF>.<CRLF>".
    lpnAction:	[out] Returns the filtering action to be applied.
    	#define ACTION_NOTHING		-1	// Do nothing
		#define ACTION_MOVEFOLDER	0	// Move to a folder
		#define ACTION_COLORLABEL	1	// Set the color label
		#define ACTION_SETFLAG		2	// Set the flag
		#define ACTION_SOUND		3	// Make a sound
		#define ACTION_RUNEXE		4	// Run executable file
		#define ACTION_REPLY		5	// Reply to the message
		#define ACTION_FORWARD		6	// Forward the message
		#define ACTION_LEAVESERVER	7	// Leave/delete on the server.
		#define ACTION_ADDHEADER	8	// Add "X-" header to the top of the message. (Plug-in only feature)
	lpMailBox: ID of the mailbox that is retrieving the message. (XXXXXXXX.mb\)
	lppParam:	[out] Returns the pointer to the filtering parameter string.
		ACTION_MOVEFOLDER:	Folder name.
							e.g. XXXXXXXX.mb\FolderName\
							or \FolderName\ (begin with '\') to use
							the mailbox the message belongs.
		ACTION_COLORLABEL:	Color code(BGR) in hexadecimal. e.g. 0088FF
		ACTION_SETFLAG:		"F" to set flag. "R" to set read.
		ACTION_SOUND:		Name of the sound file.
		ACTION_RUNEXE:		Command line to execute. %1 will be replaced with the path to the file that contains the entire message.
		ACTION_REPLY:		Path to the template file without extension.
								e.g. #Reply\MyReply
		ACTION_FORWARD:		Path to the template file without extension. + "*" + Address to forward.
								e.g. #Forward\MyForward*mail@address
									 *mail@address (no template)
		ACTION_LEAVESERVER:	The string consists of one or two decimals. The second decimal is optional.
							Two decimals must be separated with a space.
							First decimal	1: Delete the message from the server.
											0: Leave the message on the server.
							Second decimal	1: Do not store the message to the folder.
											0: Store the message to the folder. (default action)
							e.g. 0 (Leave the message on the server.)
								 1 1 (Delete the message on the server and do not save. (Means KILL))
		ACTION_ADDHEADER	"X-Header:data" that will be added at the top of the incoming message.
							You can specify multiple headers by separating CRLF, but each header must
							begin with "X-". e.g. "X-Plugindata1: test\r\nX-Plugindata2: test2";
	*/
	
	/* Return values
	BKC_FILTER_DEFAULT	Do nothing and apply default filtering rules.
	BKC_FILTER_PASS		Apply default filtering rules after applying the rule it returns.
	BKC_FILTER_DONE		Do not apply default rules.
	BKC_FILTER_NEXT		Request Becky! to call this callback again so that another rules can be added.
	*/
    return BKC_FILTER_DEFAULT;
}

#ifdef __cplusplus
}
#endif

/**
 * アドレスグループ名であるか判定
 */
bool IsAddressGroup(const TCHAR *szAddress, CBeckyAPI bkapi) {
	bool isAddressGroup = false;
	TCHAR *szDupAddress = NULL, *szExtractedAddresses = NULL;
	TCHAR *p1 = NULL, *p2 = NULL;

	/* アドレスグループ展開前後の文字列を確保 */
	szDupAddress = _tcsdup(szAddress);
	szExtractedAddresses = _tcsdup(bkapi.SerializeRcpts(szAddress));

	/* アドレスの前後に挿入されたスペースを削除 */
	if (_tcslen(szDupAddress) > 0) {
		p1 = szDupAddress + _tcslen(szDupAddress) - 1;
		while (p1 != szDupAddress && isspace(*p1)) { *p1 = _T('\0'); p1--; }
	}
	p1 = szDupAddress;
	while (*p1 != _T('\0') && *p1 == _T(' ')) p1++;

	/* アドレスの前後に挿入されたスペースを削除 */
	if (_tcslen(szExtractedAddresses) > 0) {
		p2 = szExtractedAddresses + _tcslen(szExtractedAddresses) - 1;
		while (p2 != szExtractedAddresses && isspace(*p2)) { *p2 = _T('\0'); p2--; }
	}
	p2 = szExtractedAddresses;
	while (*p2 != _T('\0') && *p2 == _T(' ')) p2++;

	isAddressGroup = (_tcscmp(p1, p2) != 0);

	/* 確保した文字列を解放する */
	free(szDupAddress);
	free(szExtractedAddresses);

	return isAddressGroup;
}

/**
 * アドレスを整形する
 */
void FormatAddress(AddressBook *const addressBook, AddressRecord *addressData, void *data = NULL)
{
	CPointerList *list = (CPointerList *) data;
	std::string strFormattedAddress;
	switch (g_extractAddressFormat) {
	case AF_ADDRESS:
		strFormattedAddress = addressData->GetEMailAddress();
		break;
	case AF_NAME_AND_ADDRESS:
		strFormattedAddress.append(addressData->GetName());
		strFormattedAddress.append(_T(" <"));
		strFormattedAddress.append(addressData->GetEMailAddress());
		strFormattedAddress.append(_T(">"));
		break;
	case AF_ADDRESS_AND_NAME:
		strFormattedAddress.append(addressData->GetEMailAddress());
		strFormattedAddress.append(_T(" ("));
		strFormattedAddress.append(addressData->GetName());
		strFormattedAddress.append(_T(")"));
		break;
	}
	list->AddTail((LPSTR) strFormattedAddress.c_str());
}

/**
 * アドレスグループを展開する
 */
int ExtractGroupAddress(const TCHAR *szGroupAddress, TCHAR *szBuffer,
						size_t nMaxBufferLen, enum ADDRESSFORMAT format) {
	/* csv 形式のアドレスをリスト化する */
	TCHAR *szDupGroupAddress = _tcsdup(szGroupAddress);
	CPointerList listTargetAddress;
	LPSTR lpTok = TokenAddr((LPSTR) szGroupAddress);
	while (lpTok) {
		listTargetAddress.AddTail(lpTok);
		lpTok = TokenAddr(NULL);
	}
	free(szDupGroupAddress);

	/* アドレスリストにあるアドレスグループを展開する */
	CPointerList *listExtractedAddress = new CPointerList();
	for (CPointerItem *item = listTargetAddress.GetTop(); item != NULL; item = item->GetNext()) {
		if (!IsAddressGroup(item->GetData(), bka)) {
			listExtractedAddress->AddTail(item->GetData());
		} else {
			AddressBook addressBook(bka.GetDataFolder(), item->GetData());
			addressBook.Lookup(bka.SerializeRcpts(szGroupAddress), FormatAddress, listExtractedAddress);
		}
	}

	/* 展開したアドレスリストを csv 形式にする */
	std::string strExtractedAddress = "";
	for (CPointerItem *item = listExtractedAddress->GetTop(); item != NULL; item = item->GetNext()) {
		strExtractedAddress.append(item->GetData());
		if (item->GetNext() != NULL) {
			strExtractedAddress.append(", ");
		}
	}
	delete listExtractedAddress;
	return _tcscpy_s(szBuffer, nMaxBufferLen, strExtractedAddress.c_str());
}

/**
 * 任意のメールヘッダーに設定されたアドレスグループを展開する
 */
void ExtractGroupAddressInHeader(HWND hWnd, TCHAR *lpHeaderName) {
	struct _DLGITEMMAP {
		const TCHAR *lpHeaderName;
		UINT nItemID;
	} map[] = {
		{ _T("To"),  BK_IDC_COMPWND_TO },
		{ _T("Cc"),  BK_IDC_COMPWND_CC },
		{ _T("Bcc"), BK_IDC_COMPWND_BCC },
	};
	UINT nLimit = 0;
	for (int i = 0; i < sizeof (map) / sizeof(map[0]); i++) {
		if (_tcscmp(map[i].lpHeaderName, lpHeaderName) == 0) {
			nLimit = SendDlgItemMessage(hWnd, map[i].nItemID, EM_GETLIMITTEXT, 0, 0);
			break;
		}
	}
	if (nLimit == 0) {
		return;
	}
	TCHAR *szAddress = (TCHAR *) calloc(nLimit + 1, sizeof (TCHAR));
	TCHAR *szExtractAddress = (TCHAR *) calloc(nLimit + 1, sizeof (TCHAR));
	if (szAddress == NULL || szExtractAddress == NULL) {
		return;
	}

	bka.CompGetSpecifiedHeader(hWnd, lpHeaderName, szAddress, nLimit + 1);
	ExtractGroupAddress(szAddress, szExtractAddress, nLimit + 1,
		                g_extractAddressFormat);
	bka.CompSetSpecifiedHeader(hWnd, lpHeaderName, szExtractAddress);
}

/**
 * メール作成ウィンドウのメッセージプロシージャ
 */
LRESULT CALLBACK HookComposeWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UINT nMsgID = HIWORD(wParam);
	UINT nItemID = LOWORD(wParam);
	HWND hDialogItem = (HWND) lParam;

	switch(uMsg) {
	case WM_COMMAND:
		if (wParam == IDOK && g_nEnableExtractAddressGroup != 0) {
			CallWindowProc(g_lpfnDefaultComposeWindowProc, hWnd, uMsg, wParam, lParam);
			ExtractGroupAddressInHeader(hWnd, _T("To"));
			ExtractGroupAddressInHeader(hWnd, _T("Cc"));
			ExtractGroupAddressInHeader(hWnd, _T("Bcc"));
			return 0;
		}

		switch(nMsgID) {
		case EN_CHANGE:
			if (hDialogItem == GetFocus()) break;
			// through bellow
		case EN_KILLFOCUS:
			if (!(nItemID == BK_IDC_COMPWND_TO
				  || nItemID == BK_IDC_COMPWND_CC
				  || nItemID == BK_IDC_COMPWND_BCC)) {
				break;
			}

			if (g_nEnableExtractAddressGroup != 0) {
				UINT nMsgLimit = SendMessage(hDialogItem, EM_GETLIMITTEXT, 0, 0);
				TCHAR *szBuf = (TCHAR *) calloc(nMsgLimit + 1, sizeof(TCHAR));
				TCHAR *szExtractAddress = (TCHAR *) calloc(nMsgLimit + 1, sizeof(TCHAR));
				if (szBuf == NULL || szExtractAddress == NULL) {  // メモリ確保失敗
					break;
				}

				GetWindowText(hDialogItem, szBuf, nMsgLimit + 1);
				ExtractGroupAddress(szBuf, szExtractAddress, nMsgLimit + 1,
					                g_extractAddressFormat);
				SetWindowText(hDialogItem, szExtractAddress);
			}
			return 0;
		}
		break;
	}
	return CallWindowProc(g_lpfnDefaultComposeWindowProc, hWnd, uMsg, wParam, lParam);
}
