#include "BkAddressGroupExtractOption.h"
#include "resource.h"

int g_nEnableExtractAddressGroup = 0;
enum ADDRESSFORMAT g_extractAddressFormat = AF_ADDRESS;

/**
 * PlugIn のオプションダイアログプロシージャ
 */
BOOL CALLBACK OptionDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	struct {
		int id;
		enum ADDRESSFORMAT format;
	} address_format[] = {
		{ IDC_FORMAT_ADDRESS,          AF_ADDRESS },
		{ IDC_FORMAT_NAME_AND_ADDRESS, AF_NAME_AND_ADDRESS },
		{ IDC_FORMAT_ADDRESS_AND_NAME, AF_ADDRESS_AND_NAME }
	};

	switch (uMsg) {
	case WM_INITDIALOG:
		/* 設定を初期化する */
		CheckDlgButton(hWndDlg, IDC_EN_EXTRACT_GROUP_ADDRESS,
			(g_nEnableExtractAddressGroup ? BST_CHECKED : BST_UNCHECKED));
		CheckRadioButton(hWndDlg, IDC_FORMAT_ADDRESS, IDC_FORMAT_ADDRESS_AND_NAME,
			IDC_FORMAT_ADDRESS + g_extractAddressFormat);

		/* コントロールの有効/無効を反映する */
		EnableWindow(GetDlgItem(hWndDlg, IDC_FORMAT), g_nEnableExtractAddressGroup);
		for (int i = 0; i < sizeof(address_format) / sizeof(address_format[0]); i++) {
			EnableWindow(GetDlgItem(hWndDlg, address_format[i].id), g_nEnableExtractAddressGroup);
		}
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_EN_EXTRACT_GROUP_ADDRESS:
			EnableWindow(GetDlgItem(hWndDlg, IDC_FORMAT), IsDlgButtonChecked(hWndDlg, IDC_EN_EXTRACT_GROUP_ADDRESS));
			for (int i = 0; i < sizeof(address_format) / sizeof(address_format[0]); i++) {
				EnableWindow(GetDlgItem(hWndDlg, address_format[i].id), 
					IsDlgButtonChecked(hWndDlg, IDC_EN_EXTRACT_GROUP_ADDRESS));
			}
			return TRUE;

		case IDC_FORMAT_ADDRESS:
		case IDC_FORMAT_NAME_AND_ADDRESS:
		case IDC_FORMAT_ADDRESS_AND_NAME:
			g_nEnableExtractAddressGroup = LOWORD(wParam);
			break;

		case IDOK:
			for (int i = 0; i < sizeof(address_format) / sizeof(address_format[0]); i++) {
				if (IsDlgButtonChecked(hWndDlg, address_format[i].id)) {
					g_extractAddressFormat = address_format[i].format;
					break;
				}
			}
			g_nEnableExtractAddressGroup = IsDlgButtonChecked(hWndDlg, IDC_EN_EXTRACT_GROUP_ADDRESS);
			EndDialog(hWndDlg, IDOK);
			return TRUE;

		case IDCANCEL:
			EndDialog(hWndDlg, IDCANCEL);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

