
#include <windows.h>
#include "resource.h"


////////////////////////////////////////////////////////////////////////////////////
// Template file for plugin.
//
// You can modify and redistribute this file without any permission.
//
// Note:
// Create a sub folder under "PlugInSDK" folder. e.g. "PlugInSDK\MyProject\" and
// place your project files there.

#include "../BeckyAPI.h"
#include "../BkCommon.h"

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
	// Always return 0.
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when the composing message is saved.
int WINAPI BKC_OnOutgoing(HWND hWnd, int nMode/* 0:SaveToOutbox, 1:SaveToDraft, 2:SaveToReminder*/) 
{
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
	// Return nonzero if you have processed.
	// return 1;
	return 0;
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
	strcpy(lpPlugInInfo->szPlugInName, "Becky2! resource dll sample.");
	strcpy(lpPlugInInfo->szVendor, "RimArts, Inc.");
	strcpy(lpPlugInInfo->szVersion, "1.0");
	strcpy(lpPlugInInfo->szDescription, "Resouce dll sample.");
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
// Called when certain image resources are requested.
// Uncomment the statements in each case condition if you want to override 
// the default image resources.
// To return the resource identifier, assign a resource name to lppResourceName,
// or simply return a integer resource identifier as a return value.

/* IMPORTANT!

 Check nImage paramater first, and if your bitmap contains less images than that, you MUST NOT
 return your resource. This is very important to avoid the compatibility problem that can occur
 in future releases.

 You can modify the image in the sample resource file, but you can not change the size and order
 of the bitmap.

 You can, however, modify the button size of toolbar resources, although you can not change
 the order and the assigned command IDs for buttons.
 If you want to use 256 (or more) color bitmap for toolbars, note that Microsoft Visual Studio's
 resource editor does not allow you to edit 256 (or more) color bitmaps.
 However, you can directly edit the bmp files that are associated to toolbar resources. In that case,
 you may need to delete *.aps file in your project, because this file caches original 16 bit bitmaps and
 sometimes it will overwrite the bitmap file you have edited by external tools.

 If you want to create 256 (or more) color images, you might want to use some painting tool other
 than Microsoft Visual Studio's resource editor, which only provides 16 colors tool. You can directly load
 bmp files, that are associated to the resource, into those painting tools.
 Note that some colors are reserved for the background color for some bitmap resources. For example,
 RGB(255,0,255) is reserved as the background color for tree/list view icons, and RGB(192,192,192) for all
 toolbar bitmaps. Be careful not to destroy those background colors.
 If you want to create high-color or true-color images, be aware that those images can be viewed correctly
 only in the machines that support those colors. I would recommend that you stick with 256 colors at most.

 If more than one plug-ins respond this callback, the first response will be used.
 So, the author should inform the users that they might need to disable some other resource
 plug-ins in "General Setup" > "Advanced" > "Plug-Ins".
 It is strongly recommended that you add "_b2icon_" prefix to your plug-in's file name
 so that users can distinguish those resource-oriented plug-ins in the plug-in list quickly.
 e.g. _b2icon_abc.dll

*/
int WINAPI BKC_OnRequestResource(int nType, int nImages, char** lppResourceName)
{
	switch (nType) {

	// Bitmap resources.
	case BKC_BITMAP_ADDRESSBOOKICON:
		// Icons for the tree view of the address book.
		// 16 * 16 * nFixed
		// background = RGB(255,0,255) (bright purple)

		/* Uncomment to activate
		if (nImages <= 6) {
			*lppResourceName = "ADDRESSBOOKICON";
		}
		*/
		break;
	case BKC_BITMAP_ADDRESSPERSON:
		// Icons for the list view of the address book.
		// 16 * 16 * nFixed
		// background = RGB(255,0,255) (bright purple)

		/* Uncomment to activate
		if (nImages <= 7) {
			*lppResourceName = "ADDRESSPERSON";
		}
		*/
		break;
	case BKC_BITMAP_ANIMATION:
		// Bitmap animation at the top-right of the main window.
		// 30 * 30 * nVariable
	
		/* Uncomment to activate
		// You can ignore "nImages" for this resource.
		*lppResourceName = "ANIMATION";
		*/

		break;
	case BKC_BITMAP_FOLDERCOLOR:
		// Selection of the colored folder icon, which are extracted from "BKC_BITMAP_FOLDERICON".
		// 16 * 16 * nFixed
		// background = RGB(255,0,255) (bright purple)

		/* Uncomment to activate
		if (nImages <= 5) {
			*lppResourceName = "FOLDERCOLOR";
		}
		*/
		break;
	case BKC_BITMAP_FOLDERICON:
		// Icons for the tree view of the main window.
		// 16 * 16 * nFixed
		// background = RGB(255,0,255) (bright purple)

		/* Uncomment to activate
		if (nImages <= 40) {
			*lppResourceName = "FOLDERICON";
		}
		*/
		break;
	case BKC_BITMAP_LISTICON:
		// Icons for the list view of the main window.
		// 32 * 15 * nFixed
		// background = RGB(255,0,255) (bright purple)

		/* Uncomment to activate
		if (nImages <= 21) {
			*lppResourceName = "LISTICON";
		}
		*/
		break;
	case BKC_BITMAP_LISTICON2:
		// Additional icons for the list view of the main window.
		// 32 * 15 * nFixed
		// background = RGB(255,0,255) (bright purple)

		/* Uncomment to activate
		if (nImages <= 21) {
			*lppResourceName = "LISTICON2";
		}
		*/
		break;
	case BKC_BITMAP_PRIORITYSTAMP:
		// Icons for selecting priority in the composing window.
		// 18 * 20 * nFixed

		/* Uncomment to activate
		if (nImages <= 5) {
			*lppResourceName = "PRIORITYSTAMP";
		}
		*/
		break;
	case BKC_BITMAP_RULETREEICON:
		// Icons for the tree view of the filtering manager.
		// 16 * 16 * nFixed
		// background = RGB(255,0,255) (bright purple)

		/* Uncomment to activate
		if (nImages <= 6) {
			*lppResourceName = "RULETREEICON";
		}
		*/
		break;
	case BKC_BITMAP_TEMPLATEFOLDER:
		// Icons for the tree view of the template selecting dialog.
		// 16 * 16 * nFixed
		// background = RGB(255,0,255) (bright purple)

		/* Uncomment to activate
		if (nImages <= 6) {
			*lppResourceName = "TEMPLATEFOLDER";
		}
		*/
		break;
	case BKC_BITMAP_WHATSNEWLIST:
		// Icons for the list in "What's New" list.
		// 16 * 16 * nFixed
		// background = RGB(255,0,255) (bright purple)

		/* Uncomment to activate
		if (nImages <= 2) {
			*lppResourceName = "WHATSNEWLIST";
		}
		*/
		break;

	// Icon resources.
	case BKC_ICON_ADDRESSBOOK:
		// An icon for the address book.
		// 32 * 32
		// 16 * 16

		/* Uncomment to activate
		*lppResourceName = "ADDRESSBOOK";
		*/
		break;
	case BKC_ICON_ANIMATION1_SMALL:
		// An icon for the task tray animation(progress) 1
		// 16 * 16

		/* Uncomment to activate
		*lppResourceName = "ANIMATION1_SMALL";
		*/
		break;
	case BKC_ICON_ANIMATION2_SMALL:
		// An icon for the task tray animation(progress) 2
		// 16 * 16

		/* Uncomment to activate
		*lppResourceName = "ANIMATION2_SMALL";
		*/
		break;
	case BKC_ICON_COMPOSEFRAME:
		// An icon for the composing window.
		// 32 * 32
		// 16 * 16

		/* Uncomment to activate
		*lppResourceName = "COMPOSEFRAME";
		*/
		break;
	case BKC_ICON_MAINFRAME:
		// An icon for the main window.
		// 32 * 32
		// 16 * 16

		/* Uncomment to activate
		*lppResourceName = "MAINFRAME";
		*/
		break;
	case BKC_ICON_NEWARRIVAL1_SMALL:
		// An icon for the task tray animation(new messages) 1
		// 16 * 16

		/* Uncomment to activate
		*lppResourceName = "NEWARRIVAL1_SMALL";
		*/
		break;
	case BKC_ICON_NEWARRIVAL2_SMALL:
		// An icon for the task tray animation(new messages) 2
		// 16 * 16

		/* Uncomment to activate
		*lppResourceName = "NEWARRIVAL2_SMALL";
		*/
		break;

	// Toolbar resources.
	// Here goes the sample that uses integer resource IDs instead of resource names.
	case BKC_TOOLBAR_ADDRESSBOOK:
		// A toolbar for the address book.
		// background = RGB(192,192,192) (light gray)

		/* Uncomment to activate
		if (nImages <= 9) {
			return IDR_ADDRESSBOOK;
		}
		*/
		break;
	case BKC_TOOLBAR_COMPOSEFRAME:
		// A toolbar for the composing window.
		// background = RGB(192,192,192) (light gray)

		/* Uncomment to activate
		if (nImages <= 22) {
			return IDR_COMPOSEFRAME;
		}
		*/
		break;
	case BKC_TOOLBAR_HTMLEDITOR:
		// A toolbar for the WYSWYG HTML editor.
		// background = RGB(192,192,192) (light gray)

		/* Uncomment to activate
		if (nImages <= 17) {
			return IDR_HTMLEDITOR;
		}
		*/
		break;
	case BKC_TOOLBAR_MAINFRAME:
		// A toolbar for the main window.
		// background = RGB(192,192,192) (light gray)

		/* Uncomment to activate
		if (nImages <= 37) {
			return IDR_MAINFRAME;
		}
		*/
		break;
	default:
		break;
	}

	return 0;
}

#ifdef __cplusplus
}
#endif
