// dlgChat.h : Handles the chat dialog box.

#ifndef _DLGCHAT_H_
#define _DLGCHAT_H_

// Create the chat dialogbox.
//
// In:  lParam  User info passed to the DlgChatProc function
//              and processed in WM_INITDIALOG. 
//
// Return: true if created false otherwise.
bool CreateChatDialog(dlgparam_t *lParam);

// Callback function for chat dialog box
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
LRESULT CALLBACK DlgChatProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Initializes the dialog box controls
//
// In: hDlg     handle to the chat dialog box.
//     name     the name of the client.
void InitializeChatDlg(HWND hDlg, const char *name);

// Sends a chat message to the server.
//
// In:  hDlg    Handle to the chat dialog box.   
void OnSendBtnClick(HWND hDlg);

// Clear the text in the client's rich edit box.
//
// In:  hDlg    Handle to the chat dialog box.
void OnClearBtnClickClient(HWND hDlg);

#endif