#include "stdafx.h"
extern HWND		g_hMainWnd;

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

			SendMessage(GetDlgItem(hWndDlg, IDC_LOGINSVR_PORT), EM_LIMITTEXT, (WPARAM)5, (LPARAM)0L);
			SendMessage(GetDlgItem(hWndDlg, IDC_LOCALPORT), EM_LIMITTEXT, (WPARAM)5, (LPARAM)0L);

			SetWindowTextA(GetDlgItem(hWndDlg, IDC_EDIT_GAMESRVIP), g_strGameSvrIP.c_str());
			SetDlgItemInt(hWndDlg, IDC_LOCALPORT, g_localPort, FALSE);
			SetDlgItemInt(hWndDlg, IDC_GAMESVR_PORT, g_GameSvrPort, FALSE);
			break;
		}
		case WM_COMMAND:
		{
			switch (wParam)
			{
				case IDOK:
				{
					char	szText[30];
					BOOL trans = FALSE;
					GetWindowTextA(GetDlgItem(hWndDlg, IDC_EDIT_GAMESRVIP), szText, 30);
					g_strGameSvrIP = szText;
					g_localPort = GetDlgItemInt(hWndDlg, IDC_LOCALPORT, &trans, FALSE);
					g_GameSvrPort = GetDlgItemInt(hWndDlg, IDC_GAMESVR_PORT, &trans, FALSE);
					SaveConfig();
				}
				case IDCANCEL:
					return EndDialog(hWndDlg, IDCANCEL);
			}
		}
	}

	return FALSE;
}
