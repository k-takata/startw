/*
 *	tnywmain.c
 */

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOIME
#include <windows.h>

#ifdef USE_CMDSHOW
#define NAKED
#else
#define NAKED	__declspec(naked)
#endif

extern int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow);

NAKED void /*__cdecl*/ WinMainCRTStartup(void)
{
	LPTSTR cmdline;
	int cmdshow;
	
#ifdef USE_CMDSHOW
	STARTUPINFO si;
	
	GetStartupInfo(&si);
	cmdshow = (si.dwFlags & STARTF_USESHOWWINDOW)
			? si.wShowWindow : SW_SHOWDEFAULT;
#else
	cmdshow = 0;
#endif

#ifdef USE_CMDLINE
	cmdline = GetCommandLine();
#else
	cmdline = NULL;
#endif
	
	ExitProcess(WinMain(GetModuleHandle(NULL), NULL, cmdline, cmdshow));
}
