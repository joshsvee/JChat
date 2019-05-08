// cl_main : Handles all local client info. 

#include <stdio.h>
#include "cl_client.h"
#include "resource.h" 
#include "richedit.h"
#include "CriticalSection.h"
#include "NetBuffer.h"

/***********/
/* GLOBALS */
/***********/

// Used to protect shared data on the client and server.
// We can use the same object for both since only one instance
// is ever running at any given time.
extern CriticalSection g_cs;

// Handle to the chat dialog box.
extern HWND hChatDlg;

// Client recv thread ID.
DWORD  dwClientRecvThreadID = 0;

// Handle to the client recv thread.
HANDLE clClientRecvThreadHandle = 0;

// Local client info.
// Shared thread data.
client_t localClient;

// Name and ID# of clients connected.
struct buddy_t
{   
    // Player name.
    char name[NAME_LEN];

    // Client ID#
    int iID;
};

// List of people connected (in the combo box).
/*

 Filled / Updated when:
 1) local client connects to the server
 2) someone new connects to the server
 3) someone disconnects from the server.

*/
buddy_t buddies[MAX_CLIENTS];

struct tClientRecvBuffer
{
	unsigned int uTotal;
	NetBuffer Buffer;
};

tClientRecvBuffer clRecvBuffer;

/***********/
/* PUBLIC  */
/***********/

// Initialize the client and connect to the server.
//
// In:  szServerIp      IP address of the server in dotted notation ("127.0.0.1").
//      port            The server's port.
//      szPlayerName    Client's name.
//
// Return: true if successful.
// NOTE: THIS FUNCTION IS CALLED FOR YOU WHEN THE CLIENT CHAT DIALOG IS INITIALIZED.
//		 NEVER CALL THIS FUNCTION!
bool CL_Init(const char *szServerIp, unsigned short port, const char *szPlayerName)
{
    //Initialize the local client.
	memset(&localClient.addr, 0, sizeof(sockaddr_in));
	localClient.clstate = cl_disconnected;
	strcpy(localClient.name, szPlayerName);

	clRecvBuffer.uTotal = 0;
	memset(clRecvBuffer.Buffer.GetBuffer(), 0, MAX_DATA); 

	//Initialize the buddy list.
	int i;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		buddies[i].iID = -1;
		memset(buddies[i].name, 0, NAME_LEN);
	}

	CL_DisplayText("Connecting to server...");

    //Connect to the server.
	localClient.clSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(localClient.clSocket == INVALID_SOCKET)
	{
		//TODO Add Message
		return false;
	}

	localClient.addr.sin_family = AF_INET;
	localClient.addr.sin_port = htons(port);
	localClient.addr.sin_addr.s_addr = inet_addr(szServerIp);

	int nReturn = connect(localClient.clSocket, (const sockaddr *)&localClient.addr, sizeof(sockaddr_in));

	if(nReturn == SOCKET_ERROR)
	{
		//TODO Add message
		closesocket(localClient.clSocket);
		return false;
	}

	localClient.clstate = cl_connected;

	NetBuffer regMsg;
	short len = 0;
	regMsg.WriteShort(len);
	regMsg.WriteByte(cl_reg);
	regMsg.WriteString(localClient.name);

	len = regMsg.GetPosition();
	memcpy(regMsg.GetBuffer(), &len, sizeof(short));

	send(localClient.clSocket, (const char *)regMsg.GetBuffer(), len, 0);

	NetBuffer getMsg;
	len = sizeof(short) + sizeof(byte);
	getMsg.WriteShort(len);
	getMsg.WriteByte(cl_get);

	send(localClient.clSocket, (const char *)getMsg.GetBuffer(), len, 0);

    //Create a thread to receive data.
    //use _beginthreadex.
	clClientRecvThreadHandle = (HANDLE)_beginthreadex(NULL, 0, CL_ReceiveServerData, (void *)&localClient, 0, (unsigned int *)&clClientRecvThreadHandle);
	if(clClientRecvThreadHandle == 0)
	{
		//TODO Add Message
		closesocket(localClient.clSocket);
		return false;
	}

    return true;
}

// Keep reading from the server stream until disconnect.
//
// In/Out:  pData  The local client info (share thread data).
// NOTE: THIS FUNCTION RUNS ON A SEPERATE THREAD.
//		 NEVER CALL THIS FUNCTION!
unsigned int __stdcall CL_ReceiveServerData(void *pData) // localClient
{
    // The local client info.
    client_t *cl = (client_t *)pData;
     
    // TODO: Receive data from the server until they want to disconnect
    // or the server disconnects.
    //          1) Graceful
    //          2) Abnormal
    //  Use a Critical Section to protect any shared data.
	int nRecvd;
	short MsgLen = 0;

    while (1)
    {
        if(cl->clstate == cl_disconnected)
            break;

		NetBuffer recvBuffer;
		memset(recvBuffer.GetBuffer(), 0, MAX_DATA);
		nRecvd = recv(cl->clSocket, (char *)recvBuffer.GetBuffer(), MAX_DATA, 0);
		
		if(nRecvd == SOCKET_ERROR)
		{
			int nError = NET_GetErrorNumber();

			if(nError == WSAECONNRESET)
			{
				//Server forced disconnect.
				CL_DisplayText("Disconnected from server.");
			}

			return -1;
		}

		if(nRecvd == 0)
		{
			//Server died.
			CL_DisplayText("Disconnected from server.");

			return -1;
		}

		int nLeftOver = 0;
		int nReadBytes = 0;
		if((clRecvBuffer.uTotal + nRecvd) > MAX_DATA)
		{
			nLeftOver = (clRecvBuffer.uTotal + nRecvd) - MAX_DATA;
		}

		if(nLeftOver > 0)
		{
			nReadBytes = nRecvd - nLeftOver;
			memcpy(clRecvBuffer.Buffer.GetBuffer() + clRecvBuffer.uTotal, recvBuffer.GetBuffer(), nReadBytes);
			nRecvd = nLeftOver;
		}
		else
		{
			memcpy(clRecvBuffer.Buffer.GetBuffer() + clRecvBuffer.uTotal, recvBuffer.GetBuffer(), nRecvd);
			nReadBytes = nRecvd;
			nRecvd = 0;
		}

		clRecvBuffer.uTotal += nReadBytes;

		if(nRecvd > 0)
		{
			unsigned char cpyBuffer[MAX_DATA];
			memset(cpyBuffer, 0, MAX_DATA);
			memcpy(cpyBuffer, recvBuffer.GetBuffer() + nReadBytes, MAX_DATA - nReadBytes);
			memcpy(recvBuffer.GetBuffer(), cpyBuffer, MAX_DATA);
		}

		while(1)
		{
			memcpy(&MsgLen, clRecvBuffer.Buffer.GetBuffer(), sizeof(short));

			if(MsgLen < 0)
				break;

			if(MsgLen == 0)
				break;

			if(MsgLen > (short)clRecvBuffer.uTotal)
			{
				if(nRecvd > 0)
				{
					if((clRecvBuffer.uTotal + nRecvd) > MAX_DATA)
					{
						nLeftOver = 0;
						nReadBytes = 0;
						if((clRecvBuffer.uTotal + nRecvd) > MAX_DATA)
						{
							nLeftOver = (clRecvBuffer.uTotal + nRecvd) - MAX_DATA;
						}

						if(nLeftOver > 0)
						{
							nReadBytes = nRecvd - nLeftOver;
							memcpy(clRecvBuffer.Buffer.GetBuffer() + clRecvBuffer.uTotal, recvBuffer.GetBuffer(), nReadBytes);
							nRecvd = nLeftOver;
						}
						else
						{
							memcpy(clRecvBuffer.Buffer.GetBuffer() + clRecvBuffer.uTotal, recvBuffer.GetBuffer(), nRecvd);
							nReadBytes = nRecvd;
							nRecvd = 0;
						}

						clRecvBuffer.uTotal += nReadBytes;

						if(nRecvd > 0)
						{
							unsigned char cpyBuffer[MAX_DATA];
							memset(cpyBuffer, 0, MAX_DATA);
							memcpy(cpyBuffer, recvBuffer.GetBuffer() + nReadBytes, MAX_DATA - nReadBytes);
							memcpy(recvBuffer.GetBuffer(), cpyBuffer, MAX_DATA);
						}

					}
					else
					{
						memcpy(clRecvBuffer.Buffer.GetBuffer() + clRecvBuffer.uTotal, recvBuffer.GetBuffer(), MAX_DATA - clRecvBuffer.uTotal);
						clRecvBuffer.uTotal += (unsigned int)nRecvd;
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
			memcpy(clMsg.GetBuffer(), clRecvBuffer.Buffer.GetBuffer(), MsgLen);

			//Remove the message from the global client buffer
			//TODO find a better way to do this.
			unsigned char TempBuffer[MAX_DATA];
			memset(TempBuffer, 0, MAX_DATA);
			memcpy(TempBuffer, clRecvBuffer.Buffer.GetBuffer() + MsgLen, MAX_DATA - MsgLen);
			memcpy(clRecvBuffer.Buffer.GetBuffer(), TempBuffer, MAX_DATA);
			clRecvBuffer.uTotal -= MsgLen;

			//Parse the message from the client.
			unsigned char ucOpCode;
			clMsg.SetPosition(sizeof(short));
			ucOpCode = clMsg.ReadByte();

			switch(ucOpCode)
			{
				//Connect message.    
				case sv_cnt:
					{
						cl->id = clMsg.ReadChar();

						CL_DisplayText("Connected!\n");
					}
					break;

					//Return a list of clients.
				case sv_list:
					{
						int j;
						byte ucTotal = clMsg.ReadByte();

						for(j = 0; j < ucTotal; j++)
						{
							buddies[j].iID = clMsg.ReadByte();
							clMsg.ReadString(buddies[j].name, NAME_LEN);

							CL_AddCbBuddyData(buddies[j].name, buddies[j].iID);
						}
					}
					break;

				//Add a single client to the combo box / buddy list.
				case sv_add:
					{
						int j;
						byte id = clMsg.ReadByte();
						for(j = 0; j < MAX_CLIENTS; j++)
						{
							if(buddies[j].iID == id)
								break;

							if(buddies[j].iID < 0)
							{
								buddies[j].iID = id;
								clMsg.ReadString(buddies[j].name, NAME_LEN);
								CL_AddCbBuddyData(buddies[j].name, buddies[j].iID);

								char szBuffer[128];
								memset(szBuffer, 0, 128);
								sprintf(szBuffer, "%s connected.\n", buddies[j].name);

								CL_DisplayText(szBuffer);

								break;
							}
						}
					}
					break;

				//Server is full.
				case sv_full:
					{
						CL_DisplayText("Server Full! Try again later!");
					}
					break;

				//Someone disconnected so remove them from the combo box / buddy list.
				case sv_remove:
					{
						byte id = clMsg.ReadByte();

						int j;
						for(j = 0; j < MAX_CLIENTS; j++)
						{
							if(buddies[j].iID == id)
							{
								CL_RemoveCbBuddyData(buddies[j].name);

								char szBuffer[128];
								memset(szBuffer, 0, 128);
								sprintf(szBuffer, "%s disconnected.\n", buddies[j].name);
								CL_DisplayText(szBuffer);

								memset(buddies[j].name, 0, NAME_LEN);
								buddies[j].iID = -1;

								break;
							}
						}
					}
					break;

				//Chat msg
				case sv_cl_msg:
					{
						char cFromid = clMsg.ReadChar();

						char szBuffer[MAX_DATA];
						memset(szBuffer, 0, MAX_DATA);

						char szString[MAX_DATA];
						memset(szString, 0, MAX_DATA);

						clMsg.ReadString(szString, MAX_DATA);

						int j;
						for(j = 0; j < MAX_CLIENTS; j++)
						{
							if(buddies[j].iID == cFromid)
							{
								sprintf(szBuffer, "%s> %s\n", buddies[j].name, szString);
								CL_DisplayText(szBuffer);

								break;
							}
						}	
					}
					break;

				default:
					break;
			}
		}
    }

    return 0; // _endthreadex(0)
}

// Send chat text to the server.
//
// In:  szChatText  The text.
//      idTo        ID # of the person we send to or -1 for send to all.
// NOTE: THIS FUNCTION IS AUTOMATICALLY CALLED WHEN THE USER CLICKS THE SEND BUTTON.
//		 NEVER CALL THIS FUNCTION!
void CL_SendChatMessage(const char *szChatText, int idTo)
{
    //Build the chat message.
	NetBuffer chatMsg;
	short len = 0;
	chatMsg.WriteShort(len);
	chatMsg.WriteByte(sv_cl_msg);
	chatMsg.WriteChar((char)idTo);
	chatMsg.WriteString(szChatText);

	//Set the chat message length.
	len = chatMsg.GetPosition();
	memcpy(chatMsg.GetBuffer(), &len, sizeof(short));

	//use to test partial message
	//char szTemp[MAX_DATA * 2];
	//memset(szTemp, 0, MAX_DATA * 2);
	//memcpy(szTemp, chatMsg.GetBuffer(), len);
	//memcpy(szTemp + len, chatMsg.GetBuffer(), len);

	//Send the chat message to the server.
	send(localClient.clSocket, (const char *)chatMsg.GetBuffer(), len, 0);

	//Partial message test send.
	//send(localClient.clSocket, (const char *)szTemp, len * 2, 0);

	char szBuffer[MAX_DATA + 128];
	memset(szBuffer, 0, MAX_DATA + 128);
	sprintf(szBuffer, "%s> %s\n", localClient.name, szChatText);

	CL_DisplayText(szBuffer);
}

// Frees all client resources.
// NOTE: THIS FUNCTION IS CALLED AUTOMATICALLY FROM THE GUI THREAD
//		 WHEN THE USER CLICKS THE EXIT BUTTON OR CLOSES THE WINDOW.
//		 NEVER CALL THIS FUNCTION!
void CL_ShutDown(void)
{    
    //Close all open sockets. 
	closesocket(localClient.clSocket);
	localClient.clstate = cl_disconnected;

	//Catch the thread exit value.
	DWORD dwExitCode;
	GetExitCodeThread(clClientRecvThreadHandle, &dwExitCode);

	//Shutdown any threads.
	if(dwExitCode == STILL_ACTIVE)
	{   
		WaitForSingleObject(clClientRecvThreadHandle, INFINITE);
		GetExitCodeThread(clClientRecvThreadHandle, &dwExitCode);        
	}

	//Close all handles.
	if (clClientRecvThreadHandle)
		CloseHandle(clClientRecvThreadHandle);
}

// Set the current selection to the first item in the combo box.
void CL_SetCbToFirstItem(void)
{
    // Set the current selection to the first item in the combo box.
    MTVERIFY(SendMessageTimeout(GetDlgItem(hChatDlg, IDC_COMBO_NAMES), 
                CB_SETCURSEL, 0, 0, SMTO_ABORTIFHUNG, 200, NULL));
} 

// Adds data to the combo box.
//
// In:  szName  The name of a buddy.
//      id      The id number of the buddy.
void CL_AddCbBuddyData(const char *szName, int id)
{
    extern CriticalSection g_HeapCS;
	int len = (int)strlen(szName) + 1;

    g_HeapCS.Enter();

		// This memory is deleted in the GUI thread.
		char *szCopy = new char[len];
		strcpy(szCopy, szName);

    g_HeapCS.Leave();

    PostMessage(hChatDlg, UWM_CB_ADDSTRING_MSG, (WPARAM)szCopy, id);
}

// Get the data (ID) associated with a buddy's name in the combo box.
//
// Return: the ID # of the currently selected buddy in the combo box or -1 for
// no item selected.
int CL_GetCbBuddyData(void)
{
    // Get the index of the current selected string.
    int iComboSel;
    MTVERIFY(SendMessageTimeout(GetDlgItem(hChatDlg, IDC_COMBO_NAMES), 
                                CB_GETCURSEL, 0, 0, SMTO_ABORTIFHUNG, 200, (DWORD *)&iComboSel));

    if (iComboSel == CB_ERR)
        // No item selected in the combo box.
        return -1;

    // Get the data associated with the selected string.
    int id;
    MTVERIFY(SendMessageTimeout(GetDlgItem(hChatDlg, IDC_COMBO_NAMES), 
                            CB_GETITEMDATA, iComboSel, 0, SMTO_ABORTIFHUNG, 200, (DWORD *)&id));
    if (id == CB_ERR)
        return -1;

	return id;
}

// Remove a name from the combo box.
//
// In:  The name of the buddy to remove.
//
// Return: true if the name was removed.
bool CL_RemoveCbBuddyData(const char *szName)
{
    // Get the index in the combo box of the string to remove.
    int iIndex;
    MTVERIFY(SendMessageTimeout(GetDlgItem(hChatDlg, IDC_COMBO_NAMES), 
                    CB_FINDSTRING, -1, (LPARAM)szName, SMTO_ABORTIFHUNG, 500, (DWORD *)&iIndex));

    if (iIndex == CB_ERR)
    {
        return false;
    }

    // Kill it.
    MTVERIFY(SendMessageTimeout(GetDlgItem(hChatDlg, IDC_COMBO_NAMES), CB_DELETESTRING, iIndex, 0, 
                SMTO_ABORTIFHUNG, 500, NULL));

    // Set the current selection to the first item in the combo box.
    MTVERIFY(SendMessageTimeout(GetDlgItem(hChatDlg, IDC_COMBO_NAMES), 
                CB_SETCURSEL, 0, 0, SMTO_ABORTIFHUNG, 200, NULL));

    return true;
}

// Displays text in the clients Rich Edit box.
//
// In:  str     The string to display.
void CL_DisplayText(CString str)
{
    extern CriticalSection g_HeapCS;

    int len = str.GetLength() + 1;

    g_HeapCS.Enter();

		// This memory is deleted in the GUI thread.
        char *szText = new char[len];
        strcpy(szText, (char *)str.GetBuffer());

    g_HeapCS.Leave();

    PostMessage(hChatDlg, UWM_PRINT_TEXT_MSG, (WPARAM)szText, 0);
}
