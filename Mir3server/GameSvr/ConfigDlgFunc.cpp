#include "stdafx.h"
BOOL CALLBACK ConfigDlgFunc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			RECT rcMainWnd, rcDlg;

			GetWindowRect(g_hMainWnd, &rcMainWnd);
			GetWindowRect(hWndDlg, &rcDlg);

			MoveWindow(hWndDlg, rcMainWnd.left + (((rcMainWnd.right - rcMainWnd.left) - (rcDlg.right - rcDlg.left)) / 2), 
						rcMainWnd.top + (((rcMainWnd.bottom - rcMainWnd.top) - (rcDlg.bottom - rcDlg.top)) / 2), 
						(rcDlg.right - rcDlg.left), (rcDlg.bottom - rcDlg.top), FALSE);

			SendMessage(GetDlgItem(hWndDlg, IDC_DBSRV_PORT), EM_LIMITTEXT, (WPARAM)5, (LPARAM)0L);
			SetDlgItemTextA(hWndDlg, IDC_MAPFILE_LOC, g_strClientPath);
			break;
		}
		case WM_COMMAND:
		{
			switch (wParam)
			{
				case IDOK:
				{
					BOOL bTrans = FALSE;
					SaveConfig();
				}
				case IDCANCEL:
					return EndDialog(hWndDlg, IDCANCEL);
				case IDC_BUTTON1:
					if (TestDsn())
						MessageBox(NULL, _T("connection successful"), _T("OK"), MB_ICONINFORMATION);
					else
						MessageBox(NULL, TEXT("connection failed"), TEXT("failed"), MB_OK);
					return 0;
			}

			break;
		}
	}

	return FALSE;
}
