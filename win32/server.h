// server.h : Chat server.

#ifndef _SERVER_H_
#define _SERVER_H_

#include <atlstr.h>
#include "net_common.h"
#include "net_msg.h"

// Initialize the server.
// Creates and binds the server socket and begins to listen for connections.
//
// In:  szServerName    Server name.
//      port            Server listen port.
//
// Return: true if ok false otherwise.
// NOTE: THIS FUNCTION IS CALLED FOR YOU WHEN THE SERVER DIALOG IS INITIALIZED.
//		 NEVER CALL THIS FUNCTION!
bool SV_Init(const char *szServerName, unsigned short port);

// Initialize all the clients.
void SV_InitClients(void);

// Receives new connections and client data.
//
// In/Out: pReserved    Shared thread data (not used).
//
// Returns: 0 for success.
// NOTE: THIS FUNCTION RUNS ON A SEPERATE THREAD.
//	     NEVER CALL THIS FUNCTION!
unsigned int __stdcall SV_Main(void *pReserved);

// Remove a client from the server.
//
// In:  cl  The client to drop.
//
// Out: cl  The disconnected client.
void SV_DropClient(client_t *cl);

// Receives a data stream from connected clients.
//
// In:  cl  The current client sending data.
//
// Return: true if the data was received.
// NOTE: If this function returns false, the client should be dropped.
bool SV_ReceiveClientData(client_t *cl);

// Send a message to all connected clients.
//
// In:  msg     The message to send.
//      len     The total number of bytes to send.
//
// NOTE: If the message can not be sent to a client, that client is dropped.
void SV_SendToAll(const char *msg, int len);

// Free all server resources.
// NOTE: THIS FUNCTION IS CALLED AUTOMATICALLY FROM THE GUI THREAD
//		 WHEN THE USER CLICKS THE EXIT BUTTON OR CLOSES THE WINDOW.
//		 NEVER CALL THIS FUNCTION!
void SV_ShutDown(void);

// Displays text in the server's Rich Edit box.
//
// In:	str     The string to display.
void SV_DisplayText(CString str);

#endif