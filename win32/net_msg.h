// net_msg.h : Network message system.

#ifndef _NET_MSG_H_
#define _NET_MSG_H_

// =========================================
// Message types.
// =========================================

// sv_* =   From server to client.
// cl_* =   From client to server.
// sv_cl_*  To/From client and server.

enum
{
    // From server to client.
    sv_cnt = 1, // Connect message.    
    sv_list,    // Return a list of clients.
	sv_add,     // Add a single client to the combo box / buddy list.
    sv_full,    // Server is full.
    sv_remove,  // Someone disconnected so remove them from the combo box / buddy list.

    // To / From both.
    sv_cl_msg,  // Chat msg

    // From client to server.
    cl_reg,     // Client registers a name.
    cl_get,     // Get the buddy list from the server.
};

// =========================================

#endif