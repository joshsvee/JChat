// dlgSessionProc.h : Handles main dialog session on app start.

#ifndef _DLGSESSION_H_
#define _DLGSESSION_H_

// Callback function for session dialog box.
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
LRESULT CALLBACK DlgSessionProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Initializes the dialog box controls
//
// In: hDlg     handle to the session dialog box.
void InitializeSessionDlg(HWND hDlg);

// Create server button clicked.
//
// In: hDlg handle to the session dialog box.
//
// Return: true if successful else return false.
bool OnCreateBtnClick(HWND hDlg);

// Connect button clicked.
//
// In: hDlg handle to the session dialog box.
//
// Return: true if successful else return false.
bool OnConnectBtnClick(HWND hDlg);

// Update the UI when the radio button is clicked.
//
//  In: hDlg handle to the session dialog box.
void UpDateUi(HWND hDlg);

#endif