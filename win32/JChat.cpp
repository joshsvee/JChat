// jchat.cpp : Main entry point for the application.
// NOTE      : All dialogs created as modeless.

#include <cstring>
#include <atlstr.h>
#include "resource.h"
#include "net_common.h"
#include "dlgSession.h"
#include "commctrl.h"
#include "CriticalSection.h"

#pragma comment(lib, "Ws2_32.lib")      // For winsock<2>.
#pragma comment(lib, "comctl32.lib")    // Common control types.

/***********/
/* GLOBALS */
/***********/

// Used to protect shared data on the client and server.
// We can use the same object for both since only one instance
// is ever running at any given time.
CriticalSection g_cs;

// Protectes heap data allocated / used across threads.
CriticalSection g_HeapCS;

// Application instance handle.
HINSTANCE hInst = 0;

// Handle to the session dialog box.
HWND hSessionDlg = 0;

// Handle to the chat dialog box.
HWND hChatDlg = 0;

// Server dialog handle.
HWND hServerDlg = 0;

// Main entry point for the application
//
// In:  hInstance       Application instance.
//      hPrevInstance   no prev in WIN32
//      lpCmdLine       cmd line args
//      nShowCmd        how the window should be displayed
//
// Return: 0
int APIENTRY WinMain(HINSTANCE hInstance,		// application instance
					 HINSTANCE hPrevInstance,	// no previnst in WIN32
					 LPSTR lpCmdLine,			// cmd line args
					 int nShowCmd)				// how the window should be displayed (min/max)
{

#if 1

    // Application instance handle.
    hInst = hInstance;

    // Needed for rich edit control.
    HMODULE richEditLib = LoadLibraryA("RICHED20.DLL");

    if (!richEditLib)
    {
        MessageBox(0, "Could not load RICHED20.DLL", "Application Error",
                    MB_OK | MB_ICONERROR);
        return 0;
    }

    // Create the main session dialog.
    hSessionDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SESSION_MANAGER),
                    NULL, (DLGPROC)DlgSessionProc);

    if (!hSessionDlg)
    {
        MessageBox(0, "Could not create application dialog!", "Application Error",
                    MB_OK | MB_ICONERROR);

        // Done with the rich edit .dll
        FreeLibrary(richEditLib);
        return 0;
    }

    // Display the session dialog box.
    ShowWindow(hSessionDlg, SW_SHOWNORMAL);
    
    // Windows message.
    MSG msg;

    // Handle to the active window.
    HWND hActive;
    
    // Error code.
    BOOL iErr;

    // Handle the messages for any window that belongs to the current thread.
    // NOTES:   NULL hWnd parameter to establish the above behavior.
    //          Returns 0 on WM_QUIT message.
    //          The session dialog will destroy the window.
    while ((iErr = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        // Error. 
        if (iErr == -1)
        {
            break;
        }

        // Get the current active window handle.
        hActive = GetActiveWindow();

        // If it is not a dialog box message let the system handle it.
        if (!IsWindow(hActive) || !IsDialogMessage(hActive, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Done with the rich edit .dll
    FreeLibrary(richEditLib);

#endif

    return 0;
}
