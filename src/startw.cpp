/*
 * startw  Ver.1.06   Copyright (C) 2005-2007  K.Takata
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <string.h>
#include <tchar.h>

#define USE_STRING_API

#ifdef USE_STRING_API
#undef _tcscpy
#undef _tcsicmp
#undef _tcsncpy
#define _tcscpy			lstrcpy
#define _tcsicmp		lstrcmpi
#define _tcsncpy(d,s,l)	lstrcpyn((d), (s), (l) + 1)
#endif /* USE_STRING_API */

#ifndef BELOW_NORMAL_PRIORITY_CLASS
#define BELOW_NORMAL_PRIORITY_CLASS		0x00004000
#define ABOVE_NORMAL_PRIORITY_CLASS		0x00008000
#endif /* BELOW_NORMAL_PRIORITY_CLASS */

#define PROGNAME	_T("startw")
#define COPYRIGHT_	_T("  Ver.1.06   Copyright (C) 2005-2007  K.Takata")
#define COPYRIGHT	PROGNAME COPYRIGHT_


LPTSTR getargs(LPCTSTR cmdline, LPTSTR arg, size_t size, int f_quote);
void usage();
int ShowExitCode(HANDLE hProcess, LPCTSTR arg, int f_show);
BOOL IsUserAdmin();


/*
 * WinMain
 */
int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst,
		LPSTR lpCmdLine, int iCmdShow)
{
	LPTSTR p, q, path = NULL;
	TCHAR buf[MAX_PATH];
	TCHAR arg[MAX_PATH];
	DWORD dwCreationFlags = 0;
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
		q = getargs(p, arg, sizeof(arg), 0);
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
		} else if (opt[0] == _T('d') || opt[0] == _T('D')) {
			if (opt[1] == _T('\0')) {
				p = q = getargs(q, buf, sizeof(buf), 0);
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
		if (f_wait) {
			WaitForSingleObject(pi.hProcess, INFINITE);
			exitcode = ShowExitCode(pi.hProcess, arg, f_exitcode);
		}
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
#else
	SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
//	sei.lpVerb = _T("open");
	if (f_runas || (f_admin && !IsUserAdmin())) {
		sei.lpVerb = _T("runas");
	}
	sei.lpFile = arg;
	sei.lpParameters = q;
	sei.nShow = (si.dwFlags & STARTF_USESHOWWINDOW)
			? si.wShowWindow : SW_SHOWDEFAULT;
	sei.lpDirectory = path;
	ret = ShellExecuteEx(&sei);
	
	if (ret && (sei.hProcess != NULL)) {
		SetPriorityClass(sei.hProcess, dwCreationFlags);
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
	LPTSTR msg =
		COPYRIGHT _T("\n\n")
		_T("usage: ") PROGNAME _T(" [<option>] cmdline...\n")
		_T(" <option>\n")
		_T("  -d<path>     Starting directory\n")
		_T("  -min         Start window minimized\n")
		_T("  -max         Start window maximized\n")
		_T("  -hide        Start window hidden\n")
		_T("  -realtime    Start app in the REALTIME priority class\n")
		_T("  -high        Start app in the HIGH priority class\n")
		_T("  -abovenormal Start app in the ABOVE_NORMAL priority class\n")
		_T("  -normal      Start app in the NORMAL priority class\n")
		_T("  -belownormal Start app in the BELOW_NORMAL priority class\n")
		_T("  -low, -idle  Start app in the IDLE priority class\n")
		_T("  -wait        Start app and wait for it to end\n")
		_T("  -z           Show the exit code\n")
		_T("  -admin       Start app as an administrator\n")
		;
	
	MessageBox(NULL, msg, PROGNAME, MB_OK);
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


typedef BOOL (WINAPI *pfnIsUserAnAdmin)(void);

BOOL IsUserAdmin()
{
//	HMODULE hShell32 = LoadLibrary(_T("shell32.dll"));
	HMODULE hShell32 = GetModuleHandle(_T("shell32.dll"));
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
