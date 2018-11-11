#pragma once
/*
** Copyright (C) QPSOFT.COM All rights reserved.
*/

#ifndef INCLUDE_MINIDUMPER_H_
#define INCLUDE_MINIDUMPER_H_

#ifdef _MSC_VER

#include <string>

#include <windows.h>
#include <tchar.h>
#pragma comment(lib, "dbghelp.lib")

#include <dbghelp.h>
#pragma comment(lib, "user32.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "detours.h"
#include <iostream>
#include <vector>
using namespace std;
namespace qp
{

	typedef std::string StringA;
	typedef std::wstring StringW;

#ifdef _UNICODE
	typedef StringW String;
#else
	typedef StringA String;
#endif // _UNICODE

	//typedef BOOL(WINAPI* PFNWRITEDUMP)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION,
	//	PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);

	typedef LONG (*ExceptFilterFun)(struct _EXCEPTION_POINTERS* ep);
	const int MAX_NAME_LENGTH = 1024;
	struct CallStackInfo
	{
		CHAR ModuleName[MAX_NAME_LENGTH];
		CHAR MethodName[MAX_NAME_LENGTH];
		CHAR FileName[MAX_NAME_LENGTH];
		CHAR LineNumber[MAX_NAME_LENGTH];
	};
	class MiniDumper
	{
		static void __cdecl _purecall(void)
		{
			DebugBreak();
		}
	protected:
		MiniDumper() 
		{ 
			::SetUnhandledExceptionFilter(&MiniDumper::TopLevelExceptionFilter); 
			::_set_purecall_handler(_purecall);
			SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
			do
			{
				m_lpUnhandledExceptionFilter = (ExceptFilterFun)DetourFindFunction("KERNEL32.DLL", "UnhandledExceptionFilter");
				
				if (NULL == m_lpUnhandledExceptionFilter) {
					break;
				}
				LONG lRes = NO_ERROR;
				lRes = DetourTransactionBegin();
				if (NO_ERROR != lRes) {
					break;
				}

				lRes = DetourAttach((PVOID*)&m_lpUnhandledExceptionFilter, NewExceptionFilter);
				if (NO_ERROR != lRes) {
					break;
				}

				lRes = DetourTransactionCommit();
				if (NO_ERROR != lRes) {
					break;
				}
			} while (0);
		}
		~MiniDumper() 
		{
			if (m_lpUnhandledExceptionFilter) {
				do {
					LONG lRes = NO_ERROR;
					lRes = DetourTransactionBegin();
					if (NO_ERROR != lRes) {
						break;
					}

					lRes = DetourDetach((PVOID*)&m_lpUnhandledExceptionFilter, NewExceptionFilter);
					if (NO_ERROR != lRes) {
						break;
					}

					lRes = DetourTransactionCommit();
					if (NO_ERROR != lRes) {
						break;
					}
				} while (0);
			}
		};

	private:
		MiniDumper(const MiniDumper&);
		const MiniDumper& operator=(const MiniDumper&);

	public:
		static MiniDumper *dumper;
		static MiniDumper* Get()
		{
			if (dumper == NULL)
			{
				dumper = new MiniDumper();
				dumper->Init();
			}
			return dumper;
		}

		void Init(const String& dumpFilePath = String(), const String& appVersion = _T("1.0.0.0"))
		{
			if (!dumpFilePath.empty() && ::PathIsDirectory(dumpFilePath.c_str()))
				m_DumpFilePath = dumpFilePath;
			else
			{
				TCHAR appPath[MAX_PATH];
				::GetModuleFileName(NULL, appPath, MAX_PATH);
				::PathRemoveFileSpec(appPath);
				m_DumpFilePath = appPath;
			}

			m_AppVersion = appVersion;
		}

		String GetDumpFile() const
		{
			SYSTEMTIME st;
			::GetLocalTime(&st);
			TCHAR dt[11];
			wsprintf(dt, _T("%02d%02d%02d%02d%02d"), st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

			TCHAR appPath[MAX_PATH];
			::GetModuleFileName(NULL, appPath, MAX_PATH);
			::PathRemoveExtension(appPath);

			String file(m_DumpFilePath);
			file += _T("\\");
			file += ::PathFindFileName(appPath);
			file += _T("_") + m_AppVersion + _T("_");
			file += dt;
			file += _T(".dmp");
			return file;
		}

		String GetStackFile() const
		{
			SYSTEMTIME st;
			::GetLocalTime(&st);
			TCHAR dt[11];
			wsprintf(dt, _T("%02d%02d%02d%02d%02d"), st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

			TCHAR appPath[MAX_PATH];
			::GetModuleFileName(NULL, appPath, MAX_PATH);
			::PathRemoveExtension(appPath);

			String file(m_DumpFilePath);
			file += _T("\\");
			file += ::PathFindFileName(appPath);
			file += _T("_") + m_AppVersion + _T("_");
			file += dt;
			file += _T("_crashstack.log");
			return file;
		}

	private:
		static LONG WINAPI NewExceptionFilter(struct _EXCEPTION_POINTERS* ep)
		{
			//TopLevelExceptionFilter(ep);
			//if (m_lpUnhandledExceptionFilter != NULL)
			//	m_lpUnhandledExceptionFilter(ep);
			return EXCEPTION_CONTINUE_SEARCH;
		}

		static LONG WINAPI TopLevelExceptionFilter(struct _EXCEPTION_POINTERS* ep)
		{
			LONG ret = EXCEPTION_CONTINUE_SEARCH;
			String dumpFile = MiniDumper::Get()->GetDumpFile();
			HANDLE file = ::CreateFile(dumpFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			if (file == INVALID_HANDLE_VALUE)
				return ret;

			_MINIDUMP_EXCEPTION_INFORMATION mdei;
			mdei.ThreadId = ::GetCurrentThreadId();
			mdei.ExceptionPointers = ep;
			mdei.ClientPointers = NULL;

			MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory |
				MiniDumpWithDataSegs |
				MiniDumpWithHandleData |
				/*MiniDumpWithFullMemoryInfo |*/
				MiniDumpWithThreadInfo /*|
										MiniDumpWithUnloadedModules*/);

			if (MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), file, mdt, &mdei, NULL, NULL))
			{
				//::MessageBox(NULL, dumpFile.c_str(), _T("Crash Report!"), MB_ICONINFORMATION);
				const String openDmp(_T("/select,") + dumpFile);
				::ShellExecute(NULL, _T("open"), _T("explorer.exe"), openDmp.c_str(), NULL, SW_SHOWNORMAL);
				ret = EXCEPTION_EXECUTE_HANDLER;
			}

			CloseHandle(file);
			string stackfile = MiniDumper::Get()->GetStackFile();
			file = ::CreateFile(stackfile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			if (file == INVALID_HANDLE_VALUE)
				return ret;
			PCONTEXT pCtx = ep->ContextRecord;
			vector<CallStackInfo> arrCallStackInfo = GetCallStack(ep->ContextRecord);
			for (vector<CallStackInfo>::iterator i = arrCallStackInfo.begin(); i != arrCallStackInfo.end(); ++i)
			{
				CallStackInfo callstackinfo = (*i);
				//cout << callstackinfo.MethodName << "() : [" << callstackinfo.ModuleName << "] (File: " << callstackinfo.FileName << " @Line " << callstackinfo.LineNumber << ")" << endl;
				_tprintf_my(file, _T("%s::%s() File:%s Line:%s\r\n"), callstackinfo.ModuleName, callstackinfo.MethodName, callstackinfo.FileName, callstackinfo.LineNumber);
			}
			CloseHandle(file);
			return ret;
		}

		
		static int __cdecl _tprintf_my(HANDLE file, const TCHAR * format, ...)
		{
			TCHAR szBuff[1024];
			int retValue;
			DWORD cbWritten;
			va_list argptr;

			va_start(argptr, format);
			retValue = wvsprintf(szBuff, format, argptr);
			va_end(argptr);

			WriteFile(file, szBuff, retValue * sizeof(TCHAR), &cbWritten, 0);

			return retValue;
		}

		static BOOL GetLogicalAddress(
			PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset, char * file, DWORD& line)
		{
			MEMORY_BASIC_INFORMATION mbi;

			if (!VirtualQuery(addr, &mbi, sizeof(mbi)))
				return FALSE;

			DWORD hMod = (DWORD)mbi.AllocationBase;

			if (!GetModuleFileName((HMODULE)hMod, szModule, len))
				return FALSE;

			// Point to the DOS header inmemory
			PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

			// From the DOS header, find the NT (PE)header
			PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

			PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHdr);

			DWORD rva = (DWORD)addr - hMod; // RVA is offset from module loadaddress

											// Iterate through the section table, looking for the one thatencompasses
											// the linear address.
			for (unsigned i = 0;
				i < pNtHdr->FileHeader.NumberOfSections;
				i++, pSection++)
			{
				DWORD sectionStart = pSection->VirtualAddress;
				DWORD sectionEnd = sectionStart
					+ max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

				// Is the address in thissection???
				if ((rva >= sectionStart) && (rva <= sectionEnd))
				{
					// Yes, address is in the section. Calculate section and offset,
					// and store in the "section" & "offset" params, whichwere
					// passed by reference.
					section = i + 1;
					offset = rva - sectionStart;
					return TRUE;
				}
			}

			return FALSE;  // Should never get here!
		}

		static vector<CallStackInfo> GetCallStack(const CONTEXT *pContext)
		{
			HANDLE hProcess = GetCurrentProcess();
			SymInitialize(hProcess, NULL, TRUE);
			vector<CallStackInfo> arrCallStackInfo;
			CONTEXT c = *pContext;
			STACKFRAME64 sf;
			memset(&sf, 0, sizeof(STACKFRAME64));
			DWORD dwImageType = IMAGE_FILE_MACHINE_I386;
			// 不同的CPU类型，具体信息可查询MSDN  
			//  
#ifdef _M_IX86  
			sf.AddrPC.Offset = c.Eip;
			sf.AddrPC.Mode = AddrModeFlat;
			sf.AddrStack.Offset = c.Esp;
			sf.AddrStack.Mode = AddrModeFlat;
			sf.AddrFrame.Offset = c.Ebp;
			sf.AddrFrame.Mode = AddrModeFlat;
#elif _M_X64  
			dwImageType = IMAGE_FILE_MACHINE_AMD64;
			sf.AddrPC.Offset = c.Rip;
			sf.AddrPC.Mode = AddrModeFlat;
			sf.AddrFrame.Offset = c.Rsp;
			sf.AddrFrame.Mode = AddrModeFlat;
			sf.AddrStack.Offset = c.Rsp;
			sf.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64  
			dwImageType = IMAGE_FILE_MACHINE_IA64;
			sf.AddrPC.Offset = c.StIIP;
			sf.AddrPC.Mode = AddrModeFlat;
			sf.AddrFrame.Offset = c.IntSp;
			sf.AddrFrame.Mode = AddrModeFlat;
			sf.AddrBStore.Offset = c.RsBSP;
			sf.AddrBStore.Mode = AddrModeFlat;
			sf.AddrStack.Offset = c.IntSp;
			sf.AddrStack.Mode = AddrModeFlat;
#else  
#error "Platform not supported!"  
#endif  

			HANDLE hThread = GetCurrentThread();
			while (true)
			{
				// 该函数是实现这个功能的最重要的一个函数  
				// 函数的用法以及参数和返回值的具体解释可以查询MSDN  
				if (!StackWalk64(dwImageType, hProcess, hThread, &sf, &c, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
				{
					break;
				}
				if (sf.AddrFrame.Offset == 0)
				{
					break;
				}
				CallStackInfo callstackinfo;
				strcpy_s(callstackinfo.MethodName, MAX_NAME_LENGTH, "Miss");
				strcpy_s(callstackinfo.FileName, MAX_NAME_LENGTH, "Miss");
				strcpy_s(callstackinfo.ModuleName, MAX_NAME_LENGTH, "Miss");
				strcpy_s(callstackinfo.LineNumber, MAX_NAME_LENGTH, "Miss");

				BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL64) + MAX_NAME_LENGTH];
				IMAGEHLP_SYMBOL64 *pSymbol = (IMAGEHLP_SYMBOL64*)symbolBuffer;
				memset(pSymbol, 0, sizeof(IMAGEHLP_SYMBOL64) + MAX_NAME_LENGTH);

				pSymbol->SizeOfStruct = sizeof(symbolBuffer);
				pSymbol->MaxNameLength = MAX_NAME_LENGTH;

				DWORD symDisplacement = 0;
				// 得到函数名  
				//  
				if (SymGetSymFromAddr64(hProcess, sf.AddrPC.Offset, NULL, pSymbol))
					strcpy_s(callstackinfo.MethodName, MAX_NAME_LENGTH, pSymbol->Name);

				IMAGEHLP_LINE64 lineInfo;
				memset(&lineInfo, 0, sizeof(IMAGEHLP_LINE64));
				lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
				DWORD dwLineDisplacement;

				// 得到文件名和所在的代码行  
				//  
				if (SymGetLineFromAddr64(hProcess, sf.AddrPC.Offset, &dwLineDisplacement, &lineInfo))
				{
					strcpy_s(callstackinfo.FileName, MAX_NAME_LENGTH, lineInfo.FileName);
					sprintf_s(callstackinfo.LineNumber, "%d", lineInfo.LineNumber);
				}

				IMAGEHLP_MODULE64 moduleInfo;
				memset(&moduleInfo, 0, sizeof(IMAGEHLP_MODULE64));
				moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
				// 得到模块名  
				//  
				if (SymGetModuleInfo64(hProcess, sf.AddrPC.Offset, &moduleInfo))
				{
					strcpy_s(callstackinfo.ModuleName, MAX_NAME_LENGTH, moduleInfo.ModuleName);
				}
				arrCallStackInfo.push_back(callstackinfo);
			}

			SymCleanup(hProcess);
			return arrCallStackInfo;
		}
	private:
		String m_DumpFilePath;
		String m_AppVersion;
		static ExceptFilterFun m_lpUnhandledExceptionFilter;
	};
	MiniDumper * MiniDumper::dumper = NULL;
	ExceptFilterFun MiniDumper::m_lpUnhandledExceptionFilter = NULL;
} // namespace qp

#endif // _MSC_VER

#endif // INCLUDE_MINIDUMPER_H_
