// cl_client.h : Handles all local client info.

#ifndef _CL_CLIENT_H_
#define _CL_CLIENT_H_

#include <atlstr.h>
#include "net_common.h"
#include "net_msg.h"

// Initialize the client and connect to the server.
//
// In:  szServerIp      IP address of the server in dotted notation (127.0.0.1).
//      port            The server's port.
//      szPlayerName    Client's name.
//
// Return: true if successful.
// NOTE: THIS FUNCTION IS CALLED FOR YOU WHEN THE CLIENT CHAT DIALOG IS INITIALIZED.
//		 NEVER CALL THIS FUNCTION!
bool CL_Init(const char *szServerIp, unsigned short port, const char *szPlayerName);

// Keep reading from the server stream until disconnect.
//
// In/Out:  pData  The local client (share thread data).
// NOTE: THIS FUNCTION RUNS ON A SEPERATE THREAD.
//		 NEVER CALL THIS FUNCTION!
unsigned int __stdcall CL_ReceiveServerData(void *pData);

// Send chat text to the server.
//
// In:  szChatText  The text.
//      idTo        ID # of the person we send to or -1 for send to all.
// NOTE: THIS FUNCTION IS AUTOMATICALLY CALLED WHEN THE USER CLICKS THE SEND BUTTON.
//		 NEVER CALL THIS FUNCTION!
void CL_SendChatMessage(const char *szChatText, int idTo);

// Frees all client resources.
// NOTE: THIS FUNCTION IS CALLED AUTOMATICALLY FROM THE GUI THREAD
//		 WHEN THE USER CLICKS THE EXIT BUTTON OR CLOSES THE WINDOW.
//		 NEVER CALL THIS FUNCTION!
void CL_ShutDown(void);

// Set the current selection to the first item in the combo box.
void CL_SetCbToFirstItem(void);

// Adds data to the combo box.
//
// In:  szName  The name of a buddy.
//      id      The id number of the buddy.
void CL_AddCbBuddyData(const char *szName, int id);

// Get the data (ID) associated with a buddy's name in the combo box.
//
// Return: The ID # of the currently selected buddy in the combo box.
//         or -1 for error.
int CL_GetCbBuddyData(void);

// Remove a name from the combo box.
//
// In:	name The name of the buddy to remove.
//
// Return: true if successful.
bool CL_RemoveCbBuddyData(const char *name);

// Displays text in the clients Rich Edit box.
//
// In:	str     The string to display.
void CL_DisplayText(CString str);

#endif
