// dlgServerConsole.cpp : Handles the server console dialog box.


#include "sys_win.h"
#include <atlstr.h>
#include "resource.h"
#include "richedit.h"
#include "dlgChat.h"
#include "server.h"
#include "dlgServerConsole.h"
#include "CriticalSection.h"

/***********/
/* GLOBALS */
/***********/

// App instance handle.
extern HINSTANCE hInst;

// Server Thread vars.
extern volatile bool bTerminateServerThread;

// Session dialog handle.
extern HWND hSessionDlg;

// Server dialog handle.
extern HWND hServerDlg;

// Brush for the dlg background color.
static HBRUSH ServerDlgBkBrush = 0;

// Create the server dialogbox.
//
// Return: true if created false otherwise.
bool CreateServerDialog(dlgparam_t *lParam)
{
    hServerDlg = CreateDialogParam(hInst, 
                MAKEINTRESOURCE(IDD_SERV), 
                0, (DLGPROC)DlgServerConsoleProc, (LPARAM)lParam);

    if (!hServerDlg)
        return false;

    // Display the server dialog box.
    ShowWindow(hServerDlg, SW_SHOWNORMAL);

    // Hide the session dialog.
    ShowWindow(hSessionDlg, SW_HIDE);

    return true;
}

// Callback function for server console dialog box
// Used for message processing and event handling.
//
// In:      HWND    hDlg   -- handle to the dialog window
//          UINT    msg    -- the message to process
//          WPARAM  wParam -- The low-order word is the control identifier.
//                         -- The high-order word is the notification 
//                            message.
//          LAPARAM lParam -- Handle to the control or user defined data.
//
// Return:  TRUE if the message was handled FALSE otherwise.
LRESULT CALLBACK DlgServerConsoleProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_INITDIALOG:
        {
            hServerDlg = hDlg;

            dlgparam_t *serverInfo = (dlgparam_t *)lParam;

            // Init the dialog controls.
            InitializeServerConsoleDlg(hDlg, serverInfo->szName);

            // In case they reinitialize the server.
            bTerminateServerThread = false;

            // Init the server.
            if (!SV_Init(serverInfo->szName, serverInfo->port))
            {
                char szErrMsg[MAX_STATUS_MSG];

                sprintf(szErrMsg, "Could not create the server\n Error: %s", NET_GetErrorString());

                MessageBox(hDlg, szErrMsg, "Server Error", MB_OK | MB_ICONERROR);

                // Just kill the dialog. No point in creatng it if we can't create the server.
                // NOTE: This will cause the CreateServerDialog function to return false.
                DestroyWindow(hServerDlg);

                hServerDlg = 0;

                return FALSE;
            }

            // FALSE since we set the focus to a control and not the 
            // dlgbox.
            return FALSE;
        }
        break;

        case UWM_PRINT_TEXT_MSG:
        {
            extern CriticalSection g_HeapCS;

            CHARRANGE ch;
            ch.cpMin = -1;
            ch.cpMax = -1;

            g_HeapCS.Enter();

                char *szText = (char *)wParam;

                SendMessage(GetDlgItem(hServerDlg, IDC_RICHEDIT_SERVER_CON), 
                            EM_EXSETSEL, 0, (LPARAM)&ch);

                SendMessage(GetDlgItem(hServerDlg, IDC_RICHEDIT_SERVER_CON), 
                            EM_REPLACESEL, 0, (LPARAM)szText);

                SendMessage(GetDlgItem(hServerDlg, IDC_RICHEDIT_SERVER_CON), 
                            WM_VSCROLL, SB_BOTTOM, (LPARAM)NULL);

                delete [] szText;

            g_HeapCS.Leave();
        }
        break;

        case WM_CTLCOLORDLG:
        {
            if (ServerDlgBkBrush)
            {
                DeleteObject(ServerDlgBkBrush);
            }
            ServerDlgBkBrush = CreateSolidBrush(RGB(0xE0, 0xE0, 0xE0));
            return (LRESULT)ServerDlgBkBrush;
        }

        case WM_COMMAND:
        {
            // Find which control was graced.
            switch (LOWORD(wParam))
            {
                // Exit button.
                case IDCANCEL:
                { 
                    // Show the session dialog.
                    ShowWindow(hSessionDlg, SW_RESTORE);
                    DestroyWindow(hDlg);
                }
                break;

                // Clear button.
                case IDC_BTN_CLEAR_SERVER:
                {
                    OnClearBtnClickServer(hDlg);
                }
                break;
            }
        }
        break; // WM_COMMAND

        case WM_DESTROY:
        {
            // Shutdown the server.
            // NOTE: This is the only place this function should get called.
            SV_ShutDown();

            if (ServerDlgBkBrush)
            {
                DeleteObject(ServerDlgBkBrush);
            }
            
            ServerDlgBkBrush = 0;
            hServerDlg = 0;
        }
        break;

        default:
            // Message not handled.
            return FALSE;
    }
    return TRUE;
}

// Initializes the dialog box controls
//
// In: hDlg         handle to the session dialog box.
//     serverName   The name of the server.
void InitializeServerConsoleDlg(HWND hDlg, const char *serverName)
{
    HICON hIcon;
    COLORREF color;
    CHARFORMAT2 charFormat;

    // Load and set the icon.
    hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    SendMessage(hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);

    // Let them see the server name in the title bar.
    CString dlgTitle = "Server - ";
    dlgTitle += serverName;
    SetWindowText(hDlg, dlgTitle);

    // Set the background color for the richedit box.
    color = RGB(0, 0, 255);
    SendMessage(GetDlgItem(hDlg, IDC_RICHEDIT_SERVER_CON), 
                EM_SETBKGNDCOLOR, 0, color);
    
    // Use plain text formatting.
    SendMessage(GetDlgItem(hDlg, IDC_RICHEDIT_SERVER_CON), 
                EM_SETTEXTMODE, TM_PLAINTEXT, 0);

    // Set the look of the text.
    memset(&charFormat, 0, sizeof(CHARFORMAT2));
    charFormat.cbSize = sizeof(CHARFORMAT2);
    charFormat.dwMask = CFM_WEIGHT | CFM_COLOR | CFM_FACE | CFM_SIZE;//CFM_BOLD;
    charFormat.crTextColor = RGB(255, 252, 0);
    charFormat.dwEffects = CFE_BOLD;
    charFormat.bCharSet = ANSI_CHARSET;
    charFormat.wWeight = FW_LIGHT;
    charFormat.yHeight = 225;
    charFormat.bPitchAndFamily = FF_MODERN | FIXED_PITCH;
    strcpy(charFormat.szFaceName, "Courier New");
    
    SendMessage(GetDlgItem(hDlg, IDC_RICHEDIT_SERVER_CON),  
                EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&charFormat);

    // Set the focus to the clear button.
    SetFocus(GetDlgItem(hDlg, IDC_BTN_CLEAR_SERVER));
}

// Clear the text in the server's rich edit box.
//
// In:  hDlg    Handle to the dialog box.
void OnClearBtnClickServer(HWND hDlg)
{
    SetDlgItemText(hDlg, IDC_RICHEDIT_SERVER_CON , "");
}