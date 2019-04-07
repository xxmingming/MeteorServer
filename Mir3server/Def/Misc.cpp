#include "../GameSvr/StdAfx.h"
#include <stdio.h>

void CenterDialog(HWND hParentWnd, HWND hWnd)
{
	RECT rcMainWnd, rcDlg;
	
	GetWindowRect(hParentWnd, &rcMainWnd);
	GetWindowRect(hWnd, &rcDlg);
	
	MoveWindow(hWnd, rcMainWnd.left + (((rcMainWnd.right - rcMainWnd.left) - (rcDlg.right - rcDlg.left)) / 2), 
				rcMainWnd.top + (((rcMainWnd.bottom - rcMainWnd.top) - (rcDlg.bottom - rcDlg.top)) / 2), 
				(rcDlg.right - rcDlg.left), (rcDlg.bottom - rcDlg.top), FALSE);
}

__int64 FileTimeToQuadWord(PFILETIME pFileTime)
{  
	__int64 qw;

	qw = pFileTime->dwHighDateTime;
	qw <<= 32;
	qw |= pFileTime->dwLowDateTime;

	return qw;
}

void QuadTimeToFileTime(__int64 qw, PFILETIME pFileTime)
{
	pFileTime->dwHighDateTime	= (DWORD)(qw >> 32);
	pFileTime->dwLowDateTime	= (DWORD)(qw & 0xFFFFFFFF);
}

int memlen(const char *str)
{
    const char *eos = str;

    while(*eos++);

    return((int)(eos - str));
}


void ChangeSpaceToNull(char *pszData)
{
	char *pszCheck = pszData;

	while (*pszCheck)
	{
		if (*pszCheck == 0x20)
		{
			*pszCheck = '\0';
			break;
		}

		pszCheck++;
	}
}

void GetDate(char *pszBuf)
{
	time_t	t;
	struct tm ttm;
	
	time(&t);
	memcpy(&ttm, localtime(&t), sizeof(struct tm));
	
	sprintf(pszBuf, "%02d%02d%02d", ttm.tm_year - 100, ttm.tm_mon + 1, ttm.tm_mday);
}

int GetTime()
{
	time_t	t;
	struct tm ttm;
	
	time(&t);
	memcpy(&ttm, localtime(&t), sizeof(struct tm));
	
//	wsprintf(pszBuf, _TEXT("%02d%02d%02d"), ttm.tm_year - 100, ttm.tm_mon + 1, ttm.tm_mday);

	return ttm.tm_hour;
}

string GBK2UTF8(const string& strGBK)
{
	wchar_t* str1;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, nullptr, 0);
	str1 = new wchar_t[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, nullptr, 0, nullptr, nullptr);
	char * str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, nullptr, nullptr);
	string strUTF8(str2);
	delete[]str1;
	delete[]str2;

	return strUTF8;
}


string UTF82GBK(const string& strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, nullptr, 0);
	unsigned short * str1 = new unsigned short[len + 1];
	memset(str1, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, (const char*)strUTF8.c_str(), -1, (LPWSTR)str1, len);
	len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)str1, -1, nullptr, 0, nullptr, nullptr);
	char *str2 = new char[len + 1];
	memset(str2, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)str1, -1, str2, len, nullptr, nullptr);
	string strGBK(str2);
	delete[]str1;
	delete[]str2;

	return strGBK;
}