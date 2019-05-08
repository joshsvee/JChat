// dlgChatProc.cpp : Handles chat dialog box messages.

#include "net_common.h"
#include "resource.h"
#include "dlgChat.h"
#include "cl_client.h"
#include "commctrl.h"
#include "richedit.h"
#include "CriticalSection.h"

/***********/
/* GLOBALS */
/***********/

// App instance handle.
extern HINSTANCE hInst;

// Session dialog handle.
extern HWND hSessionDlg;

// Handle to the chat dialog box.
extern HWND hChatDlg;

// Brush for the dlg background color.
static HBRUSH ChatDlgBkBrush = 0;

// Create the chat dialogbox.
//
// In:  lParam  User info passed to the DlgChatProc function
//              and processed in WM_INITDIALOG. 
//
// Return: true if created false otherwise.
bool CreateChatDialog(dlgparam_t *lParam)
{

    hChatDlg = CreateDialogParam(hInst, 
                MAKEINTRESOURCE(IDD_CHAT), 
                0, (DLGPROC)DlgChatProc, (LPARAM)lParam);

    if (!hChatDlg)
        return false;

    // Display the chat dialog box.
    ShowWindow(hChatDlg, SW_SHOWNORMAL);

    // Hide the session dialog.
    ShowWindow(hSessionDlg, SW_HIDE);

    return true;
}

// Callback function for chat dialog box
// Used for message processing and event handling.
//
// In:      HWND    hDlg   -- handle to the dialog window
//          UINT    msg    -- the message to process
//          WPARAM  wParam -- The low-order word is the control identifier. 
//                         -- The high-order word is the notification 
//                            message.
//          LAPARAM lParam -- Handle to the control or user defined data.
//
// Return:  TRUE if the message was handled FALSE otherwise
LRESULT CALLBACK DlgChatProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{     
    switch (msg)
    {
        case WM_INITDIALOG:
        {
            // NOTE:    Set here since the CreateDialogParam doesn't return the
            //          HWND until this message is processed.
            hChatDlg = hDlg;

            dlgparam_t *clientInfo = (dlgparam_t *)lParam;

            // Init the dialog controls.
            InitializeChatDlg(hDlg, clientInfo->szName);

            // Init the local client and Connect to the server.
            if (!CL_Init(clientInfo->szIp, clientInfo->port, clientInfo->szName))
            {
                char szErrMsg[MAX_STATUS_MSG];

                sprintf(szErrMsg, "Could not connect to %s:%d\n Error: %s", 
                clientInfo->szIp, clientInfo->port, NET_GetErrorString());

                MessageBox(hDlg, szErrMsg, "Connect Error", MB_OK | MB_ICONERROR);

                // Just kill the dialog. No point in creatng it if we can't connect.
                // NOTE: This will cause the CreateChatDialog function to return false.
                DestroyWindow(hChatDlg);

                hChatDlg = 0;

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

                SendMessage(GetDlgItem(hDlg, IDC_RICHEDIT_CHAT), 
                            EM_EXSETSEL, 0, (LPARAM)&ch);

                SendMessage(GetDlgItem(hDlg, IDC_RICHEDIT_CHAT), 
                            EM_REPLACESEL, 0, (LPARAM)szText);

                SendMessage(GetDlgItem(hDlg, IDC_RICHEDIT_CHAT), 
                            WM_VSCROLL, SB_BOTTOM, (LPARAM)NULL);

                delete [] szText;

            g_HeapCS.Leave();
        }
        break;

        case UWM_CB_ADDSTRING_MSG:
        {
            extern CriticalSection g_HeapCS;

            g_HeapCS.Enter();

                char *szName = (char *)wParam;

                // NOTE: sort property is on, so the list will sort itself.
                int iIndex = (int)SendMessage(
                            GetDlgItem(hChatDlg, IDC_COMBO_NAMES), 
                            CB_ADDSTRING, 0, (LPARAM)szName);

                delete [] szName;

            g_HeapCS.Leave();

            int id = (int)lParam;

            // Set the ID# associated with a name in the combo box.
            // NOTE: ID# may NOT be the index of the name in the 
            //       combo box.
            SendMessage(GetDlgItem(hChatDlg, IDC_COMBO_NAMES), 
                CB_SETITEMDATA, iIndex, (LPARAM)id);

            // Set the current selection to the first item in the combo box.
            // Move to a seperate message?
            SendMessage(GetDlgItem(hChatDlg, IDC_COMBO_NAMES), 
                        CB_SETCURSEL, 0, 0);
        }
        break;

        case WM_CTLCOLORSTATIC:
        {
            HDC hDC = (HDC)wParam;
            SetBkMode(hDC, TRANSPARENT);
            return (LRESULT)ChatDlgBkBrush;
        }

        case WM_CTLCOLORDLG:
        {
            if (ChatDlgBkBrush)
            {
                DeleteObject(ChatDlgBkBrush);
            }
            ChatDlgBkBrush = CreateSolidBrush(RGB(0xE0, 0xE0, 0xE0));
            return (LRESULT)ChatDlgBkBrush;
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

                // Send button.
                case IDOK:
                {
                    OnSendBtnClick(hDlg);
                }
                break;

                // Clear button.
                case IDC_BTN_CLEAR:
                {
                    OnClearBtnClickClient(hDlg);
                }
                break;
            }
        }
        break; // WM_COMMAND

        case WM_DESTROY:
        {
            // Kill the client.
            // NOTE: This is the only place this function should get called.
            CL_ShutDown();

            DeleteObject(ChatDlgBkBrush);
            
            hChatDlg = 0;
        }
        break;
        
        default:
            return FALSE;
    }
    return TRUE;
}

// Initializes the dialog box controls
//
// In: hDlg     handle to the chat dialog box.
//     name     the name of the client.
void InitializeChatDlg(HWND hDlg, const char *name)
{
    // Handle to the app icon.
    HICON hIcon;

    // Used to set the color of the rich edit box.
    COLORREF color;

    // Used to set the character format of the rich edit box.
    CHARFORMAT2 charFormat;

    // Load and set the icon.
    hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    SendMessage(hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);

    // Set the image icon.
    SendMessage(GetDlgItem(hDlg, IDC_STATIC_PIC), STM_SETIMAGE, 
                IMAGE_ICON, (LPARAM)hIcon);

    // Let them see their name in the title bar.
    CString dlgTitle = "Chat - ";
    dlgTitle += name;
    SetWindowText(hDlg, dlgTitle);

    // Send to everyone is initially selected.
    SendMessage(GetDlgItem(hDlg, IDC_RADIO_EVERYONE), BM_SETCHECK, 
                BST_CHECKED, 0);

    // Set the focus to the send text box.
    SetFocus(GetDlgItem(hDlg, IDC_EDIT_SEND_TEXT));

    // Set the background color for the richedit box.
    color = RGB(0, 0, 255);
    SendMessage(GetDlgItem(hDlg, IDC_RICHEDIT_CHAT), 
        EM_SETBKGNDCOLOR, 0, color);

    // Use plain text formatting.
    SendMessage(GetDlgItem(hDlg, IDC_RICHEDIT_CHAT), 
        EM_SETTEXTMODE, TM_PLAINTEXT, 0);

    // Set the look of the text in the rich edit box.
    charFormat.cbSize = sizeof(CHARFORMAT2);
    charFormat.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE; //CFM_BOLD;
    charFormat.crTextColor = RGB(255, 252, 0);
    charFormat.dwEffects = CFE_BOLD;
    charFormat.bCharSet = ANSI_CHARSET;
    charFormat.yHeight = 225;
    //charFormat.bPitchAndFamily = FIXED_PITCH;
    strcpy(charFormat.szFaceName, "Arial");

    SendMessage(GetDlgItem(hDlg, IDC_RICHEDIT_CHAT),  
        EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&charFormat);

    // Set the max amount of text they cant type/send.
    // MAX_DATA - 50 to leave room for a the packet type and client name +
    // some slop.
    SendMessage(GetDlgItem(hDlg, IDC_EDIT_SEND_TEXT), EM_LIMITTEXT, 
                MAX_DATA - 50, 0);
}

// Sends a chat message to the server.
//
// In:  hDlg    Handle to chat the dialog box.
void OnSendBtnClick(HWND hDlg)
{
    char szTextBuffer[MAX_DATA];
    int iLen;

    // ID of -1 means send to all.
    int iToID = -1;

    // Get the text they want to send.
    iLen = GetDlgItemText(hDlg, IDC_EDIT_SEND_TEXT, szTextBuffer, 
                            MAX_DATA);
    if (iLen)
    {
        // Check for private message or send to all based on the radio button.
        if (SendMessage(GetDlgItem(hDlg, IDC_RADIO_EVERYONE), BM_GETCHECK, 
            0, 0) != BST_CHECKED)
        {
            // Get the ID # of the currently selected buddy.
            iToID = CL_GetCbBuddyData();
        }

        // Send the message to the server.
        CL_SendChatMessage(szTextBuffer, iToID);

        // Clear and set the focus to the send text box.
        SetDlgItemText(hDlg, IDC_EDIT_SEND_TEXT, "");
        SetFocus(GetDlgItem(hDlg, IDC_EDIT_SEND_TEXT));
    }
}

// Clear the text in the client's rich edit box.
//
// In:  hDlg    Handle to the dialog box.
void OnClearBtnClickClient(HWND hDlg)
{
    SetDlgItemText(hDlg, IDC_RICHEDIT_CHAT , "");
    SetFocus(GetDlgItem(hDlg, IDC_EDIT_SEND_TEXT));
}