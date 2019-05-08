// dlgServerConsole.h : Handles the server console dialog box.

#ifndef _DLG_SERVER_CONSOLE_H_
#define _DLG_SERVER_CONSOLE_H_

// Create the server dialogbox.
//
// Return: true if created false otherwise.
bool CreateServerDialog(dlgparam_t *lParam);

// Handle server dialog box message translation (keyboard etc...).
//
// Return: -1 on error.
int RunServerDialogMsgPump(void);

// Callback function for server console dialog box
// Used for message processing and event handling.
//
// In:      HWND    hDlg   -- handle to the dialog window
//          UINT    msg    -- the message to process
//          WPARAM  wParam -- The low-order word is the control identifier. 
//		                   -- The high-order word is the notification 
//                            message.
//          LAPARAM lParam -- Handle to the control
//
// Return:  TRUE if the message was handled FALSE otherwise.
LRESULT CALLBACK DlgServerConsoleProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// Initializes the dialog box controls
//
// In: hDlg         handle to the session dialog box.
//     serverName   The name of the server.
void InitializeServerConsoleDlg(HWND hDlg, const char *serverName);

// Clear the text in the server's rich edit box.
//
// In:  hDlg    Handle to the dialog box.
void OnClearBtnClickServer(HWND hDlg);

#endif