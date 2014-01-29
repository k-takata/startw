/*
 * startw  Ver.1.11
 */
/*
 * Copyright (C) 2005-2014, K.Takata
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>

#ifndef lengthof
#define lengthof(arr)	(sizeof(arr) / sizeof((arr)[0]))
#endif

#define USE_STRING_API

#ifdef USE_STRING_API
#undef _tcscpy
#undef _tcsicmp
#undef _tcsncpy
#undef _tcschr
#define _tcscpy			lstrcpy
#define _tcsicmp		lstrcmpi
#define _tcsncpy(d,s,l)	lstrcpyn((d), (s), (l) + 1)
#define _tcschr			StrChr
#endif /* USE_STRING_API */

#ifndef BELOW_NORMAL_PRIORITY_CLASS
#define BELOW_NORMAL_PRIORITY_CLASS		0x00004000
#define ABOVE_NORMAL_PRIORITY_CLASS		0x00008000
#endif /* BELOW_NORMAL_PRIORITY_CLASS */

#define PROGNAME	"startw"
#define COPYRIGHT_	"  Ver.1.11   Copyright (C) 2005-2014  K.Takata"
#define COPYRIGHT	PROGNAME COPYRIGHT_


LPTSTR getargs(LPCTSTR cmdline, LPTSTR arg, size_t size, int f_quote);
LPTSTR getenvarg(LPCTSTR cmdline, LPTSTR arg, size_t argsize,
		LPTSTR val, size_t valsize);
void usage();
int ShowExitCode(HANDLE hProcess, LPCTSTR arg, int f_show);
BOOL IsUserAdmin();


/*
 * WinMain
 */
int WINAPI _tWinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst,
		LPTSTR lpCmdLine, int iCmdShow)
{
	LPTSTR p, q, path = NULL;
	TCHAR buf[MAX_PATH];
	TCHAR arg[MAX_PATH];
	DWORD dwCreationFlags = 0;
	DWORD dwAffinity = 0;
	BOOL ret;
	int f_wait = 0;
	int f_exitcode = 0;
	int exitcode = 0;
	int f_admin = 0;
	int f_runas = 0;
	
	STARTUPINFO si;
	GetStartupInfo(&si);
	
	p = getargs(GetCommandLine(), NULL, 0, 0);
	while (p != NULL) {
		LPTSTR opt = arg + 1;
		q = getargs(p, arg, lengthof(arg), 0);
		if (_tcschr(arg, _T('=')) != NULL) {
			TCHAR val[MAX_PATH];
			p = q = getenvarg(p, arg, lengthof(arg), val, lengthof(val));
			SetEnvironmentVariable(arg, val[0] ? val : NULL);
			continue;
		}
		if (arg[0] != _T('/') && arg[0] != _T('-')) {
			break;
		}
		p = q;
		
		if (_tcsicmp(opt, _T("realtime")) == 0) {
			dwCreationFlags = REALTIME_PRIORITY_CLASS;
		} else if (_tcsicmp(opt, _T("high")) == 0) {
			dwCreationFlags = HIGH_PRIORITY_CLASS;
		} else if (_tcsicmp(opt, _T("abovenormal")) == 0) {
			dwCreationFlags = ABOVE_NORMAL_PRIORITY_CLASS;
		} else if (_tcsicmp(opt, _T("belownormal")) == 0) {
			dwCreationFlags = BELOW_NORMAL_PRIORITY_CLASS;
		} else if (_tcsicmp(opt, _T("normal")) == 0) {
			dwCreationFlags = NORMAL_PRIORITY_CLASS;
		} else if (_tcsicmp(opt, _T("low")) == 0
				|| _tcsicmp(opt, _T("idle")) == 0) {
			dwCreationFlags = IDLE_PRIORITY_CLASS;
		} else if (_tcsicmp(opt, _T("min")) == 0) {
			si.dwFlags |= STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_MINIMIZE;
		} else if (_tcsicmp(opt, _T("max")) == 0) {
			si.dwFlags |= STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_MAXIMIZE;
		} else if (_tcsicmp(opt, _T("hide")) == 0) {
			si.dwFlags |= STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;
		} else if (_tcsicmp(opt, _T("wait")) == 0) {
			f_wait = 1;
		} else if (_tcsicmp(opt, _T("admin")) == 0) {
			f_admin = 1;
		} else if (_tcsicmp(opt, _T("runas")) == 0) {
			f_runas = 1;
		} else if (_tcsicmp(opt, _T("z")) == 0) {
			f_exitcode = 1;
			f_wait = 1;
#ifdef UNICODE
		} else if (_tcsicmp(opt, _T("affinity")) == 0) {
			//dwAffinity = _tcstoul(p, NULL, 0);
			StrToIntEx(p, STIF_SUPPORT_HEX, (int*) &dwAffinity);
			p = q = getargs(q, NULL, 0, 0);
#endif
		} else if (opt[0] == _T('d') || opt[0] == _T('D')) {
			if (opt[1] == _T('\0')) {
				p = q = getargs(q, buf, lengthof(buf), 0);
			} else {
				_tcscpy(buf, opt + 1);
			}
			path = buf;
		} else if (opt[0] == _T('h') || opt[0] == _T('H')
				|| opt[0] == _T('?')) {
			p = NULL;
			break;
		}
	}
	if (p == NULL) {
		usage();
		return 0;
	}
	
	
#ifdef USE_CREATEPROCESS
	PROCESS_INFORMATION pi;
	ret = CreateProcess(NULL, p, NULL, NULL, FALSE, dwCreationFlags,
			NULL, path, &si, &pi);
	if (ret) {
		SetPriorityClass(pi.hProcess, dwCreationFlags);
		if (dwAffinity != 0) {
			SetProcessAffinityMask(sei.hProcess, dwAffinity);
		}
		if (f_wait) {
			WaitForSingleObject(pi.hProcess, INFINITE);
			exitcode = ShowExitCode(pi.hProcess, arg, f_exitcode);
		}
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
#else
	SHELLEXECUTEINFO sei;
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = NULL;
//	sei.lpVerb = _T("open");
	if (f_runas || (f_admin && !IsUserAdmin())) {
		sei.lpVerb = _T("runas");
	} else {
		sei.lpVerb = NULL;
	}
	sei.lpFile = arg;
	sei.lpParameters = q;
	sei.lpDirectory = path;
	sei.nShow = (si.dwFlags & STARTF_USESHOWWINDOW)
			? si.wShowWindow : SW_SHOWDEFAULT;
	ret = ShellExecuteEx(&sei);
	
	if (ret && (sei.hProcess != NULL)) {
		SetPriorityClass(sei.hProcess, dwCreationFlags);
#ifdef UNICODE
		if (dwAffinity != 0) {
			SetProcessAffinityMask(sei.hProcess, dwAffinity);
		}
#endif
		if (f_wait) {
			WaitForSingleObject(sei.hProcess, INFINITE);
			exitcode = ShowExitCode(sei.hProcess, arg, f_exitcode);
		}
		CloseHandle(sei.hProcess);
	}
#endif
	return exitcode;
}

int ShowExitCode(HANDLE hProcess, LPCTSTR arg, int f_show)
{
	TCHAR buf[128];
	DWORD exit = 0;
	
	if (GetExitCodeProcess(hProcess, &exit) && f_show) {
		wsprintf(buf, _T("ExitCode: %d"), exit);
		MessageBox(NULL, buf, arg, MB_OK);
	}
	return exit;
}


/*
 * usage
 */
void usage()
{
	// use ANSI string to reduce the program size
	LPSTR msg =
		COPYRIGHT "\n\n"
		"usage: " PROGNAME " [<option>] [<envname>=<value>] cmdline...\n"
		" <option>\n"
		"  -d<path> \tStarting directory\n"
		"  -min\t\tStart window minimized\n"
		"  -max\t\tStart window maximized\n"
		"  -hide\t\tStart window hidden\n"
		"  -realtime \tStart app in the REALTIME priority class\n"
		"  -high\t\tStart app in the HIGH priority class\n"
		"  -abovenormal\tStart app in the ABOVE_NORMAL priority class\n"
		"  -normal\t\tStart app in the NORMAL priority class\n"
		"  -belownormal\tStart app in the BELOW_NORMAL priority class\n"
		"  -low, -idle\tStart app in the IDLE priority class\n"
		"  -wait\t\tStart app and wait for it to end\n"
		"  -z\t\tShow the exit code\n"
		"  -admin\t\tStart app as an administrator\n"
#ifdef UNICODE
		"  -affinity <n>\tSet the process affinity mask\n"
#endif
		;
	
	MessageBoxA(NULL, msg, PROGNAME, MB_OK);
}


/*
 * getargs
 *
 * 先頭の引数を arg にコピー。
 * f_quote が 0 以外なら引用符('"', '\'')を含めてコピー。
 * 次の引数へのポインタを返す。次の引数がないときは NULL を返す。
 */
LPTSTR getargs(LPCTSTR cmdline, LPTSTR arg, size_t size, int f_quote)
{
	PTBYTE p = (PTBYTE) cmdline;
	PTBYTE start, end;
	TBYTE quote;
	size_t len;
	
	start = p;
	if (*p == _T('"') || *p == _T('\'')) {
		quote = *p;
		++p;
		if (!f_quote)
			start = p;
		while (*p && (*p != quote))
			++p;
		end = p;
		if (*p == quote) {
			++p;
			if (f_quote)
				end = p;
		}
	} else {
		while (*p > _T(' '))
			++p;
		end = p;
	}
	if (arg != NULL && size > 0) {
		len = end - start;
		if (len > size - 1)
			len = size - 1;
		_tcsncpy(arg, (LPTSTR) start, len);
#ifndef USE_STRING_API
		arg[len] = _T('\0');
#endif
	}
	while (*p && (*p <= _T(' ')))
		++p;
	if (*p == _T('\0'))
		return NULL;
	return (LPTSTR) p;
}

/*
 * getenvarg
 *
 * 環境変数の引数を解析し、変数名を arg に、値を val にコピー。
 * 次の引数へのポインタを返す。次の引数がないときは NULL を返す。
 */
LPTSTR getenvarg(LPCTSTR cmdline, LPTSTR arg, size_t argsize,
		LPTSTR val, size_t valsize)
{
	PTBYTE p = (PTBYTE) cmdline;
	PTBYTE start, end;
	TBYTE quote = _T('\0');
	size_t len;
	
	start = p;
	if (*p == _T('"') || *p == _T('\'')) {
		quote = *p;
		++p;
		start = p;
		while (*p && (*p != quote) && (*p != _T('=')))
			++p;
		end = p;
		if (*p == quote) {
			++p;
			quote = _T('\0');
		}
	} else {
		while (*p != _T('='))
			++p;
		end = p;
	}
	if (arg != NULL && argsize > 0) {
		len = end - start;
		if (len > argsize - 1)
			len = argsize - 1;
		_tcsncpy(arg, (LPTSTR) start, len);
#ifndef USE_STRING_API
		arg[len] = _T('\0');
#endif
	}
	++p;		// skip '='
	start = p;
	if (quote == _T('\0') && (*p == _T('"') || *p == _T('\''))) {
		quote = *p;
		++p;
		start = p;
	}
	if (quote != _T('\0')) {
		while (*p && (*p != quote))
			++p;
		end = p;
		if (*p == quote) {
			++p;
			quote = _T('\0');
		}
	} else {
		while (*p > _T(' '))
			++p;
		end = p;
	}
	if (val != NULL && valsize > 0) {
		len = end - start;
		if (len > valsize - 1)
			len = valsize - 1;
		_tcsncpy(val, (LPTSTR) start, len);
#ifndef USE_STRING_API
		val[len] = _T('\0');
#endif
	}
	while (*p && (*p <= _T(' ')))
		++p;
	if (*p == _T('\0'))
		return NULL;
	return (LPTSTR) p;
}


typedef BOOL (WINAPI *pfnIsUserAnAdmin)(void);

BOOL IsUserAdmin()
{
//	HMODULE hShell32 = LoadLibrary(_T("shell32.dll"));
	HMODULE hShell32 = GetModuleHandleA("shell32.dll");
	if (hShell32 == NULL) {
		return TRUE;
	}
	BOOL fIsAdmin = TRUE;
	pfnIsUserAnAdmin pIsUserAnAdmin = (pfnIsUserAnAdmin)
			GetProcAddress(hShell32, (LPCSTR) 680);
	if (pIsUserAnAdmin != NULL) {
		fIsAdmin = pIsUserAnAdmin();
	}
//	FreeLibrary(hShell32);
	return fIsAdmin;
}
