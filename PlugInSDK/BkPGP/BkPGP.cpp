// BkPGP.cpp
//
// You can modify and redistribute this file without any permission.

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>
#include <time.h>
#include "BkPGP.h"
#include "../BeckyAPI.h"
#include "../BkCommon.h"

CBeckyAPI bka;
// If you want to modify and redistribute this plug-in, please change the 
// following information.
char g_szVer[] = "Ver. 1.0 (RimArts)";
char g_szVendor[] = "RimArts, Inc.";

HINSTANCE g_hInstance = NULL;

int g_nPGPVer = 651;
BOOL g_bPGPMIME = TRUE;

char szIni[_MAX_PATH+2];

#define WM_SET_TRANSFER_SAFE	WM_USER+300 // Makes compose window to encode text in transfer safe form.
////////////////////////////////////////////////////////////////////////////
char szUserName[8194];
char szPassPhrase[8194];

////////////////////////////////////////////////////////////////////////////
// Dialog procs
BOOL CALLBACK PGPSetupProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg){
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			BOOL bSuccess;
			int n = GetDlgItemInt(hWnd, IDC_EDTPGPVERSION, &bSuccess, FALSE);
			if (bSuccess) {
				g_nPGPVer = n;
			}
			g_bPGPMIME = ((SendDlgItemMessage(hWnd, IDC_CHKPGPMIME, BM_GETCHECK, 0, 0) == BST_CHECKED)? TRUE:FALSE);

			char szVal[60];
			sprintf(szVal, "%d", g_nPGPVer);
			WritePrivateProfileString("Settings", "PGPVersion", szVal, szIni);
			sprintf(szVal, "%d", g_bPGPMIME);
			WritePrivateProfileString("Settings", "PGPMIME", szVal, szIni);
			EndDialog(hWnd, IDOK);
			return TRUE;
		} else
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		break;

	case WM_INITDIALOG:
		SetDlgItemInt(hWnd, IDC_EDTPGPVERSION, g_nPGPVer, FALSE);
		SendDlgItemMessage(hWnd, IDC_CHKPGPMIME, BM_SETCHECK, g_bPGPMIME, 0);
		return TRUE;

	default:
		break;
	}
	return FALSE;
}


BOOL CALLBACK PGPPassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg){
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			GetDlgItemText(hWnd, IDC_EDIT1, szPassPhrase, 8192);
			GetDlgItemText(hWnd, IDC_EDIT2, szUserName, 8192);
			EndDialog(hWnd, IDOK);
			return TRUE;
		} else
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		break;

	case WM_INITDIALOG:
		SetDlgItemText(hWnd, IDC_EDIT1, szPassPhrase);
		SetDlgItemText(hWnd, IDC_EDIT2, szUserName);
		return TRUE;

	default:
		break;
	}
	return FALSE;
}

LPSTR lpOutput;

BOOL CALLBACK PGPOutputProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg){
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hWnd, IDOK);
			return TRUE;
		} else
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		break;

	case WM_INITDIALOG:
		if (lpOutput) {
			SetDlgItemText(hWnd, IDC_EDIT1, lpOutput);
		}
		return TRUE;

	default:
		break;
	}
	return FALSE;
}

BOOL CALLBACK PGPUserProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg){
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			GetDlgItemText(hWnd, IDC_EDIT1, szUserName, 8192);
			EndDialog(hWnd, IDOK);
			return TRUE;
		} else
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		break;

	case WM_INITDIALOG:
		SetDlgItemText(hWnd, IDC_EDIT1, szUserName);
		return TRUE;

	default:
		break;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////
// Menu Callbacks
void WINAPI SetupPGP(HWND hWnd, LPARAM lParam)
{
	int nRC = DialogBox(g_hInstance,
			MAKEINTRESOURCE(IDD_PGP_SETUP), hWnd, (DLGPROC)PGPSetupProc);
}

//////////////////////////////////////////////////////////////////////////////////////
void WINAPI CmdPGP(HWND hWnd, LPARAM lParam)
{
	char szTempPath[_MAX_PATH+2];
	int nLen = GetTempPath(_MAX_PATH, szTempPath);
	if (nLen) {
		if (szTempPath[nLen-1] != '\\') {
			strcat(szTempPath, "\\");
		}
	}
	char szAsc[_MAX_PATH+10];
	char szTxt[_MAX_PATH+10];
	char szOut[_MAX_PATH+10];
	strcpy(szAsc, szTempPath); strcat(szAsc, "pt.asc");
	strcpy(szTxt, szTempPath); strcat(szTxt, "pt");
	strcpy(szOut, szTempPath); strcat(szOut, "pt.out");
	DeleteFile(szAsc);
	DeleteFile(szTxt);
	DeleteFile(szOut);

	BOOL bDecrypt = TRUE;
	BOOL bSign = FALSE;

	CMIMEItem item;
	LPSTR lpSource = bka.GetSource(NULL);
	if (lpSource) {
		LPSTR lpStr = lpSource;
		item.FromString(lpStr);
		bka.Free(lpSource);
	}
	CMIMEItem* pTop = item.FindMIMEItem("multipart", "signed");
	if (pTop) {
		bSign = TRUE;
	} else {
		pTop = item.FindMIMEItem("multipart", "encrypted");
	}

	char szHeader[32768];
	if (pTop) {
		pTop->GetHeader("Content-Type", szHeader, 32768);
		if (bSign) {
			/* I should not use this because of PGP's bug.
			if (g_nPGPVer >= 651) {
				LPSTR lpMail = pTop->ToString();
				FILE* fp;
				if (fp = fopen(szAsc, "wb")) {
					fwrite(lpMail, strlen(lpMail), 1, fp);
					fclose(fp);
				}	
				free (lpMail);
			} else {
			*/
				CMIMEItem* pSign, pContent;
				pSign = pTop->FindMIMEItem("application", "pgp-signature");
				if (pSign) {
					LPSTR lpHash = __stristr(szHeader, "micalg=");
					if (lpHash) {
						lpHash += 7;
						lpHash = strtok(lpHash, "\r\n\t\" ");
						if (strnicmp(lpHash, "pgp-", 4) == 0) {
							lpHash += 4;
						}
						strupr(lpHash);
					}
					CMIMEItem* pContent = pTop->GetChild();
					while (stricmp(pContent->m_szSubType, "pgp-signature") == 0 &&
						pContent) {
						pContent = pContent->GetNext();
					}
					LPSTR lpSign = pSign->ToString();
					LPSTR lpBody = NULL;
					FILE* fp,* fp2;
					if ((fp = fopen(szAsc, "wb")) &&
						(fp2 = fopen(szTxt, "wb"))) {
						if (strstr(lpSign, "-----BEGIN PGP SIGNATURE-----")) {
							if (pContent) {
								lpBody = pContent->ToString();
								int nLen = strlen(lpBody);
								// Remove trailing CRLF (it actually belongs to the boundary)
								if (nLen > 2) {
									if (lpBody[nLen-1] == '\n' &&
										lpBody[nLen-2] == '\r') {
										nLen -= 2;
										lpBody[nLen] = '\0';
									}
								}
								fwrite(lpBody, nLen, 1, fp2);
								free(lpBody);
								/* To work around PGP bug.
								fprintf(fp, "-----BEGIN PGP SIGNED MESSAGE-----\r\n");
								if (lpHash) {
									fprintf(fp, "Hash: %s\r\n\r\n", lpHash);
								} else {
									fprintf(fp, "\r\n");
								}
								int n;
								char szString[32768];
								LPSTR lpRead = lpBody;
								while (n = sGets((LPCTSTR&)lpRead, szString, 32768)) {
									if (strncmp(szString, "--", 2) == 0) {
										fprintf(fp, "- ");
									}
									if (szString[n-1] == '\n') {
										szString[n-1] = '\r';
										szString[n] = '\n';
										szString[n+1] = '\0';
									}
									fwrite(szString, strlen(szString), 1, fp);
								}
								*/
							}
						} else {
							bSign = FALSE;
						}
						LPSTR lpPGP = strstr(lpSign, "-----BEGIN PGP ");
						if (lpPGP) {
							fwrite(lpPGP, strlen(lpPGP), 1, fp);
						}
						fclose(fp);
						fclose(fp2);
					}
					free(lpSign);
				}
			//}
		} else {
			strcpy(szPassPhrase, "");
			int nRC = DialogBox(g_hInstance,
					MAKEINTRESOURCE(IDD_DLGINPUTPASS1), hWnd, (DLGPROC)PGPPassProc);
			if (nRC != IDOK) return;
			CMIMEItem* pContent = pTop->FindMIMEItem("application", "octet-stream");
			if (pContent) {
				LPSTR lpContent = pContent->ToString();
				if (lpContent) {
					FILE* fp;
					if (fp = fopen(szAsc, "wb")) {
						LPSTR lpPGP = strstr(lpContent, "-----BEGIN PGP ");
						if (lpPGP) {
							fwrite(lpPGP, strlen(lpPGP), 1, fp);
						}
						fclose(fp);
					}
					free(lpContent);
				}
			}
		}
		char szCommandLine[32768];
		szCommandLine[0] = '\0';

		char szCom[_MAX_PATH];
		GetEnvironmentVariable("COMSPEC" , szCom, _MAX_PATH);

		LPSTR lpCom = GetFnameTop(szCom, NULL);

		strcat(szCommandLine, lpCom);
		strcat(szCommandLine, " /C ");

		strcat(szCommandLine, "pgp.exe ");
		strcat(szCommandLine, GetFnameTop(szAsc, NULL));
		if (g_bPGPMIME && bSign) {
			strcat(szCommandLine, " ");
			strcat(szCommandLine, GetFnameTop(szTxt, NULL));
		}
		if (szPassPhrase[0] != '\0') {
			strcat(szCommandLine, " -z\"");
			strcat(szCommandLine, szPassPhrase);
			strcat(szCommandLine, "\"");
		}

		if (bDecrypt || bSign) {
			strcat(szCommandLine, " +batchmode +force");
		}

		DWORD dwStatus = 0;
		STARTUPINFO sStartInfo;
		PROCESS_INFORMATION sProcessInfo;
		DWORD dwFlags;

		memset (&sStartInfo, 0, sizeof(STARTUPINFO));
		memset (&sProcessInfo, 0, sizeof(PROCESS_INFORMATION));
		sStartInfo.cb = sizeof(sStartInfo);
		sStartInfo.lpReserved = NULL;
		sStartInfo.lpDesktop = NULL;
		sStartInfo.lpTitle = NULL;
		sStartInfo.dwFlags = STARTF_USESHOWWINDOW;
		sStartInfo.wShowWindow = (bDecrypt)? SW_SHOWMINNOACTIVE:SW_SHOW;
		sStartInfo.cbReserved2 = 0;
		sStartInfo.lpReserved2 = NULL;
		dwFlags = HIGH_PRIORITY_CLASS;

		char szCurrPath[_MAX_PATH+2];
		GetCurrentDirectory(_MAX_PATH, szCurrPath);
		SetCurrentDirectory(szTempPath);

		if (bDecrypt || bSign) {
			strcat(szCommandLine, " >");
			strcat(szCommandLine, szOut);
		}

		if (!CreateProcess(NULL, szCommandLine, NULL, NULL,
						FALSE, dwFlags, NULL, NULL, &sStartInfo, &sProcessInfo)) {
			char szError[258];
			LoadString(g_hInstance, IDS_PGP_NOTPROCESSED, szError, 255);
			MessageBox(hWnd, szError, "Message from BkPGP", MB_OK);
			return;
		}
		BOOL bBreak = FALSE;
		while (!bBreak) {
			GetExitCodeProcess(sProcessInfo.hProcess, &dwStatus);
			if (dwStatus != STILL_ACTIVE) bBreak = TRUE;
		}
		SetCurrentDirectory(szCurrPath);

		if (bDecrypt || bSign) {
			lpOutput = FileToString(szOut);
			int nRC = DialogBox(g_hInstance,
						MAKEINTRESOURCE(IDD_DLGVIEW), hWnd, (DLGPROC)PGPOutputProc);

			free(lpOutput);
		}
		if (!bSign && bDecrypt) {
			LPSTR lpArea = FileToString(szTxt);
			if (lpArea) {
				CMIMEItem itemDecrypt;
				LPSTR lpStr = lpArea;
				itemDecrypt.FromString(lpStr);
				strcpy(pTop->m_szType, itemDecrypt.m_szType);
				strcpy(pTop->m_szSubType, itemDecrypt.m_szSubType);
				char szHeader[32768];
				itemDecrypt.GetHeader("Content-Type", szHeader, 32768);
				if (szHeader[0]) {
					pTop->SetHeader("Content-Type", szHeader);
				}
				itemDecrypt.GetHeader("Content-Transfer-Encoding", szHeader, 32768);
				if (szHeader[0]) {
					pTop->SetHeader("Content-Transfer-Encoding", szHeader);
				}
				if (pTop->m_lpBoundary) {
					free(pTop->m_lpBoundary);
					pTop->m_lpBoundary = NULL;
				}
				if (itemDecrypt.m_lpBoundary) {
					pTop->m_lpBoundary = strdup(itemDecrypt.m_lpBoundary);
				}
				pTop->m_lstBody.Empty();
				pTop->SetChild(NULL);
				LPSTR lpBody = strstr(lpArea, "\r\n\r\n");
				if (lpBody) {
					lpBody += 4;
				} else {
					lpBody = "\r\n";
				}
				pTop->m_lstBody.AddTail(lpBody);
				free(lpArea);
				lpArea = item.ToString();
				//bka.SetText(-2, lpArea); // for debug
				bka.SetSource("TEMP", lpArea);
				free(lpArea);
			}
		}

		DeleteFile(szOut);
		DeleteFile(szAsc);
		DeleteFile(szTxt);
	} else {
		char szMimeType[80];
		char szError[258];
		LoadString(g_hInstance, IDS_PGP_ERROR, szError, 255);

		LPSTR lpData = bka.GetText(szMimeType, 80);
		LPSTR lpStr;
		if (strncmp(lpData, "-----BEGIN PGP ", 15) == 0) {
			lpStr = lpData;
		} else {
			if (!(lpStr = strstr(lpData, "\n-----BEGIN PGP "))) {
				MessageBox(hWnd, szError, "Message from BkPGP", MB_OK);
				bka.Free(lpData);
				return;
			}
			lpStr++;
		}
		strcpy(szPassPhrase, "");
		char szCharSet[80];
		bka.GetCharSet(NULL, szCharSet, 80);
		do {
			bDecrypt = TRUE;
			bSign = FALSE;
			if (strncmp(lpStr, "-----BEGIN PGP PUBLIC KEY BLOCK-----", 36) == 0) {
				bDecrypt = FALSE;
			} else if (strncmp(lpStr, "-----BEGIN PGP SIGNED MESSAGE-----",34) == 0) {
				bSign = TRUE;
			} else {
				if (strcmp(szPassPhrase, "") == 0) {
					int nRC = DialogBox(g_hInstance,
							MAKEINTRESOURCE(IDD_DLGINPUTPASS1), hWnd, (DLGPROC)PGPPassProc);
					if (nRC != IDOK) return;
				}
			}
			LPSTR lpEnd = strstr(lpStr, "\n-----END PGP ");
			if (!lpEnd) {
				MessageBox(hWnd, szError, "Message from BkPGP", MB_OK);
				bka.Free(lpData);
				return;
			} else {
				lpEnd++;
				LPSTR lpTmp = strchr(lpEnd,'\n');
				if (lpTmp) lpEnd = lpTmp;
				else lpEnd = strchr(lpEnd, '\0');
			}
			FILE* fp;
			if (fp = fopen(szAsc, "wb")) {
				if (stricmp(szCharSet, "ISO-2022-JP") == 0 ||
					(szCharSet[0] == '\0' && GetACP() == 932)) {
					LPSTR lpTemp = (LPSTR)malloc(lpEnd - lpStr + 10);
					strncpy(lpTemp, lpStr, lpEnd-lpStr);
					lpTemp[lpEnd-lpStr] = '\0';
					LPSTR lpJIS = bka.ISO_2022_JP(lpTemp, TRUE);
					free(lpTemp);
					fwrite(lpJIS, strlen(lpJIS), 1, fp);
					bka.Free(lpJIS);
				} else {
					fwrite(lpStr, lpEnd-lpStr, 1, fp);
				}
				fclose(fp);
			} else {
				bka.Free(lpData);
				return;
			}

			char szCommandLine[32768];
			szCommandLine[0] = '\0';

			char szCom[_MAX_PATH];
			GetEnvironmentVariable("COMSPEC" , szCom, _MAX_PATH);

			LPSTR lpCom = GetFnameTop(szCom, NULL);

			strcat(szCommandLine, lpCom);
			strcat(szCommandLine, " /C ");

			strcat(szCommandLine, "pgp.exe ");
			strcat(szCommandLine, GetFnameTop(szAsc, NULL));
			if (szPassPhrase[0] != '\0') {
				strcat(szCommandLine, " -z\"");
				strcat(szCommandLine, szPassPhrase);
				strcat(szCommandLine, "\"");
			}

			if (bDecrypt || bSign) {
				strcat(szCommandLine, " +batchmode +force");
			}

			DWORD dwStatus = 0;
			STARTUPINFO sStartInfo;
			PROCESS_INFORMATION sProcessInfo;
			DWORD dwFlags;

			memset (&sStartInfo, 0, sizeof(STARTUPINFO));
			memset (&sProcessInfo, 0, sizeof(PROCESS_INFORMATION));
			sStartInfo.cb = sizeof(sStartInfo);
			sStartInfo.lpReserved = NULL;
			sStartInfo.lpDesktop = NULL;
			sStartInfo.lpTitle = NULL;
			sStartInfo.dwFlags = STARTF_USESHOWWINDOW;
			sStartInfo.wShowWindow = (bDecrypt)? SW_SHOWMINNOACTIVE:SW_SHOW;
			sStartInfo.cbReserved2 = 0;
			sStartInfo.lpReserved2 = NULL;
			dwFlags = HIGH_PRIORITY_CLASS;

			char szCurrPath[_MAX_PATH+2];
			GetCurrentDirectory(_MAX_PATH, szCurrPath);
			SetCurrentDirectory(szTempPath);

			if (bDecrypt || bSign) {
				strcat(szCommandLine, " >");
				strcat(szCommandLine, szOut);
			}

			if (!CreateProcess(NULL, szCommandLine, NULL, NULL,
							FALSE, dwFlags, NULL, NULL, &sStartInfo, &sProcessInfo)) {
				char szError[258];
				LoadString(g_hInstance, IDS_PGP_NOTPROCESSED, szError, 255);
				MessageBox(hWnd, szError, "Message from BkPGP", MB_OK);
				bka.Free(lpData);
				return;
			}
			BOOL bBreak = FALSE;
			while (!bBreak) {
				GetExitCodeProcess(sProcessInfo.hProcess, &dwStatus);
				if (dwStatus != STILL_ACTIVE) bBreak = TRUE;
			}
			SetCurrentDirectory(szCurrPath);

			if (bDecrypt || bSign) {
				lpOutput = FileToString(szOut);
				int nRC = DialogBox(g_hInstance,
							MAKEINTRESOURCE(IDD_DLGVIEW), hWnd, (DLGPROC)PGPOutputProc);

				free(lpOutput);
			}
			if (!bSign && bDecrypt) {
				LPSTR lpArea = FileToString(szTxt);
				if (lpArea) {
					bka.SetText(-2, "***************** Original PGP message follows *****************\r\n\r\n");
					if (stricmp(szCharSet, "ISO-2022-JP") == 0) {
						LPSTR lpSJIS = bka.ISO_2022_JP(lpArea, FALSE);
						bka.SetText(-2, lpSJIS);
						bka.Free(lpSJIS);
					} else {
						bka.SetText(-2, lpArea);
					}
					free(lpArea);
				}
			}

			DeleteFile(szOut);
			DeleteFile(szAsc);
			DeleteFile(szTxt);
			if (!(lpStr = strstr(lpEnd, "\n-----BEGIN PGP "))) {
				break;
			}
			lpStr++;
			strcpy(szAsc, szTempPath); strcat(szAsc, "pt.asc");
			strcpy(szTxt, szTempPath); strcat(szTxt, "pt");
			strcpy(szOut, szTempPath); strcat(szOut, "pt.out");
			DeleteFile(szAsc);
			DeleteFile(szTxt);
			DeleteFile(szOut);
		} while (TRUE);
		bka.Free(lpData);
	}
}

////////////////////////////////////////////////////////////////////////////
BOOL PGPOutgoing(HWND hWnd, LPCTSTR lpszOpt)
{
	if (g_bPGPMIME) {
		SendMessage(hWnd, WM_SET_TRANSFER_SAFE, (WPARAM)TRUE, 0);
	}
	if (strchr(lpszOpt, 's')) {
		char szFrom[32768];
		bka.CompGetSpecifiedHeader(hWnd, "From", szFrom, 32768);
		char szName[256], szEMail[256];
		GetNameAndAddr(szName, 256, szEMail, 256, szFrom);
		sprintf(szUserName, "<%s>", szEMail);
		strcpy(szPassPhrase, "");
		int nRC = DialogBox(g_hInstance,
				MAKEINTRESOURCE(IDD_DLGINPUTPASS), hWnd, (DLGPROC)PGPPassProc);
		if (nRC != IDOK) return FALSE;
	}
	LPSTR lpszUsers = NULL;
	int nBufLength = 0;
	if (strchr(lpszOpt, 'e')) {
		char szTo[32768];
		char szCc[32768];
		char szBcc[32768];
		bka.CompGetSpecifiedHeader(hWnd, "To", szTo, 32768);
		bka.CompGetSpecifiedHeader(hWnd, "Cc", szCc, 32768);
		bka.CompGetSpecifiedHeader(hWnd, "Bcc", szBcc, 32768);

		char szName[256], szEMail[256];
		char szUserID[260];
		CPointerList lstAddr;
		LPSTR lpTok;
		LPSTR lpTo = bka.SerializeRcpts(szTo);
		lpTok = TokenAddr(lpTo);
		while (lpTok) {
			GetNameAndAddr(szName, 256, szEMail, 256, lpTok);
			sprintf(szUserID, "<%s>", szEMail);
			lstAddr.AddTail(szUserID);
			nBufLength += strlen(szUserID) + 2;
			lpTok = TokenAddr(NULL);
		}
		bka.Free(lpTo);

		LPSTR lpCc = bka.SerializeRcpts(szCc);
		lpTok = TokenAddr(lpCc);
		while (lpTok) {
			GetNameAndAddr(szName, 256, szEMail, 256, lpTok);
			sprintf(szUserID, "<%s>", szEMail);
			lstAddr.AddTail(szUserID);
			nBufLength += strlen(szUserID) + 2;
			lpTok = TokenAddr(NULL);
		}
		bka.Free(lpCc);

		LPSTR lpBcc = bka.SerializeRcpts(szBcc);
		lpTok = TokenAddr(lpBcc);
		while (lpTok) {
			GetNameAndAddr(szName, 256, szEMail, 256, lpTok);
			sprintf(szUserID, "<%s>", szEMail);
			lstAddr.AddTail(szUserID);
			nBufLength += strlen(szUserID) + 2;
			lpTok = TokenAddr(NULL);
		}
		bka.Free(lpBcc);

		if (!lstAddr.GetTop()) {
			char szError[258];
			LoadString(g_hInstance, IDS_PGP_NORECIPIENT, szError, 255);
			MessageBox(hWnd, szError, "Message from BkPGP", MB_OK);
			return FALSE;
		}
		CPointerItem* pItem = lstAddr.GetTop();
		lpszUsers = (LPSTR)malloc(nBufLength+2);
		lpszUsers[0] = '\0';
		while (pItem) {
			LPSTR lpStr = pItem->GetData();
			strcat(lpszUsers, lpStr);
			strcat(lpszUsers, "\r\n");
			pItem = pItem->GetNext();
		}
	}
	char szTempPath[_MAX_PATH+2];
	int nLen = GetTempPath(_MAX_PATH, szTempPath);
	if (nLen) {
		if (szTempPath[nLen-1] != '\\') {
			strcat(szTempPath, "\\");
		}
	}
	char szAsc[_MAX_PATH+10];
	char szTxt[_MAX_PATH+10];
	char szAdr[_MAX_PATH+10];
	char szOut[_MAX_PATH+10];
	strcpy(szAsc, szTempPath); strcat(szAsc, "pt.asc");
	strcpy(szTxt, szTempPath); strcat(szTxt, "pt");
	strcpy(szAdr, szTempPath); strcat(szAdr, "pt.adr");
	strcpy(szOut, szTempPath); strcat(szOut, "pt.out");
	DeleteFile(szAsc);
	DeleteFile(szTxt);
	DeleteFile(szAdr);
	DeleteFile(szOut);

	FILE* fp;
	if (!(fp = fopen(szTxt, "wb"))) return FALSE;

	char szMimeType[80];
	LPSTR lpBody = bka.CompGetText(hWnd, szMimeType, 80);
	char szCharSet[82];
	bka.CompGetCharSet(hWnd, szCharSet, 80);
	CMIMEItem item;
	BOOL bTrailSPC = FALSE;
	if (g_bPGPMIME) {
		LPSTR lpSrc = bka.CompGetSource(hWnd);
		if (lpSrc) {
			LPSTR lpStr = lpSrc;
			item.FromString(lpStr);
			char szData[1024];
			item.GetHeader("Content-Type", szData, 1024);
			fprintf(fp, "Content-Type: %s", szData);
			item.GetHeader("Content-Transfer-Encoding", szData, 1024);
			fprintf(fp, "Content-Transfer-Encoding: %s", szData);
			LPSTR lpContent = strstr(lpSrc, "\r\n\r\n");
			if (lpContent) {
				if (strstr(lpContent, " \r\n") ||
					strstr(lpContent, "\t\r\n") ) {
					bTrailSPC = TRUE;
				}
				lpContent += 2;
				fwrite(lpContent, strlen(lpContent), 1, fp);
			}
			bka.Free(lpSrc);
		} else return FALSE;
	} else {
		if (stricmp(szCharSet, "ISO-2022-JP") == 0 ||
			(szCharSet[0] == '\0' && GetACP() == 932)) {
			LPSTR lpJIS = bka.ISO_2022_JP(lpBody, TRUE);
			fwrite(lpJIS, strlen(lpJIS), 1, fp);
			bka.Free(lpJIS);
		} else {
			fwrite(lpBody, strlen(lpBody), 1, fp);
		}
	}
	fclose(fp);
	bka.Free(lpBody);

	BOOL bClear = FALSE;
	if (strchr(lpszOpt, 't')) {
		bClear = TRUE;
	}

	char szCommandLine[32768];
	szCommandLine[0] = '\0';

	char szCom[_MAX_PATH];
	GetEnvironmentVariable("COMSPEC" , szCom, _MAX_PATH);

	LPSTR lpCom = GetFnameTop(szCom, NULL);

	strcat(szCommandLine, lpCom);
	strcat(szCommandLine, " /C ");

	strcat(szCommandLine, "pgp.exe ");
	strcat(szCommandLine, lpszOpt);
	strcat(szCommandLine, " ");
	strcat(szCommandLine, GetFnameTop(szTxt, NULL));
	strcat(szCommandLine, " ");

	LPSTR lpszOut;
	lpszOut = szCommandLine;
	lpszOut += strlen(szCommandLine);
	if (lpszUsers) {
		if (g_nPGPVer != 263) {
			LPSTR lp = lpszUsers;
			LPSTR lpOut = lpszOut;
			while (*lp) {
				*lpOut++ = '\"';
				while (*(LPBYTE)lp >= (BYTE)' ') {
					*lpOut++ = *lp++;
				}
				*lpOut++ = '\"';
				*lpOut++ = ' ';
				while (*lp && *(LPBYTE)lp < (BYTE)' ') lp++;
			}
			*lpOut = '\0';
		} else {
			FILE* fp;
			fp = fopen(szAdr, "wb");
			if (fp) {
				fprintf(fp, lpszUsers);
				fclose(fp);
			} else return FALSE;
			strcpy(lpszOut, " -@");
			strcat(lpszOut, GetFnameTop(szAdr, NULL));
		}
	}
	if (strchr(lpszOpt, 's') && szUserName[0] != '\0') {
		strcat(szCommandLine, " -u\"");
		strcat(szCommandLine, szUserName);
		strcat(szCommandLine, "\"");
	}
	strcat(szCommandLine, " +batchmode +force");

	DWORD dwStatus;
	STARTUPINFO sStartInfo;
	PROCESS_INFORMATION sProcessInfo;
	DWORD dwFlags;
	memset (&sStartInfo, 0, sizeof(STARTUPINFO));
	memset (&sProcessInfo, 0, sizeof(PROCESS_INFORMATION));

	sStartInfo.cb = sizeof(sStartInfo);
	sStartInfo.lpReserved = NULL;
	sStartInfo.lpDesktop = NULL;
	sStartInfo.lpTitle = NULL;
	sStartInfo.dwFlags = STARTF_USESHOWWINDOW;
	sStartInfo.wShowWindow = SW_SHOWMINNOACTIVE;
	sStartInfo.cbReserved2 = 0;
	sStartInfo.lpReserved2 = NULL;
	dwFlags = HIGH_PRIORITY_CLASS;

	if (szPassPhrase[0]) {
		strcat(szCommandLine, " -z\"");
		strcat(szCommandLine, szPassPhrase);
		strcat(szCommandLine, "\"");
	}

	char szCurrPath[_MAX_PATH+2];
	GetCurrentDirectory(_MAX_PATH, szCurrPath);
	SetCurrentDirectory(szTempPath);

	strcat(szCommandLine, " >");
	strcat(szCommandLine, szOut);

	if (!CreateProcess(NULL, szCommandLine, NULL, NULL,
					FALSE, dwFlags, NULL, NULL, &sStartInfo, &sProcessInfo)) {
		char szError[258];
		LoadString(g_hInstance, IDS_PGP_NOTPROCESSED, szError, 255);
		MessageBox(hWnd, szError, "Message from BkPGP", MB_OK);
		return FALSE;
	}
	BOOL bBreak = FALSE;
	while (!bBreak) {
		GetExitCodeProcess(sProcessInfo.hProcess, &dwStatus);
		if (dwStatus != STILL_ACTIVE) bBreak = TRUE;
	}
	SetCurrentDirectory(szCurrPath);

	LPSTR lpArea = FileToString(szAsc);
	if (lpArea) {
		if (g_bPGPMIME) {
			LPSTR lpStr = NULL;
			if (strchr(lpszOpt, 's')) {
				lpStr = FileToString(szTxt);
			}
			char szBoundary[1024];
			char szData[8192];
			time_t t;
			time(&t);
			sprintf(szBoundary, "===[PGP/MIME_RFC2015]===%08X.%04X===", (DWORD)t, &t);
			if (strchr(lpszOpt, 's') && !strchr(lpszOpt, 'e')) {
				char szBuf[32768];
				int n;
				LPSTR lpRead = lpArea;
				char szHash[20];
				strcpy(szHash, "md5");
				while (n = sGets((LPCTSTR&)lpRead, szBuf, 32768)) {
					if (strcmp(szBuf, "\n") == 0) break;
					if (strnicmp(szBuf, "Hash:", 5) == 0) {
						LPSTR lpTemp = &szBuf[5];
						while (*lpTemp == ' ' || *lpTemp == '\t') lpTemp++;
						strncpy(szHash, lpTemp, 19); szHash[19] = '\0';
						strlwr(szHash);
						LPSTR lpHash = szHash;
						while (*lpHash) {
							if (*lpHash == '\r' || *lpHash == '\n') {
								*lpHash = '\0';
								break;
							}
							lpHash++;
						}
					}
				}
				if (bTrailSPC) { // You have to sign again when trailing whitespaces are found.
					szCommandLine[0] = '\0';

					strcat(szCommandLine, szCom);
					strcat(szCommandLine, " /C ");

					strcat(szCommandLine, "pgp.exe ");
					strcat(szCommandLine, "-sba");
					strcat(szCommandLine, " ");
					strcat(szCommandLine, GetFnameTop(szTxt, NULL));
					strcat(szCommandLine, " ");

					LPSTR lpszOut;
					lpszOut = szCommandLine;
					lpszOut += strlen(szCommandLine);
					strcat(szCommandLine, " -u\"");
					strcat(szCommandLine, szUserName);
					strcat(szCommandLine, "\"");
					strcat(szCommandLine, " +batchmode +force");

					DWORD dwStatus;
					STARTUPINFO sStartInfo;
					PROCESS_INFORMATION sProcessInfo;
					DWORD dwFlags;
					memset (&sStartInfo, 0, sizeof(STARTUPINFO));
					memset (&sProcessInfo, 0, sizeof(PROCESS_INFORMATION));

					sStartInfo.cb = sizeof(sStartInfo);
					sStartInfo.lpReserved = NULL;
					sStartInfo.lpDesktop = NULL;
					sStartInfo.lpTitle = NULL;
					sStartInfo.dwFlags = STARTF_USESHOWWINDOW;
					sStartInfo.wShowWindow = SW_SHOWMINNOACTIVE;
					sStartInfo.cbReserved2 = 0;
					sStartInfo.lpReserved2 = NULL;
					dwFlags = HIGH_PRIORITY_CLASS;
					strcat(szCommandLine, " -z\"");
					strcat(szCommandLine, szPassPhrase);
					strcat(szCommandLine, "\"");
					char szCurrPath[_MAX_PATH+2];
					GetCurrentDirectory(_MAX_PATH, szCurrPath);
					SetCurrentDirectory(szTempPath);
					strcat(szCommandLine, " >");
					strcat(szCommandLine, szOut);

					if (!CreateProcess(NULL, szCommandLine, NULL, NULL,
									FALSE, dwFlags, NULL, NULL, &sStartInfo, &sProcessInfo)) {
						char szError[258];
						LoadString(g_hInstance, IDS_PGP_NOTPROCESSED, szError, 255);
						MessageBox(hWnd, szError, "Message from BkPGP", MB_OK);
						return FALSE;
					}
					BOOL bBreak = FALSE;
					while (!bBreak) {
						GetExitCodeProcess(sProcessInfo.hProcess, &dwStatus);
						if (dwStatus != STILL_ACTIVE) bBreak = TRUE;
					}
					SetCurrentDirectory(szCurrPath);
					free(lpArea);
					lpArea = FileToString(szAsc);
				}
				sprintf(szData, "multipart/signed;\r\n boundary=\"%s\";\r\n protocol=\"application/pgp-signature\"; micalg=pgp-%s\r\n",
					szBoundary, szHash);
				item.SetHeader("Content-Type", szData);
				char szEncoding[40];
				item.GetHeader("Content-Transfer-Encoding", szEncoding, 40);
				if (stricmp(szEncoding, "8bit") == 0 ||
					stricmp(szEncoding, "binary") == 0) {
				} else {
					item.SetHeader("Content-Transfer-Encoding", "7bit\r\n");
				}
				item.m_lstBody.Empty();
				item.SetChild(NULL); // Remove all the child items or the body of "multipart" will not be used.
				sprintf(szBuf, "--%s\r\n", szBoundary);
				item.m_lstBody.AddTail(szBuf);
				item.m_lstBody.AddTail(lpStr);
				sprintf(szBuf, "\r\n--%s\r\n", szBoundary);
				item.m_lstBody.AddTail(szBuf);
				item.m_lstBody.AddTail("Content-Type: application/pgp-signature\r\n");
				item.m_lstBody.AddTail("Content-Transfer-Encoding: 7bit\r\n\r\n");
				LPSTR lpSig = strstr(lpArea, "-----BEGIN PGP SIGNATURE-----");
				if (!lpSig) {
					lpSig = strstr(lpArea, "-----BEGIN PGP MESSAGE-----");
				}
				if (lpSig) {
					item.m_lstBody.AddTail(lpSig);
				}
				sprintf(szBuf, "\r\n--%s--\r\n", szBoundary);
				item.m_lstBody.AddTail(szBuf);
				LPSTR lpSource = item.ToString();
				//bka.CompSetText(hWnd, 0, lpSource); // for debug
				bka.CompSetSource(hWnd, lpSource);
				free(lpSource);
			} else {
				char szBuf[32768];
				sprintf(szData, "multipart/encrypted;\r\n boundary=\"%s\";\r\n protocol=\"application/pgp-encrypted\"\r\n",
					szBoundary);
				item.SetHeader("Content-Type", szData);
				item.SetHeader("Content-Transfer-Encoding", "7bit\r\n");
				item.m_lstBody.Empty();
				item.SetChild(NULL); // Remove all the child items or the body of "multipart" will not be used.
				sprintf(szBuf, "--%s\r\n", szBoundary);
				item.m_lstBody.AddTail(szBuf);
				item.m_lstBody.AddTail("Content-Type: application/pgp-encrypted\r\n");
				item.m_lstBody.AddTail("Content-Transfer-Encoding: 7bit\r\n\r\n");
				item.m_lstBody.AddTail("Version: 1\r\n");
				sprintf(szBuf, "\r\n--%s\r\n", szBoundary);
				item.m_lstBody.AddTail(szBuf);
				item.m_lstBody.AddTail("Content-Type: application/octet-stream\r\n");
				item.m_lstBody.AddTail("Content-Transfer-Encoding: 7bit\r\n\r\n");
				LPSTR lpMsg = strstr(lpArea, "-----BEGIN PGP MESSAGE-----");
				if (lpMsg) {
					item.m_lstBody.AddTail(lpMsg);
				}
				sprintf(szBuf, "\r\n--%s--\r\n", szBoundary);
				item.m_lstBody.AddTail(szBuf);
				LPSTR lpSource = item.ToString();
				//bka.CompSetText(hWnd, 0, lpSource); // for debug
				bka.CompSetSource(hWnd, lpSource);
				free(lpSource);
			}
			free(lpStr);
		} else {
			bka.CompSetText(hWnd, 0, lpArea);
		}
		bka.Command(hWnd, "TextTop");
		free(lpArea);
	}

	lpArea = FileToString(szOut);
	if (lpArea) {
		if (strchr(lpArea, '\x07') || strchr(lpszOpt, 'e')) {
			lpOutput = lpArea;
			int nRC = DialogBox(g_hInstance,
						MAKEINTRESOURCE(IDD_DLGVIEW), hWnd, (DLGPROC)PGPOutputProc);
		}
	}

	DeleteFile(szAdr);
	DeleteFile(szAsc);
	DeleteFile(szTxt);
	DeleteFile(szOut);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////
void WINAPI CmdPGPEnc(HWND hWnd, LPARAM lParam)
{
	PGPOutgoing(hWnd, "-ea");
}

//////////////////////////////////////////////////////////////////////////////////////
void WINAPI CmdPGPSig(HWND hWnd, LPARAM lParam)
{
	PGPOutgoing(hWnd, "-sta");
}

//////////////////////////////////////////////////////////////////////////////////////
void WINAPI CmdPGPSigEnc(HWND hWnd, LPARAM lParam)
{
	PGPOutgoing(hWnd, "-esa");
}


//////////////////////////////////////////////////////////////////////////////////////
void WINAPI CmdPGPKey(HWND hWnd, LPARAM lParam)
{
	char szTempPath[_MAX_PATH+2];
	int nLen = GetTempPath(_MAX_PATH, szTempPath);
	if (nLen) {
		if (szTempPath[nLen-1] != '\\') {
			strcat(szTempPath, "\\");
		}
	}
	char szFrom[32768];
	bka.CompGetSpecifiedHeader(hWnd, "From", szFrom, 32768);
	char szName[256], szEMail[256];
	GetNameAndAddr(szName, 256, szEMail, 256, szFrom);
	char szFile[_MAX_PATH];
	GetLegalFileName(szEMail, szFile, _MAX_PATH);

	char szAsc[_MAX_PATH+10];
	char szTxt[_MAX_PATH+10];
	char szAdr[_MAX_PATH+10];
	char szOut[_MAX_PATH+10];
	strcpy(szAsc, szTempPath); strcat(szAsc, "pt.asc");
	strcpy(szOut, szTempPath); strcat(szOut, "pt.out");
	DeleteFile(szAsc);
	DeleteFile(szOut);

	sprintf(szUserName, "<%s>", szEMail);
	int nRC = DialogBox(g_hInstance,
				MAKEINTRESOURCE(IDD_DLGINPUTID), hWnd, (DLGPROC)PGPUserProc);
	if (nRC != IDOK) return;
	if (strcmp(szUserName, "") == 0) return;


	char szCommandLine[32768];
	szCommandLine[0] = '\0';

	char szCom[_MAX_PATH];
	GetEnvironmentVariable("COMSPEC" , szCom, _MAX_PATH);

	LPSTR lpCom = GetFnameTop(szCom, NULL);

	strcat(szCommandLine, lpCom);
	strcat(szCommandLine, " /C ");

	strcat(szCommandLine, "pgp.exe -kxa \"");
	strcat(szCommandLine, szUserName);
	strcat(szCommandLine, "\" ");

	strcat(szCommandLine, GetFnameTop(szAsc, NULL));
	strcat(szCommandLine, " +batchmode +force");

	DWORD dwStatus;
	STARTUPINFO sStartInfo;
	PROCESS_INFORMATION sProcessInfo;
	DWORD dwFlags;

	sStartInfo.cb = sizeof(sStartInfo);
	sStartInfo.lpReserved = NULL;
	sStartInfo.lpDesktop = NULL;
	sStartInfo.lpTitle = NULL;
	sStartInfo.dwFlags = STARTF_USESHOWWINDOW;
	sStartInfo.wShowWindow = SW_SHOWMINNOACTIVE;
	sStartInfo.cbReserved2 = 0;
	sStartInfo.lpReserved2 = NULL;
	dwFlags = HIGH_PRIORITY_CLASS;

	char szCurrPath[_MAX_PATH+2];
	GetCurrentDirectory(_MAX_PATH, szCurrPath);
	SetCurrentDirectory(szTempPath);

	strcat(szCommandLine, " >");
	strcat(szCommandLine, GetFnameTop(szOut, NULL));

	if (!::CreateProcess(NULL, (LPTSTR)(LPCTSTR)szCommandLine, NULL, NULL,
					FALSE, dwFlags, NULL, NULL, &sStartInfo, &sProcessInfo)) {
		char szError[258];
		LoadString(g_hInstance, IDS_PGP_NOTPROCESSED, szError, 255);
		MessageBox(hWnd, szError, "Message from BkPGP", MB_OK);
		return;
	}
	BOOL bBreak = FALSE;
	while (!bBreak) {
		::GetExitCodeProcess(sProcessInfo.hProcess, &dwStatus);
		if (dwStatus != STILL_ACTIVE) bBreak = TRUE;
	}
	SetCurrentDirectory(szCurrPath);

	char szFileName[_MAX_PATH+2];
	strcpy(szFileName, szTempPath); strcat(szFileName, szFile); strcat(szFileName, ".asc");
	if (::MoveFile(szAsc, szFileName)) {
		strcpy(szAsc, szFileName);
	}

	if (g_bPGPMIME) {
		bka.CompAttachFile(hWnd, szAsc, "application/pgp-keys");
	} else {
		LPSTR lpArea = NULL;

		int nPasteFlg = 1;
		if (!(lpArea = FileToString(szAsc))) {
			if (!(lpArea = FileToString(szAsc))) {
				return;
			}
			nPasteFlg = -2;
		}
		bka.CompSetText(hWnd, nPasteFlg, lpArea);
		bka.Command(hWnd, "TextTop");
	}

	DeleteFile(szAdr);
	DeleteFile(szAsc);
	DeleteFile(szTxt);
	DeleteFile(szOut);
}

//////////////////////////////////////////////////////////////////////////////////////
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
				g_nPGPVer = GetPrivateProfileInt("Settings", "PGPVersion", 651, szIni);
				g_bPGPMIME = GetPrivateProfileInt("Settings", "PGPMIME", TRUE, szIni);
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

int WINAPI BKC_OnStart()
{
	return 0;
}

int WINAPI BKC_OnExit()
{
	return 0;
}

int WINAPI BKC_OnMenuInit(HWND hWnd, HMENU hMenu, int nType)
{
	switch (nType) {
	case BKC_MENU_MAIN:
		{
            HMENU hSubMenu = GetSubMenu(hMenu, 4);
            AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);

			char szMsg[82];
			UINT nID;
			LoadString(g_hInstance, IDS_PGP_DECRYPT, szMsg, 80);
			nID = bka.RegisterCommand(szMsg, nType, CmdPGP);
			LoadString(g_hInstance, IDS_PGPMENU_DECRYPT, szMsg, 80);
            AppendMenu(hSubMenu, MF_STRING, nID, szMsg);

			LoadString(g_hInstance, IDS_PGP_SETUP, szMsg, 80);
			nID = bka.RegisterCommand(szMsg, nType, SetupPGP);
			LoadString(g_hInstance, IDS_PGPMENU_SETUP, szMsg, 80);
            AppendMenu(hSubMenu, MF_STRING, nID, szMsg);
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
	case BKC_MENU_COMPOSE:
		{
            HMENU hSubMenu = GetSubMenu(hMenu, 3);
            AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
			char szMsg[82];
			UINT nID;
			LoadString(g_hInstance, IDS_PGP_ENCRYPT, szMsg, 80);
			nID = bka.RegisterCommand(szMsg, nType, CmdPGPEnc);
			LoadString(g_hInstance, IDS_PGPMENU_ENCRYPT, szMsg, 80);
            AppendMenu(hSubMenu, MF_STRING, nID, szMsg);

			LoadString(g_hInstance, IDS_PGP_SIGN, szMsg, 80);
			nID = bka.RegisterCommand(szMsg, nType, CmdPGPSig);
			LoadString(g_hInstance, IDS_PGPMENU_SIGN, szMsg, 80);
            AppendMenu(hSubMenu, MF_STRING, nID, szMsg);

			LoadString(g_hInstance, IDS_PGP_SIGNENCRYPT, szMsg, 80);
			nID = bka.RegisterCommand(szMsg, nType, CmdPGPSigEnc);
			LoadString(g_hInstance, IDS_PGPMENU_SIGNENCRYPT, szMsg, 80);
            AppendMenu(hSubMenu, MF_STRING, nID, szMsg);

			LoadString(g_hInstance, IDS_PGP_ATTACHKEY, szMsg, 80);
			nID = bka.RegisterCommand(szMsg, nType, CmdPGPKey);
			LoadString(g_hInstance, IDS_PGPMENU_ATTACHKEY, szMsg, 80);
            AppendMenu(hSubMenu, MF_STRING, nID, szMsg);
		}
		break;
	case BKC_MENU_COMPEDIT:
		break;
	case BKC_MENU_COMPREF:
		break;
	default:
		break;
	}
	return 0;
}

int WINAPI BKC_OnOpenFolder(LPCTSTR)
{
	return 0;
}

int WINAPI BKC_OnOpenMail(LPCTSTR)
{
	return 0;
}

int WINAPI BKC_OnEveryMinute()
{
	return 0;
}

int WINAPI BKC_OnOpenCompose(HWND, int)
{
	return 0;
}

int WINAPI BKC_OnOutgoing(HWND, int)
{
	return 0;
}

int WINAPI BKC_OnKeyDispatch(HWND, int, int)
{
	return 0;
}

int WINAPI BKC_OnRetrieve(LPCTSTR, LPCTSTR)
{
	return 0;
}

int WINAPI BKC_OnSend(LPCTSTR)
{
	return 0;
}

int WINAPI BKC_OnFinishRetrieve(int)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Called when plug-in setup is needed.
int WINAPI BKC_OnPlugInSetup(HWND hWnd)
{
	SetupPGP(hWnd, 0);
    // Return nonzero if processed.
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
	char sz[256];
	LoadString(g_hInstance, IDS_PIN_NAME, sz, 255);
    strcpy(lpPlugInInfo->szPlugInName, sz);
    strcpy(lpPlugInInfo->szVendor, g_szVendor);
    strcpy(lpPlugInInfo->szVersion, g_szVer);
	LoadString(g_hInstance, IDS_PIN_DESCRIPTION, sz, 255);
    strcpy(lpPlugInInfo->szDescription, sz);
    // Always return 0.
    return 0;
}

#ifdef __cplusplus
}
#endif
