// sv_main.cpp : Server using select.

#include <cstring>
#include <stdio.h>
#include "server.h"
#include "resource.h"
#include "richedit.h"
#include "dlgServerConsole.h"
#include "cl_client.h"
#include "CriticalSection.h"
#include "NetBuffer.h"

/***********/
/* GLOBALS */
/***********/

// Used to protect shared data on the client and server.
// We can use the same object for both since only one instance
// is ever running at any given time.
extern CriticalSection g_cs;

// Handle to the server dialog box.
extern HWND hServerDlg;

// Server.
struct server_t
{
    // Server connection end point.
    SOCKET svSocket;

	// The current number of connected clients.
	int numConnected;

    // Server name.
    char servername[NAME_LEN];

    // Master socket list for select().
    fd_set masterset;

    // Socket list to check for readability.
    fd_set readset;
};

// Shared thread data.

// Server.
server_t sv;

// Set to true to terminate the server loop.
volatile bool bTerminateServerThread = false;

// All our little players (this could be dynamic blah...).
client_t clients[MAX_CLIENTS];

// Server thread ID.
DWORD  dwServerThreadID = 0;

// Server thread handle.
HANDLE svServerThreadHandle = 0;

//This structure will be used to store partial
//client packets.
struct tClientRecvBuffer
{
	int nClientid;
	unsigned int uTotal;
	unsigned char ucBuffer[MAX_DATA];
};

//There must be one partial message buffer for each client.
tClientRecvBuffer clientBuffers[MAX_CLIENTS];

/**********/
/* PUBLIC */
/**********/

// Initialize the server.
// Creates and binds the server socket and begins to listen for connections.
//
// In:  szServerName    Server name.
//      port            Server listen port.
//
// Return: true if ok false otherwise.
// NOTE: THIS FUNCTION IS CALLED FOR YOU WHEN THE SERVER DIALOG IS INITIALIZED.
//		 NEVER CALL THIS FUNCTION!
bool SV_Init(const char *szServerName, unsigned short port)
{
	sockaddr_in svAddress;

	//Initialize the server.
	sv.numConnected = 0;
	memset(sv.servername, 0, NAME_LEN);
	strcpy(sv.servername, szServerName);
	FD_ZERO(&sv.masterset);

	//Initialize the client list.
	SV_InitClients();
	
	//Create the server socket.
	sv.svSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sv.svSocket == INVALID_SOCKET)
	{
		SV_DisplayText("Failed to create socket.\n");
		return false;
	}

	//Setup the server address.
	memset(&svAddress, 0, sizeof(sockaddr_in));
	svAddress.sin_family = AF_INET;
	svAddress.sin_addr.s_addr = INADDR_ANY;
	svAddress.sin_port = htons(port);

	// Name the socket.
	if(bind(sv.svSocket, (sockaddr *)&svAddress, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		SV_DisplayText("Failed to bind socket.\n");
		return false;
	}

	//Listen for connections.
	//Try to listen on the specified port.
	if(listen(sv.svSocket, MAX_CLIENTS) == SOCKET_ERROR)
	{
		SV_DisplayText("Failed to listen on socket.\n");
		return false;
	}

	//Add the server socket to the set.
	FD_SET(sv.svSocket, &sv.masterset);

	//Create a thread to run the server.
	//use _beginthreadex.
	svServerThreadHandle = (HANDLE)_beginthreadex(NULL, 0, SV_Main, (void *)0, 0, (unsigned int *)&dwServerThreadID);
	if(svServerThreadHandle == 0)
	{
		SV_DisplayText("Failed to create server thread.\n");
		closesocket(sv.svSocket);
		return false;
	}

    return true;
}

// Initialize all the clients.
void SV_InitClients(void)
{
    //Set up the array of clients.
	int i;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		memset(&clients[i].addr, 0, sizeof(sockaddr_in));
		clients[i].clSocket = INVALID_SOCKET;
		clients[i].clstate = cl_disconnected;
		memset(clients[i].name, 0, NAME_LEN);
		strcpy(clients[i].name, "Anonymous");
		clients[i].id = i + 1;

		clientBuffers[i].nClientid = clients[i].id;
		clientBuffers[i].uTotal = 0;
		memset(clientBuffers[i].ucBuffer, 0, MAX_DATA);
	}
}

// Receives new connections and client data.
//
// In/Out: pReserved    Shared thread data (not used).
//
// Returns: 0 for success.
// NOTE: THIS FUNCTION RUNS ON A SEPERATE THREAD.
//	     NEVER CALL THIS FUNCTION!
unsigned int __stdcall SV_Main(void *pReserved)
{
    UNREFERENCED_PARAMETER(pReserved);

    //Accept new client connections and assign each client an ID #.
    //Server should relay messages to the correct clients based on client ID #.
    //Use the select function to multiplex client sockets to 
    //determine when a client is ready for reading.
    //Receive client data.
    //Detect client disconnect.
    //Detect MAX clients.
    //Use a Critical Section to protect any shared data.

    // NOTE: Some of the above functionality may be handled in other server functions.

	sockaddr_in clAddress;
	int nReturn;

	//Needed for use with non-blocking select.
	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	int i;
    while(1)
    {        
        if(bTerminateServerThread)
            break;

		sv.readset = sv.masterset;

		nReturn = select(0, &sv.readset, NULL, NULL, &tv);

		if(nReturn == 0)
			continue;

		if(nReturn == SOCKET_ERROR)
		{
			SV_DisplayText("Socket Error: Select\n");
			continue;
		}
		else
		{
			if(FD_ISSET(sv.svSocket, &sv.readset))
			{
				//Server socket ready for reading.
				//We got a new connection.
				SOCKET clNewSocket;

				int iClientAddrLen = sizeof(sockaddr_in);
				clNewSocket = accept(sv.svSocket, (sockaddr *)&clAddress, &iClientAddrLen);
				if(clNewSocket != INVALID_SOCKET)
				{
					bool bSlotFound = false;
					for(i = 0; i < MAX_CLIENTS; i++)
					{
						if(clients[i].clstate == cl_disconnected)
						{
							clients[i].clSocket = clNewSocket;
							clients[i].clstate = cl_connected;
							memcpy(&clients[i].addr, &clAddress, sizeof(sockaddr_in));

							FD_SET(clients[i].clSocket, &sv.masterset);
							
							NetBuffer cntMsg;
							short len = 0;
							cntMsg.WriteShort(len);
							cntMsg.WriteByte(sv_cnt);
							cntMsg.WriteChar(clients[i].id);

							len = cntMsg.GetPosition();
							memcpy(cntMsg.GetBuffer(), &len, sizeof(short));

							clientBuffers[i].uTotal = 0;
							memset(clientBuffers[i].ucBuffer, 0, MAX_DATA);

							char szBuffer[128];
							memset(szBuffer, 0, 128);
							sprintf(szBuffer, "Client: %s:%d> Connected, assigned ID: %d\n", inet_ntoa(clients[i].addr.sin_addr),
								(int)ntohs(clients[i].addr.sin_port), (int)clients[i].id);
							SV_DisplayText(szBuffer);

							send(clients[i].clSocket, (const char *)cntMsg.GetBuffer(), len, 0);

							memset(szBuffer, 0, 128);
							sprintf(szBuffer, "Server> Sent connect message to Client:%d.\n", (int)clients[i].id);

							bSlotFound = true;
							break;
						}
					}

					if(!bSlotFound)
					{
						//All client slots are currently in use.
						NetBuffer fullMsg;
						short len = sizeof(short) + sizeof(byte);
						fullMsg.WriteShort(len);
						fullMsg.WriteByte(sv_full);

						send(clNewSocket, (const char *)fullMsg.GetBuffer(), len, 0);
					
						closesocket(clNewSocket);
					}
				}
			}

			//Check all clients for data.
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(clients[i].clstate == cl_disconnected)
					continue;

				if(FD_ISSET(clients[i].clSocket, &sv.readset))	
				{	
					SV_ReceiveClientData(&clients[i]);
				}
			}
		}
    }
    
    return 0; // _endthreadex(0)
}

// Remove a client from the server.
//
// In:  cl  The client to drop.
//
// Out: cl  The disconnected client.
void SV_DropClient(client_t *cl)
{
	//Build remove message.
	NetBuffer discon;
	short len = 0;
	discon.WriteShort(len);
	discon.WriteByte(sv_remove);
	discon.WriteByte(cl->id);
	len = discon.GetPosition();
	memcpy(discon.GetBuffer(), &len, sizeof(short));

	//Remove a client from the server.
	FD_CLR(cl->clSocket, &sv.masterset);
	closesocket(cl->clSocket);
	cl->clstate = cl_disconnected;

	//Notify the remaining clients who disconnected
	//so they can remove them from their buddy list.
	int i;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(clients[i].clstate == cl_disconnected)
			continue;

		send(clients[i].clSocket, (const char *)discon.GetBuffer(), len, 0);
	}
}

// Receives a data stream from connected clients.
//
// In:  cl  The current client sending data.
//
// Return: true if the data was received.
// NOTE: If this function returns false, the client should be dropped.
bool SV_ReceiveClientData(client_t *cl)
{    
    //Get client message data.
	int i, nRecvd;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(clientBuffers[i].nClientid == cl->id)
			break;
	}

	unsigned char ucBuffer[MAX_DATA];
	nRecvd = recv(cl->clSocket, (char *)ucBuffer, MAX_DATA, 0);

	//Check for client disconnects.
	//1) Graceful
	//2) Abnormal
	if(nRecvd == SOCKET_ERROR)
	{
		int nError = NET_GetErrorNumber();

		if(nError == WSAECONNRESET)
		{
			//TODO Add message
			//Client forced disconnect.
			SV_DropClient(cl);
			return false;
		}
	}

	if(nRecvd == 0)
	{
		//Client died.
		SV_DropClient(cl);
		return false;
	}

	int nLeftOver = 0;
	int nReadBytes = 0;
	if((clientBuffers[i].uTotal + nRecvd) > MAX_DATA)
	{
		nLeftOver = (clientBuffers[i].uTotal + nRecvd) - MAX_DATA;
	}

	if(nLeftOver > 0)
	{
		nReadBytes = nRecvd - nLeftOver;
		memcpy(clientBuffers[i].ucBuffer + clientBuffers[i].uTotal, ucBuffer, nReadBytes);
		nRecvd = nLeftOver;
	}
	else
	{
		memcpy(clientBuffers[i].ucBuffer + clientBuffers[i].uTotal, ucBuffer, nRecvd);
		nReadBytes = nRecvd;
		nRecvd = 0;
	}

	clientBuffers[i].uTotal += nReadBytes;

	if(nRecvd > 0)
	{
		unsigned char cpyBuffer[MAX_DATA];
		memset(cpyBuffer, 0, MAX_DATA);
		memcpy(cpyBuffer, ucBuffer + nReadBytes, MAX_DATA - nReadBytes);
		memcpy(ucBuffer, cpyBuffer, MAX_DATA);
	}

	while(1)
	{
		short MsgLen = 0;

		memcpy(&MsgLen, clientBuffers[i].ucBuffer, sizeof(short));

		if(MsgLen < 0)
			break;

		if(MsgLen == 0)
			break;

		if(MsgLen > (short)clientBuffers[i].uTotal)
		{
			if(nRecvd > 0)
			{
				if((clientBuffers[i].uTotal + nRecvd) > MAX_DATA)
				{
					nLeftOver = 0;
					nReadBytes = 0;
					if((clientBuffers[i].uTotal + nRecvd) > MAX_DATA)
					{
						nLeftOver = (clientBuffers[i].uTotal + nRecvd) - MAX_DATA;
					}

					if(nLeftOver > 0)
					{
						nReadBytes = nRecvd - nLeftOver;
						memcpy(clientBuffers[i].ucBuffer + clientBuffers[i].uTotal, ucBuffer, nReadBytes);
						nRecvd = nLeftOver;
					}
					else
					{
						memcpy(clientBuffers[i].ucBuffer + clientBuffers[i].uTotal, ucBuffer, nRecvd);
						nReadBytes = nRecvd;
						nRecvd = 0;
					}

					clientBuffers[i].uTotal += nReadBytes;

					if(nRecvd > 0)
					{
						unsigned char cpyBuffer[MAX_DATA];
						memset(cpyBuffer, 0, MAX_DATA);
						memcpy(cpyBuffer, ucBuffer + nReadBytes, MAX_DATA - nReadBytes);
						memcpy(ucBuffer, cpyBuffer, MAX_DATA);
					}
				}
				else
				{
					memcpy(clientBuffers[i].ucBuffer + clientBuffers[i].uTotal, ucBuffer, nRecvd);
					clientBuffers[i].uTotal += (unsigned int)nRecvd;
					nRecvd = 0;
				}
			}
			else
			{
				break;
			}
		}

		//Copy the complete message into a NetBuffer class.
		NetBuffer clMsg;
		memcpy(clMsg.GetBuffer(), clientBuffers[i].ucBuffer, MsgLen);

		//Remove the message from the global client buffer
		//TODO find a better way to do this.
		unsigned char TempBuffer[MAX_DATA];
		memset(TempBuffer, 0, MAX_DATA);
		memcpy(TempBuffer, clientBuffers[i].ucBuffer + MsgLen, MAX_DATA - MsgLen);
		memcpy(clientBuffers[i].ucBuffer, TempBuffer, MAX_DATA);
		clientBuffers[i].uTotal -= MsgLen;

		//Parse the message from the client.
		unsigned char ucOpCode;
		clMsg.SetPosition(sizeof(short));
		ucOpCode = clMsg.ReadByte();

		switch(ucOpCode)
		{
			case cl_reg:
				{
					memset(cl->name, 0, NAME_LEN);
					memcpy(cl->name, clMsg.GetBuffer() + clMsg.GetPosition(), MsgLen - clMsg.GetPosition());
					
					char szBuffer[128];
					memset(szBuffer, 0, 128);
					sprintf(szBuffer, "Client: %d> Registered name: %s\n", (int)cl->id, cl->name);
					SV_DisplayText(szBuffer);

					NetBuffer addMsg;
					short len = 0;
					addMsg.WriteShort(len);
					addMsg.WriteByte(sv_add);
					addMsg.WriteChar(cl->id);
					addMsg.WriteString(cl->name);
					addMsg.WriteChar(0);

					len = addMsg.GetPosition();
					memcpy(addMsg.GetBuffer(), &len, sizeof(short));

					int j;
					for(j = 0; j < MAX_CLIENTS; j++)
					{
						if(clients[j].clstate == cl_connected && clients[j].id != cl->id)
						{
							send(clients[j].clSocket, (const char *)addMsg.GetBuffer(), len, 0);
						}
					}

					continue;
				}
				break;

			case cl_get:
				{
					NetBuffer listMsg;
					short len = 0;
					listMsg.WriteShort(len);
					listMsg.WriteByte(sv_list);
					byte ucTotal = 0;
					listMsg.WriteByte(ucTotal);

					int j;
					for(j = 0; j < MAX_CLIENTS; j++)
					{
						if(clients[j].clstate == cl_connected)
						{
							listMsg.WriteByte(clients[j].id);
							listMsg.WriteString(clients[j].name);
							listMsg.WriteChar(0);

							ucTotal = ucTotal + 1;
						}
					}

					len = listMsg.GetPosition();

					memcpy(listMsg.GetBuffer(), &len, sizeof(short));
					memcpy(listMsg.GetBuffer() + sizeof(short) + sizeof(byte), &ucTotal, sizeof(byte));

					send(cl->clSocket, (const char *)listMsg.GetBuffer(), len, 0);
				}
				break;

			case sv_cl_msg:
				{
					clMsg.SetPosition(0);
					short len = clMsg.ReadShort();
					clMsg.ReadByte(); //Read Opcode
					char cToid = clMsg.ReadChar();

					NetBuffer chatMsg;
					memcpy(chatMsg.GetBuffer(), clMsg.GetBuffer(), len);
					char cFromid = (char)cl->id;
					memcpy(chatMsg.GetBuffer() + sizeof(short) + sizeof(byte), &cFromid, sizeof(byte));

					int j;
					if(cToid < 0)
					{
						for(j = 0; j < MAX_CLIENTS; j++)
						{
							if(clients[j].clstate == cl_connected && clients[j].id != cl->id)
							{
								send(clients[j].clSocket, (const char *)chatMsg.GetBuffer(), len, 0);
							}
						}
					}
					else
					{
						for(j = 0; j < MAX_CLIENTS; j++)
						{
							if(clients[j].id == cToid && clients[j].clstate == cl_connected)
							{
								send(clients[j].clSocket, (const char *)chatMsg.GetBuffer(), len, 0);
							}
						}
					}
				}
				break;

			default:
				break;
		}
	}

    return true;
}

// Free all server resources.
// NOTE: THIS FUNCTION IS CALLED AUTOMATICALLY FROM THE GUI THREAD
//		 WHEN THE USER CLICKS THE EXIT BUTTON OR CLOSES THE WINDOW.
//		 NEVER CALL THIS FUNCTION!
void SV_ShutDown(void)
{
    //Signal the server thread to shut down.
	g_cs.Enter();
	bTerminateServerThread = true;
	g_cs.Leave();

	//Catch the thread exit value.
	DWORD dwExitCode;
	GetExitCodeThread(svServerThreadHandle, &dwExitCode);

	//Ensure threads have closed BEFORE closing the app.
	if(dwExitCode == STILL_ACTIVE)
	{   
		WaitForSingleObject(svServerThreadHandle, INFINITE);
		GetExitCodeThread(svServerThreadHandle, &dwExitCode);        
	}

	//Close all handles
	if (svServerThreadHandle)
		CloseHandle(svServerThreadHandle);

	//Clean up.
	int i;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(clients[i].clstate == cl_disconnected)
			continue;

		closesocket(clients[i].clSocket);
		clients[i].clstate = cl_disconnected;
	}

	if(sv.svSocket != INVALID_SOCKET)
		closesocket(sv.svSocket);
}

// Displays text in the server's Rich Edit box.
//
// In:	str     The string to display.
void SV_DisplayText(CString str)
{
    extern CriticalSection g_HeapCS;

    int len = str.GetLength() + 1;

    g_HeapCS.Enter();

		// This memory is deleted in the GUI thread.
        char *szText = new char[len];
        strcpy(szText, (char *)str.GetBuffer());

    g_HeapCS.Leave();
    
    PostMessage(hServerDlg, UWM_PRINT_TEXT_MSG, (WPARAM)szText, 0);
}
