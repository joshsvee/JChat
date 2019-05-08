
#ifndef _SYS_WIN_H_
#define _SYS_WIN_H_

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				    // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0501		    // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		    // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0501		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		    // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410   // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			    // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0601	    // Change this to the appropriate value to target IE 5.0 or later.
#endif

#include <winsock2.h>

// Max length of any name.
#define NAME_LEN 31

// For printing messages to the richedit box. Both client and server use this.
// WPARAM - The string to display (allocated on the heap).
// LPARAM - Not used. Should be zero.
// NOTE: The message handler should delete the memory.
#define UWM_PRINT_TEXT_MSG      (WM_APP + 1)

// Adds a string to the client's combo box and sets the data associated with the string.
//
// WPARAM - The string to add (alocated on the heap).
// LPARAM - The buddy id number associated with the string.
// NOTE: The message handler should delete the memory.
#define UWM_CB_ADDSTRING_MSG    (WM_APP + 2)

// Dialog box param.
// Passed as lParam to a DlgProc.
struct dlgparam_t
{
    // Server IPv4 address.
    char szIp[20];

    // Server Port.
    unsigned short port;

    // Player 's name.
    char szName[NAME_LEN];
};

// Displays a message box and prints an error message and shuts down the application.
//
// In:  linedesc    Function where the error occured.
//      filename    The file where the error occured.
//      lineno      The line number for where the error occured.
//      errnum      The error number from GetLastError()
void PrintError(LPSTR linedesc, LPSTR filename, int lineno, DWORD errnum);

// Verify function calls and prints an error on failure.
#define MTVERIFY(a) if (!(a)) PrintError(#a, __FILE__, __LINE__, GetLastError())

#endif