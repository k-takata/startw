/*
 * startw  Ver.1.00   Copyright (C) 2005  K.Takata
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <string.h>

#define strcpy			lstrcpy
#define stricmp			lstrcmpi
#define strncpy(d,s,l)	lstrcpyn((d), (s), (l) + 1)

#ifndef BELOW_NORMAL_PRIORITY_CLASS
#define BELOW_NORMAL_PRIORITY_CLASS		0x00004000
#define ABOVE_NORMAL_PRIORITY_CLASS		0x00008000
#endif /* BELOW_NORMAL_PRIORITY_CLASS */

#define PROGNAME	"startw"
#define COPYRIGHT	\
	PROGNAME##"  Ver.1.00   Copyright (C) 2005  K.Takata"


char *getargs(const char *cmdline, char *arg, int f_quote);
void usage();


/*
 * WinMain
 */
int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst,
		LPSTR lpCmdLine, int iCmdShow)
{
	char *p, *q;
	char path[MAX_PATH] = "";
	char arg[MAX_PATH];
	DWORD dwCreationFlags = 0;
	BOOL ret;
	int f_wait = 0;
	
	STARTUPINFO si;
	GetStartupInfo(&si);
	
	p = getargs(GetCommandLine(), NULL, 0);
	while (p != NULL) {
		char *opt = arg + 1;
		q = getargs(p, arg, 0);
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
		} else if (stricmp(opt, "wait") == 0) {
			f_wait = 1;
		} else if (opt[0] == 'd' || opt[0] == 'D') {
			if (opt[1] == '\0') {
				p = q = getargs(q, path, 0);
			} else {
				strcpy(path, opt + 1);
			}
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
	char *dir = NULL;
	if (path[0] != '\0') {
		dir = path;
	}
	ret = CreateProcess(NULL, p, NULL, NULL, FALSE, dwCreationFlags,
			NULL, dir, &si, &pi);
	if (f_wait) {
		WaitForSingleObject(pi.hProcess, INFINITE);
	}
	if (ret) {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
#else
	SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
//	sei.lpVerb = "open";
	sei.lpFile = arg;
	sei.lpParameters = q;
	if (si.dwFlags & STARTF_USESHOWWINDOW) {
		sei.nShow = si.wShowWindow;
	} else {
		sei.nShow = SW_SHOWDEFAULT;
	}
	if (path[0] != '\0') {
		sei.lpDirectory = path;
	}
	ret = ShellExecuteEx(&sei);
	
	if (ret && (sei.hProcess != NULL)) {
		SetPriorityClass(sei.hProcess, dwCreationFlags);
		if (f_wait) {
			WaitForSingleObject(sei.hProcess, INFINITE);
		}
		CloseHandle(sei.hProcess);
	}
#endif
	return 0;
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
		"  -realtime    Start app in the REALTIME priority class\n"
		"  -high        Start app in the HIGH priority class\n"
		"  -abovenormal Start app in the ABOVE_NORMAL priority class\n"
		"  -normal      Start app in the NORMAL priority class\n"
		"  -belownormal Start app in the BELOW_NORMAL priority class\n"
		"  -low, -idle  Start app in the IDLE priority class\n"
		"  -wait        Start app and wait for it to end\n"
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
char *getargs(const char *cmdline, char *arg, int f_quote)
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
	if (arg != NULL) {
		len = end - start;
		strncpy(arg, (char *) start, len);
		arg[len] = '\0';
	}
	while (*p && (*p <= ' '))
		++p;
	if (*p == '\0')
		return NULL;
	return (char *) p;
}
