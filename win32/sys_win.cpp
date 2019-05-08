
#include "sys_win.h"

// Displays a message box and prints an error message and shuts down the application.
//
// In:  linedesc    Function where the error occured.
//      filename    The file where the error occured.
//      lineno      The line number for where the error occured.
//      errnum      The error number from GetLastError()
void PrintError(LPSTR linedesc, LPSTR filename, int lineno, DWORD errnum)
{
	LPSTR lpBuffer;
	char errbuf[256];
	char modulename[MAX_PATH];


	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errnum, LANG_NEUTRAL, (LPTSTR)&lpBuffer, 0, NULL);

	wsprintf(errbuf, "\nThe following call failed at line %d in %s:\n\n"
               "    %s\n\nReason: %s\n", lineno, filename, linedesc, lpBuffer);

	GetModuleFileName(NULL, modulename, MAX_PATH);
	MessageBox(NULL, errbuf, modulename, MB_ICONWARNING|MB_OK|MB_TASKMODAL|MB_SETFOREGROUND);

	exit(EXIT_FAILURE);
}
