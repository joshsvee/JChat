// dlgSessionProc.cpp : Handles main dialog session on app start.

#include "sys_win.h"
#include "net_common.h"
#include "resource.h"
#include "commctrl.h"
#include "dlgSession.h"
#include "dlgServerConsole.h"
#include "dlgChat.h"
#include "server.h"
#include "cl_client.h"

/***********/
/* GLOBALS */
/***********/

// App instance handle.
extern HINSTANCE hInst;

// Handle to the session dialog box.
extern HWND hSessionDlg;

// Brush for the dlg background color.
HBRUSH SessionDlgBkBrush = 0;
HBRUSH SessionTextBoxBkBrush = 0;

// Passed to the chat or server dialog proc.
dlgparam_t dlgParam;

// Callback function for session dialog box
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
LRESULT CALLBACK DlgSessionProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_INITDIALOG:

            hSessionDlg = hDlg;

            // Init the dialog controls.
            InitializeSessionDlg(hDlg);

            // FALSE since we set the focus to a control and not the 
            // dlgbox.
            return FALSE;

        break;

        // Text Box control color.
        case WM_CTLCOLOREDIT:
        {
            //HDC hDC = (HDC)wParam;
            //SetBkMode(hDC, TRANSPARENT);
            //SetTextColor(hDC, RGB(255, 0, 255));

            //SessionTextBoxBkBrush = CreateSolidBrush(RGB(0xFF, 0xCC, 0));
            //return (LRESULT)SessionTextBoxBkBrush;
            return FALSE;
        }

        case WM_CTLCOLORSTATIC:
        {
            HDC hDC = (HDC)wParam;
            //SetTextColor(hDC, RGB(255, 255, 255));
            SetBkMode(hDC, TRANSPARENT);
            return (LRESULT)SessionDlgBkBrush;
        }

        // Button controls color.
        case WM_CTLCOLORBTN:
        {
            HDC hDc = (HDC)wParam;
            SetBkMode(hDc, TRANSPARENT);
            return (LRESULT)SessionDlgBkBrush;
        }
        break;

        break;

        // Dialog box color.
        case WM_CTLCOLORDLG:
        {
            HDC hDc = (HDC)wParam;
            
            if (SessionDlgBkBrush)
            {
                DeleteObject(SessionDlgBkBrush);
            }
            SetTextColor(hDc, RGB(255, 0, 0));
            // RGB(119, 150, 224)
            SessionDlgBkBrush = CreateSolidBrush(RGB(0xE0, 0xE0, 0xE0));

            // Cast to INT_PTR as per MSDN.
            return (INT_PTR)SessionDlgBkBrush;
        }
        break;

        case WM_COMMAND:
        {    
            // Find which control was graced.
            switch (LOWORD(wParam))
            {
                // Exit button.
                case IDCANCEL:
                {
                    DestroyWindow(hDlg);
                }
                break;

                // Create server radio button.
                case IDC_RADIO_SERVER:        // Intentional fall through.

                // Join server radio button.
                case IDC_RADIO_JOIN:
                {
                    UpDateUi(hDlg);
                }
                break;

                // Create button.
                case IDC_BTN_CREATE_SERVER:
                {
                    if (OnCreateBtnClick(hDlg))
                    {
                        // Display the server console dialog box 
                        // (modeless).
                        CreateServerDialog(&dlgParam);

                        return TRUE;

                    }           
                }
                break;

                // Connect button.
                case IDC_BTN_CONNECT:
                {
                    if (OnConnectBtnClick(hDlg))
                    {
                        // Create the client chat dialog box (modeless).
                        CreateChatDialog(&dlgParam);
 
                        return TRUE;
                    }
                }
                break;

            } // LOWWORD(wParam)
        }
        break; // WM_COMMAND

        case WM_DESTROY:
        {
            DeleteObject(SessionDlgBkBrush);
            DeleteObject(SessionTextBoxBkBrush);

            SessionDlgBkBrush = 0;
            SessionTextBoxBkBrush = 0;

            // Done with winsock.
            NET_ShutDown();

            PostQuitMessage(0);
        }
        break; // WM_DESTROY

        default:
            // Message not handled.
            return FALSE;

    } // msg

    return TRUE;
}

// Initializes the dialog box controls
//
// In: hDlg         handle to the session dialog box.
void InitializeSessionDlg(HWND hDlg)
{
    HICON hIcon;
    INITCOMMONCONTROLSEX commonCtrls;

    commonCtrls.dwICC = ICC_INTERNET_CLASSES | 
                        ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    commonCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCommonControlsEx(&commonCtrls);

    // Load and set the icon.
    hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    SendMessage(hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);

    // Show a status message.
    char szStatus[MAX_STATUS_MSG];

    // Init the network system.
    if (!NET_Init())
    {        
        sprintf(szStatus, "ERROR: Winsock : %s : %d", NET_GetErrorString(), NET_GetErrorNumber()); 
        SetWindowText(GetDlgItem(hDlg, IDC_STATIC_STATUS), szStatus);
    }
    else
    {
        LoadString(hInst, IDS_WINSOCK_INIT_OK, szStatus, MAX_STATUS_MSG);
        SetWindowText(GetDlgItem(hDlg, IDC_STATIC_STATUS), szStatus);
    }

    // Limit how many bytes to use for their name and server they create.
    SendMessage(GetDlgItem(hDlg, IDC_EDIT_NAME), 
                EM_LIMITTEXT, NAME_LEN, 0);
    SendMessage(GetDlgItem(hDlg, IDC_EDIT_SERVER), 
                EM_LIMITTEXT, NAME_LEN, 0);

    // Server radio button selected by default.
    SendMessage(GetDlgItem(hDlg, IDC_RADIO_SERVER), BM_SETCHECK, 
                BST_CHECKED, 0);

#ifdef _DEBUG

    SetDlgItemText(hDlg, IDC_EDIT_SERVER, "Chat Server");
    SetDlgItemText(hDlg, IDC_EDIT_SERVER_PORT, "27000");

    SetDlgItemText(hDlg, IDC_EDIT_NAME, "UnnamedPlayer");
    DWORD dwIpAddress = MAKEIPADDRESS(127, 0, 0, 1);
    SendMessage(GetDlgItem(hDlg, IDC_JOIN_IPADDRESS_SERVER), 
                IPM_SETADDRESS, 0, (LPARAM)dwIpAddress);
    SetDlgItemText(hDlg, IDC_EDIT_JOIN_SERVER_PORT, "27000");

#endif

    // Up date the user interface.
    UpDateUi(hDlg);
}

// Create server button clicked.
//
// In: hDlg handle to the session dialog box.
//
// Return: true if successful else return false.
bool OnCreateBtnClick(HWND hDlg)
{	
    // Error message.
    char szErrMsg[MAX_STATUS_MSG];
    char szCaption[MAX_STATUS_MSG];

    // Server name they create.
    char szServerName[NAME_LEN];

    // Length of the server's name.
    int iLen;

    // Server port.
    unsigned short port;

    // True if port # was translated from text.
    BOOL bTrans = false;

    // Get the server name.
    iLen = GetDlgItemText(hDlg, IDC_EDIT_SERVER, szServerName, NAME_LEN);

    if (!iLen)
    {
        strncpy(szServerName, "NONAME", 6);
        szServerName[6] = 0;
    }

    // Get the server port #.
    port = GetDlgItemInt(hDlg, IDC_EDIT_SERVER_PORT, &bTrans, false);

    if (!bTrans || port < 1024 || port > 65535)
    {
        // Could not translate the port from text or port out of range.
        LoadString(hInst, IDS_PORT_ERR, szErrMsg, MAX_STATUS_MSG);
        LoadString(hInst, IDS_CAPTION_SESSION_MGR, szCaption, 
                    MAX_STATUS_MSG);
        MessageBox(hDlg, szErrMsg, szCaption, MB_OK | MB_ICONINFORMATION); 
        SetFocus(GetDlgItem(hDlg, IDC_EDIT_SERVER_PORT));
        SendMessage(GetDlgItem(hDlg, IDC_EDIT_SERVER_PORT), 
                    EM_SETSEL, 0, -1);
        return false;
    }

    // Setup the server dialog param.
    dlgParam.port = port;
    strncpy(dlgParam.szName, szServerName, NAME_LEN - 1);
    dlgParam.szName[NAME_LEN - 1] = 0;
    
    return true;
}

// Connect button clicked.
//
// In: hDlg handle to the session dialog box.
//
// Return: true if successful else return false.
bool OnConnectBtnClick(HWND hDlg)
{
    // Error message.
    char szErrMsg[MAX_STATUS_MSG];
    char szCaption[MAX_STATUS_MSG];
    
    // Name.
    char szPlayerName[NAME_LEN];
    
    // Len of the player's name.
    int iNameLen;
    
    // The total number of Octets filled in the IP address control 
    // (1 -> 4).
    LRESULT lResult;

    // IP address.
    DWORD dwIPAddr;
    BYTE firstOctet;
    BYTE secondOctet;
    BYTE thirdOctet;
    BYTE fourthOctet;

    // Port.
    unsigned short port;

    // True if port # was translated from text.
    BOOL bTrans = false;

    // Get the player name they want to use.
    iNameLen = GetDlgItemText(hDlg, IDC_EDIT_NAME, szPlayerName, NAME_LEN);

    if (!iNameLen)
    {
        LoadString(hInst, IDS_NAME_ERR, szErrMsg, MAX_STATUS_MSG);
        LoadString(hInst, IDS_CAPTION_SESSION_MGR, szCaption, 
                    MAX_STATUS_MSG);
        MessageBox(hDlg, szErrMsg, szCaption, MB_OK | MB_ICONINFORMATION);
        SetFocus(GetDlgItem(hDlg, IDC_EDIT_NAME));
        return false;
    }
    
    // Get the ip address they wish to join.
    lResult = SendMessage(GetDlgItem(hDlg, IDC_JOIN_IPADDRESS_SERVER), 
                IPM_GETADDRESS, 0, (LPARAM)&dwIPAddr);
 
    if (lResult < 4)
    {
        LoadString(hInst, IDS_JOIN_ERR, szErrMsg, MAX_STATUS_MSG);
        LoadString(hInst, IDS_CAPTION_SESSION_MGR, szCaption, 
                    MAX_STATUS_MSG);
        MessageBox(hDlg, szErrMsg, szCaption, MB_OK | MB_ICONINFORMATION);
        SetFocus(GetDlgItem(hDlg, IDC_JOIN_IPADDRESS_SERVER));
        return false;         
    }

    // Each byte of the ip address.    
    firstOctet = (BYTE)FIRST_IPADDRESS(dwIPAddr);   // 192
    secondOctet = (BYTE)SECOND_IPADDRESS(dwIPAddr); // 168
    thirdOctet = (BYTE)THIRD_IPADDRESS(dwIPAddr);   // 1
    fourthOctet = (BYTE)FOURTH_IPADDRESS(dwIPAddr); // 1

    // Get the server port #.
    port = GetDlgItemInt(hDlg, IDC_EDIT_JOIN_SERVER_PORT, &bTrans, false);

    if (!bTrans || port < 1024 || port > 65535)
    {
        // Could not translate the port from text or port out of range.
        LoadString(hInst, IDS_PORT_ERR, szErrMsg, MAX_STATUS_MSG);
        LoadString(hInst, IDS_CAPTION_SESSION_MGR, szCaption, 
                    MAX_STATUS_MSG);
        MessageBox(hDlg, szErrMsg, szCaption, MB_OK | MB_ICONINFORMATION);
        SetFocus(GetDlgItem(hDlg, IDC_EDIT_JOIN_SERVER_PORT));
        SendMessage(GetDlgItem(hDlg, IDC_EDIT_JOIN_SERVER_PORT), 
                    EM_SETSEL, 0, -1);
        return false;
    }

    // Convert the IP to dotted string format.
    char szIpBuffer[NAME_LEN];
    sprintf(szIpBuffer, "%d.%d.%d.%d", firstOctet, secondOctet, 
            thirdOctet, fourthOctet);

    // Setup the chat dialog param.
    sprintf(dlgParam.szIp, "%d.%d.%d.%d", firstOctet, secondOctet, 
            thirdOctet, fourthOctet);
    dlgParam.port = port;
    strncpy(dlgParam.szName, szPlayerName, NAME_LEN - 1);
    dlgParam.szName[NAME_LEN - 1] = 0;

    return true;
}

// Update the UI when the radio button is clicked.
//
// In: hDlg handle to the session dialog box.
void UpDateUi(HWND hDlg)
{
    int iResult;

    iResult = (int)SendMessage(GetDlgItem(hDlg, IDC_RADIO_SERVER), 
                            BM_GETCHECK, 0, 0);

    if (BST_CHECKED == iResult)
    {
        // Enable all server creation.
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_CREATE_SERVER), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_SERVER), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_SERVER_PORT), TRUE);

        // Disable join server.
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_CONNECT), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_NAME), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_JOIN_IPADDRESS_SERVER), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOIN_SERVER_PORT), FALSE);

        // Reset the current default push button to a regular button.
        SendDlgItemMessage(hDlg, IDC_BTN_CONNECT,
                BM_SETSTYLE, BS_PUSHBUTTON, (LPARAM)TRUE);

        // Update the default push button's control ID.
        SendMessage(hDlg, DM_SETDEFID, IDC_BTN_CREATE_SERVER, 0L);

        // Set the new style.
        SendDlgItemMessage(hDlg, IDC_BTN_CREATE_SERVER,
                BM_SETSTYLE, BS_DEFPUSHBUTTON, (LPARAM)TRUE);

        //SetFocus(GetDlgItem(hDlg, IDC_EDIT_SERVER));
        SetFocus(GetDlgItem(hDlg, IDC_RADIO_SERVER));
    }
    else
    {
        // Disable all server creation.
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_CREATE_SERVER), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_SERVER), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_SERVER_PORT), FALSE);

        // Enable join server.
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_CONNECT), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_NAME), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_JOIN_IPADDRESS_SERVER), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_JOIN_SERVER_PORT), TRUE);

        // Reset the current default push button to a regular button.
        SendDlgItemMessage(hDlg, IDC_BTN_CREATE_SERVER,
                BM_SETSTYLE, BS_PUSHBUTTON, (LPARAM)TRUE);

        // Update the default push button's control ID.
        SendMessage(hDlg, DM_SETDEFID, IDC_BTN_CONNECT, 0L);

        // Set the new style.
        SendDlgItemMessage(hDlg, IDC_BTN_CONNECT,
                BM_SETSTYLE, BS_DEFPUSHBUTTON, (LPARAM)TRUE);

        SetFocus(GetDlgItem(hDlg, IDC_EDIT_NAME));
    }
}