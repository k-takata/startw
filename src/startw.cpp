/*
 * startw  Ver.1.04   Copyright (C) 2005  K.Takata
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <string.h>

#define USE_STRING_API

#ifdef USE_STRING_API
#define strcpy			lstrcpy
#define stricmp			lstrcmpi
#define strncpy(d,s,l)	lstrcpyn((d), (s), (l) + 1)
#endif /* USE_STRING_API */

#ifndef BELOW_NORMAL_PRIORITY_CLASS
#define BELOW_NORMAL_PRIORITY_CLASS		0x00004000
#define ABOVE_NORMAL_PRIORITY_CLASS		0x00008000
#endif /* BELOW_NORMAL_PRIORITY_CLASS */

#define PROGNAME	"startw"
#define COPYRIGHT	\
	PROGNAME##"  Ver.1.04   Copyright (C) 2005  K.Takata"


char *getargs(const char *cmdline, char *arg, size_t size, int f_quote);
void usage();
int ShowExitCode(HANDLE hProcess, const char *arg, int f_show);


/*
 * WinMain
 */
int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst,
		LPSTR lpCmdLine, int iCmdShow)
{
	char *p, *q, *path = NULL;
	char buf[MAX_PATH];
	char arg[MAX_PATH];
	DWORD dwCreationFlags = 0;
	BOOL ret;
	int f_wait = 0;
	int f_exitcode = 0;
	int exitcode = 0;
	
	STARTUPINFO si;
	GetStartupInfo(&si);
	
	p = getargs(GetCommandLine(), NULL, 0, 0);
	while (p != NULL) {
		char *opt = arg + 1;
		q = getargs(p, arg, sizeof(arg), 0);
		if (arg[0] != '/' && arg[0] != '-') {
			break;
		}
		p = q;
		
		if (stricmp(opt, "realtime") == 0) {
			dwCreationFlags = REALTIME_PRIORITY_CLASS;
		} else if (stricmp(opt, "high") == 0) {
			dwCreationFlags = HIGH_PRIORITY_CLASS;
		} else if (stricmp(opt, "abovenormal") == 0) {
			dwCreationFlags = ABOVE_NORMAL_PRIORITY_CLASS;
		} else if (stricmp(opt, "belownormal") == 0) {
			dwCreationFlags = BELOW_NORMAL_PRIORITY_CLASS;
		} else if (stricmp(opt, "normal") == 0) {
			dwCreationFlags = NORMAL_PRIORITY_CLASS;
		} else if (stricmp(opt, "low") == 0 || stricmp(opt, "idle") == 0) {
			dwCreationFlags = IDLE_PRIORITY_CLASS;
		} else if (stricmp(opt, "min") == 0) {
			si.dwFlags |= STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_MINIMIZE;
		} else if (stricmp(opt, "max") == 0) {
			si.dwFlags |= STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_MAXIMIZE;
		} else if (stricmp(opt, "hide") == 0) {
			si.dwFlags |= STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;
		} else if (stricmp(opt, "wait") == 0) {
			f_wait = 1;
		} else if (stricmp(opt, "z") == 0) {
			f_exitcode = 1;
			f_wait = 1;
		} else if (opt[0] == 'd' || opt[0] == 'D') {
			if (opt[1] == '\0') {
				p = q = getargs(q, buf, sizeof(buf), 0);
			} else {
				strcpy(buf, opt + 1);
			}
			path = buf;
		} else if (opt[0] == 'h' || opt[0] == 'H' || opt[0] == '?') {
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
//	sei.lpVerb = "open";
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

int ShowExitCode(HANDLE hProcess, const char *arg, int f_show)
{
	char buf[128];
	DWORD exit = 0;
	
	if (GetExitCodeProcess(hProcess, &exit) && f_show) {
		wsprintf(buf, "ExitCode: %d", exit);
		MessageBox(NULL, buf, arg, MB_OK);
	}
	return exit;
}


/*
 * usage
 */
void usage()
{
	char *msg =
		COPYRIGHT "\n\n"
		"usage: " PROGNAME " [<option>] cmdline...\n"
		" <option>\n"
		"  -d<path>     Starting directory\n"
		"  -min         Start window minimized\n"
		"  -max         Start window maximized\n"
		"  -hide        Start window hidden\n"
		"  -realtime    Start app in the REALTIME priority class\n"
		"  -high        Start app in the HIGH priority class\n"
		"  -abovenormal Start app in the ABOVE_NORMAL priority class\n"
		"  -normal      Start app in the NORMAL priority class\n"
		"  -belownormal Start app in the BELOW_NORMAL priority class\n"
		"  -low, -idle  Start app in the IDLE priority class\n"
		"  -wait        Start app and wait for it to end\n"
		"  -z           Show the exit code\n"
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
char *getargs(const char *cmdline, char *arg, size_t size, int f_quote)
{
	unsigned char *p = (unsigned char *) cmdline;
	unsigned char *start, *end;
	unsigned char quote;
	size_t len;
	
	start = p;
	if (*p == '"' || *p == '\'') {
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
		while (*p > ' ')
			++p;
		end = p;
	}
	if (arg != NULL && size > 0) {
		len = end - start;
		if (len > size - 1)
			len = size - 1;
		strncpy(arg, (char *) start, len);
#ifndef USE_STRING_API
		arg[len] = '\0';
#endif
	}
	while (*p && (*p <= ' '))
		++p;
	if (*p == '\0')
		return NULL;
	return (char *) p;
}
