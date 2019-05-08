// net_common.h : Common functions for the network system.

#ifndef _NET_COMMON_H_
#define _NET_COMMON_H_

#include "sys_win.h"

// Max number of bytes we send/receive.
#define MAX_DATA 512

#define MAX_STATUS_MSG 256

// Max number of handled clients.
#define MAX_CLIENTS 5

// Client states.
typedef enum 
{
    cl_disconnected,// Client has disconnected.
    cl_connected    // Client connected.

} clientstate_t;

// Client.
struct client_t
{
    // Client IPv4 address.
    sockaddr_in addr;

    // Client connection end point.
    SOCKET clSocket;

    // Client state.
    clientstate_t clstate;

    // Client id number.
    int id;

    // Client name.
    char name[NAME_LEN];
};

//////////////////////////////////////////////////////////////////////////
// Initialize the network system.
//
// Parameters
// None.
//
// Returns
// Bool     TRUE if OK.
//////////////////////////////////////////////////////////////////////////
bool NET_Init(void);

//////////////////////////////////////////////////////////////////////////
// Free all network resources.
//
// Parameters
// None.
//
// Returns
// None.
//////////////////////////////////////////////////////////////////////////
void NET_ShutDown(void);

//////////////////////////////////////////////////////////////////////////
// Gets the last error that occurred.
//
// Parameters
// None.
//
// Returns
// Int32    The error number.
//////////////////////////////////////////////////////////////////////////
int NET_GetErrorNumber(void);

//////////////////////////////////////////////////////////////////////////
// Gets the last error that occurred in string format.
//
// Parameters
// None.
//
// Returns
// Char *   The error in string format.
//////////////////////////////////////////////////////////////////////////
const char *NET_GetErrorString(void);

#endif