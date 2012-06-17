/*
 * tnywmain.c
 */
/*
 * Copyright (C) 2005-2012, K.Takata
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
#define NOGDI
#define NOIME
#include <windows.h>
#include <tchar.h>

#if defined(USE_CMDSHOW) || defined(_WIN64)
#define NAKED
#else
// note: compile option /O1 or /O2 is needed
#define NAKED	__declspec(naked)
#endif

extern int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow);
extern int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPWSTR lpCmdLine, int nCmdShow);

#ifdef UNICODE
#define _tWinMainCRTStartup wWinMainCRTStartup
#else
#define _tWinMainCRTStartup WinMainCRTStartup
#endif

NAKED void _tWinMainCRTStartup(void)
{
#ifdef USE_CMDLINE
	TBYTE *lpszCommandLine;	// must be unsigned for multibyte strings
#endif
	LPTSTR cmdline;
	int cmdshow;
	HANDLE hModule;
	
#ifdef USE_CMDSHOW
	STARTUPINFO si;
	
	GetStartupInfo(&si);
	cmdshow = (si.dwFlags & STARTF_USESHOWWINDOW)
			? si.wShowWindow : SW_SHOWDEFAULT;
#else
	cmdshow = 0;
#endif

#ifdef USE_CMDLINE
	lpszCommandLine = GetCommandLine();
	
	// Skip past program name (first token in command line)
	
	// Check for and handle quoted program name
	if (*lpszCommandLine == '"') {
		// Scan, and skip over, subsequent characters until another
		// double-quote or a null is encountered
		do {
			lpszCommandLine++;
		} while (*lpszCommandLine && (*lpszCommandLine != '"'));
		
		// If we stopped on a double-quote (usual case), skip over it.
		if (*lpszCommandLine == '"')
			lpszCommandLine++;
	} else {
		// First token wasn't a quote
		while (*lpszCommandLine > ' ')
			lpszCommandLine++;
	}
	
	// skip past any white space preceeding the second token.
	while (*lpszCommandLine && (*lpszCommandLine <= ' '))
		lpszCommandLine++;
	
	cmdline = lpszCommandLine;
#else
	cmdline = NULL;
#endif
	
#ifdef USE_MODULEHANDLE
	hModule = GetModuleHandle(NULL);
#else
	hModule = NULL;
#endif
	ExitProcess(_tWinMain(hModule, NULL, cmdline, cmdshow));
}
