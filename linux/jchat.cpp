#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "NetBuffer.h"

#define MAX_CLIENTS 5
#define PORTNUM 1119

using namespace std;

enum SClientState
{
	CL_Disconnected,
	CL_Connected
};

enum
{
				//Operation Codes
				//Server to Client
	sv_cnt = 1,	//Assigned ID upon connection
	sv_list,	//List connected users
	sv_add,		//Add user to list (Connected) 
	sv_full,	//Server full 
	sv_remove,	//Remove user from list (Disconnected)
				//Neutral
	sv_cl_msg,	//Chat Message
				//Client to Server
	cl_reg,		//Register user name
	cl_get,		//Request list of ID/User name pairs
};

struct SClient
{
	sockaddr_in		clAddr;
	int				clSocketID;
	SClientState	clState;
	int				clID;
	char			clName[NAME_LEN];
};

struct SServer
{
	int		svSocketID;
	int		nConnected;
	char	svName[NAME_LEN];
	fd_set	svMasterSet;
	fd_set	svReadSet;
};

struct SClientBuffer
{
	int				nClientID;
	unsigned int	uBufferUsed;
	unsigned char	ucBuffer[MAX_DATA];
};

SServer g_tServer;
bool g_bShutdown;
SClient g_tClients[MAX_CLIENTS];
SClientBuffer g_tClientBuffers[MAX_CLIENTS];

int ShutdownServer(void);
bool RecvCLData(SClient *cl);
void DropClient(SClient *cl);
void Input(void);
void eatline(void);

int main(int argc, char **argv)
{
	//Initialize the server data
	g_tServer.nConnected = 0;
	memset(g_tServer.svName, 0, NAME_LEN);
	sprintf(g_tServer.svName, "Server");
	FD_ZERO(&g_tServer.svMasterSet);

	//Initialize the clients data
	int i;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		memset(&g_tClients[i].clAddr, 0, sizeof(sockaddr_in));
		g_tClients[i].clSocketID = -1;
		g_tClients[i].clState = CL_Disconnected;
		memset(g_tClients[i].clName, 0, NAME_LEN);
		sprintf(g_tClients[i].clName, "Anonymous");
		g_tClients[i].clID = i + 1;

		g_tClientBuffers[i].nClientID = g_tClients[i].clID;
		g_tClientBuffers[i].uBufferUsed = 0;
		memset(g_tClientBuffers[i].ucBuffer, 0, MAX_DATA);
	}

	printf("JChat>Starting JChat Server...");

	//Create Socket
	g_tServer.svSocketID = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_tServer.svSocketID < 0)
	{
		printf("\nError: Failed to create socket!\n");

		return 0;
	}

	//Setup server address
	sockaddr_in svAddr;
	memset(&svAddr, 0, sizeof(sockaddr_in));
	svAddr.sin_family = AF_INET;
	svAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svAddr.sin_port = htons(PORTNUM);

	//Bind socket
	if(bind(g_tServer.svSocketID, (sockaddr *)&svAddr, sizeof(sockaddr_in)) < 0)
	{
		printf("\nError: Failed to bind socket!\n");

		return 0;
	}

	//Listen
	if(listen(g_tServer.svSocketID, MAX_CLIENTS) < 0)
	{
		printf("\nError: Failed to listen! O.o\n");

		return 0;
	}

	//Add stdin to the master set
	FD_SET(0, &g_tServer.svMasterSet);

	//Add the server socket to the set
	FD_SET(g_tServer.svSocketID, &g_tServer.svMasterSet);

	//Client address
	sockaddr_in clAddr;
	int nReturn;

	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	printf("Done.\nJChat>");
	fflush(stdout);
	
	g_bShutdown = false;

	//Main Loop
	while(1)
	{
		if(g_bShutdown)
			break;

		usleep(3000);

		g_tServer.svReadSet = g_tServer.svMasterSet;

		nReturn = select(FD_SETSIZE, &g_tServer.svReadSet, 0, 0, &tv);

		if(nReturn == 0)
		{
			continue;
		}

		if(nReturn < 0)
		{
			printf("Error: Select.\nJChat>");
			continue;
		}
		else
		{
			//stdin
			if(FD_ISSET(0, &g_tServer.svReadSet))
			{
				Input();
			}

			//Listen socket
			if(FD_ISSET(g_tServer.svSocketID, &g_tServer.svReadSet))
			{
				int clNewSocketID;

				int iClientAddrLen = sizeof(sockaddr_in);

				clNewSocketID = accept(g_tServer.svSocketID, (sockaddr *)&clAddr, (socklen_t *)&iClientAddrLen);

				if(clNewSocketID > 0)
				{
					bool bSlotFound = false;
					for(i = 0; i < MAX_CLIENTS; i++)
					{
						if(g_tClients[i].clState == CL_Disconnected)
						{
							printf("Assigning Socket ID:%d...\nJChat>", clNewSocketID);
							g_tClients[i].clSocketID = clNewSocketID;
							g_tClients[i].clState = CL_Connected;
							memcpy(&g_tClients[i].clAddr, &clAddr, sizeof(sockaddr_in));

							FD_SET(g_tClients[i].clSocketID, &g_tServer.svMasterSet);

							CNetBuffer cntMsg;
							short len = 0;
							cntMsg.WriteShort(len);
							cntMsg.WriteByte(sv_cnt);
							cntMsg.WriteChar(g_tClients[i].clID);

							len = cntMsg.GetPosition();
							memcpy(cntMsg.GetBuffer(), &len, sizeof(short));

							g_tClientBuffers[i].uBufferUsed = 0;
							memset(g_tClientBuffers[i].ucBuffer, 0, MAX_DATA);

							char szBuffer[128];
							memset(szBuffer, 0, 128);
							sprintf(szBuffer, "Address:%s Port:%d ID:%d - Connected.\nJChat>",
									inet_ntoa(g_tClients[i].clAddr.sin_addr),
									(int)ntohs(g_tClients[i].clAddr.sin_port),
									(int)g_tClients[i].clID);

							printf(szBuffer);

							if(!send(g_tClients[i].clSocketID, (const char*)cntMsg.GetBuffer(), len, 0))
							{
								printf("Failed to send connect messages.\nJChat>");
							}

							bSlotFound = true;
							break;
						}
					}

					if(!bSlotFound)
					{
						CNetBuffer fullMsg;
						short len = sizeof(short) + sizeof(byte);
						fullMsg.WriteShort(len);
						fullMsg.WriteByte(sv_full);

						send(clNewSocketID, (const char *)fullMsg.GetBuffer(), len, 0);

						shutdown(clNewSocketID, 2);
					}
				}
			}

			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(g_tClients[i].clState == CL_Disconnected)
				{
					continue;
				}

				if(FD_ISSET(g_tClients[i].clSocketID, &g_tServer.svReadSet))
				{
					RecvCLData(&g_tClients[i]);
				}
			}
		}
	}

	return ShutdownServer();
}

int ShutdownServer(void)
{
	shutdown(g_tServer.svSocketID, 2);

	return 1;
}

void Input(void)
{
	char szInput[256];
	memset(szInput, 0, 256);

	cin.getline(szInput, 256 - NAME_LEN);

	if(szInput[0] == '!')
	{
		if(strcmp(&szInput[1], "exit") == 0)
		{
			g_bShutdown = true;
			printf("JChat>Server Shutdown.\n");

			return;
		}

		if(strcmp(&szInput[1], "help") == 0)
		{
			printf("JChat>Commands:\n");
			printf("JChat>!exit - Shutdown server.\n");
			printf("JChat>!help - List commands.\n");
			printf("JChat>!kick # - Kick user with ID #.\n");
			printf("JChat>");
			fflush(stdout);

			return;
		}

		if(strncmp(&szInput[1], "kick", 4) == 0)
		{
			int nID = atoi(&szInput[6]);

			int i;
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(g_tClients[i].clID == nID)
				{
					printf("JChat>");
					DropClient(&g_tClients[i]);

					return;
				}
			}

			printf("Client: %d - Not Found!\nJChat>", nID);
			fflush(stdout);

			return;
		}

		printf("JChat>%s is not a valid command. Type !help for a list of commands.\n", szInput );
		printf("JChat>");
		fflush(stdout);

		return;
	}

	char temp[256];
	memcpy(temp, szInput, 256);

	sprintf(szInput, "%s>%s", g_tServer.svName, temp);
	
	CNetBuffer chatMsg;
	short len = 0;
	chatMsg.WriteShort(len);
	chatMsg.WriteByte(sv_cl_msg);
	chatMsg.WriteChar((char)0);
	chatMsg.WriteString(szInput);
	len = chatMsg.GetPosition();
	memcpy(chatMsg.GetBuffer(), &len, sizeof(short));

	int i;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(g_tClients[i].clState == CL_Connected)
		{
			send(g_tClients[i].clSocketID, (const char *)chatMsg.GetBuffer(), len, 0);
		}
	}

	printf("JChat>");

	fflush(stdout);
}

void DropClient(SClient *cl)
{
	//Build remove message.
	CNetBuffer discon;
	short len = 0;
	discon.WriteShort(len);
	discon.WriteByte(sv_remove);
	discon.WriteByte(cl->clID);
	len = discon.GetPosition();
	memcpy(discon.GetBuffer(), &len, sizeof(short));
	
	//Remove a client from the server.
	FD_CLR(cl->clSocketID, &g_tServer.svMasterSet);
	shutdown(cl->clSocketID, 2);
	cl->clState = CL_Disconnected;
	
	//Notify the remaining clients who disconnected
	//so they can remove them from their buddy list.
	int i;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(g_tClients[i].clState == CL_Connected)
		{
			send(g_tClients[i].clSocketID, (const char *)discon.GetBuffer(), len, 0);
		}
	}

	printf("Client:%d - %s, disconnected.\nJChat>", cl->clID, cl->clName);

	fflush(stdout);
}

bool RecvCLData(SClient *cl)
{
	int i;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(g_tClientBuffers[i].nClientID == cl->clID)
			break;
	}
	
	unsigned char Buffer[MAX_DATA];
	int nRecvd = 0;
	nRecvd = recv(cl->clSocketID, (char *)Buffer, MAX_DATA, 0);

	if(nRecvd <= 0)
	{
		//Client died
		DropClient(cl);
		return false;
	}

	int nRemaining = 0;
	int nReadBytes = 0;

	if((g_tClientBuffers[i].uBufferUsed + nRecvd) > MAX_DATA)
	{
		nRemaining = (g_tClientBuffers[i].uBufferUsed + nRecvd) - MAX_DATA;
	}

	if(nRemaining > 0)
	{
		nReadBytes = nRecvd - nRemaining;
		memcpy(g_tClientBuffers[i].ucBuffer + g_tClientBuffers[i].uBufferUsed, Buffer, nReadBytes);
		nRecvd = nRemaining;
	}
	else
	{
		memcpy(g_tClientBuffers[i].ucBuffer + g_tClientBuffers[i].uBufferUsed, Buffer, nRecvd);
		nReadBytes = nRecvd;
		nRecvd = 0;
	}

	g_tClientBuffers[i].uBufferUsed += nReadBytes;

	if(nRecvd > 0)
	{
		unsigned char cpyBuffer[MAX_DATA];
		memset(cpyBuffer, 0, MAX_DATA);
		memcpy(cpyBuffer, Buffer + nReadBytes, MAX_DATA - nReadBytes);
		memcpy(Buffer, cpyBuffer, MAX_DATA);
	}

	short MsgLen;
	while(1)
	{
		MsgLen = 0;
	
		memcpy(&MsgLen, g_tClientBuffers[i].ucBuffer, sizeof(short));
	
		if(MsgLen <= 0)
			break;
			
		if(MsgLen > (short)g_tClientBuffers[i].uBufferUsed)
		{
			if(nRecvd > 0)
			{
				if((g_tClientBuffers[i].uBufferUsed + nRecvd) > MAX_DATA)
				{
					nRemaining = 0;
					nReadBytes = 0;
					if((g_tClientBuffers[i].uBufferUsed + nRecvd) > MAX_DATA)
					{
						nRemaining = (g_tClientBuffers[i].uBufferUsed + nRecvd) - MAX_DATA;
					}
					
					if(nRemaining > 0)
					{
						nReadBytes = nRecvd - nRemaining;
						memcpy(g_tClientBuffers[i].ucBuffer + g_tClientBuffers[i].uBufferUsed, Buffer, nReadBytes);
						nRecvd = nRemaining;
					}
					else
					{
						memcpy(g_tClientBuffers[i].ucBuffer + g_tClientBuffers[i].uBufferUsed, Buffer, nRecvd);
						nReadBytes = nRecvd;
						nRecvd = 0;
					}
				
					g_tClientBuffers[i].uBufferUsed += nReadBytes;
					
					if(nRecvd > 0)
					{
						unsigned char cpyBuffer[MAX_DATA];
						memset(cpyBuffer, 0, MAX_DATA);
						memcpy(cpyBuffer, Buffer + nReadBytes, MAX_DATA - nReadBytes);
						memcpy(Buffer, cpyBuffer, MAX_DATA);
					}
				}
				else
				{
					memcpy(g_tClientBuffers[i].ucBuffer + g_tClientBuffers[i].uBufferUsed, Buffer, nRecvd);
					g_tClientBuffers[i].uBufferUsed += (unsigned int)nRecvd;
					nRecvd = 0;
				}
			}
			else
			{
				break;
			}
		}
			
		//Copy the complete message into a CNetBuffer class.
		CNetBuffer clMsg;
		memcpy(clMsg.GetBuffer(), g_tClientBuffers[i].ucBuffer, MsgLen);
		
		//Remove the message from the global client buffer
		unsigned char TempBuffer[MAX_DATA];
		memset(TempBuffer, 0, MAX_DATA);
		memcpy(TempBuffer, g_tClientBuffers[i].ucBuffer + MsgLen, MAX_DATA - MsgLen);
		memcpy(g_tClientBuffers[i].ucBuffer, TempBuffer, MAX_DATA);
		g_tClientBuffers[i].uBufferUsed -= MsgLen;
			
		//Parse the message from the client.
		unsigned char ucOpCode;
		clMsg.SetPosition(sizeof(short));
		ucOpCode = clMsg.ReadByte();

//		printf("Server>Processing OpCode:%d fron Client:%d.\n", (int)ucOpCode, cl->clID);
			
		switch(ucOpCode)
		{
			case cl_reg:
			{
				memset(cl->clName, 0, NAME_LEN);
				memcpy(cl->clName, clMsg.GetBuffer() + clMsg.GetPosition(), MsgLen - clMsg.GetPosition());
		
				char szBuffer[128];
				memset(szBuffer, 0, 128);
				sprintf(szBuffer, "Client: %d - Name: %s - Registered.\nJChat>", (int)cl->clID, cl->clName);
				printf(szBuffer);
			
				CNetBuffer addMsg;
				short len = 0;
				addMsg.WriteShort(len);
				addMsg.WriteByte(sv_add);
				addMsg.WriteChar(cl->clID);
				addMsg.WriteString(cl->clName);
				addMsg.WriteChar(0);
			
				len = addMsg.GetPosition();
				memcpy(addMsg.GetBuffer(), &len, sizeof(short));
			
				int j;
				for(j = 0; j < MAX_CLIENTS; j++)
				{
					if(g_tClients[j].clState == CL_Connected && g_tClients[j].clID != cl->clID)
					{
						send(g_tClients[j].clSocketID, (const char *)addMsg.GetBuffer(), len, 0);
					}
				}
		
				fflush(stdout);

				continue;
			}
			break;
			
			case cl_get:
			{
				CNetBuffer listMsg;
				short len = 0;
				listMsg.WriteShort(len);
				listMsg.WriteByte(sv_list);
				byte ucTotal = 0;
				listMsg.WriteByte(ucTotal);
			
				int j;
				for(j = 0; j < MAX_CLIENTS; j++)
				{
					if(g_tClients[j].clState == CL_Connected)
					{
						listMsg.WriteByte(g_tClients[j].clID);
						listMsg.WriteString(g_tClients[j].clName);
						listMsg.WriteChar(0);
			
						ucTotal = ucTotal + 1;
					}
				}
			
				len = listMsg.GetPosition();
			
				memcpy(listMsg.GetBuffer(), &len, sizeof(short));
				memcpy(listMsg.GetBuffer() + sizeof(short) + sizeof(byte), &ucTotal, sizeof(byte));
			
				send(cl->clSocketID, (const char *)listMsg.GetBuffer(), len, 0);
			}
			break;
			
			case sv_cl_msg:
			{
				clMsg.SetPosition(0);
				short len = clMsg.ReadShort();
				clMsg.ReadByte(); //Read Opcode
				char cToid = clMsg.ReadChar();
			
				CNetBuffer chatMsg;
				memcpy(chatMsg.GetBuffer(), clMsg.GetBuffer(), len);
				char cFromid = (char)cl->clID;
				memcpy(chatMsg.GetBuffer() + sizeof(short) + sizeof(byte), &cFromid, sizeof(byte));
			
				int j;
				if(cToid < 0)
				{
					for(j = 0; j < MAX_CLIENTS; j++)
					{
						if(g_tClients[j].clState == CL_Connected && g_tClients[j].clID != cl->clID)
						{
							send(g_tClients[j].clSocketID, (const char *)chatMsg.GetBuffer(), len, 0);
						}
					}

					printf("%s>%s\nJChat>", 
					cl->clName, 
					(char *)(chatMsg.GetBuffer() + (sizeof(short) + sizeof(byte) + sizeof(char))));

				}
				else
				{
					for(j = 0; j < MAX_CLIENTS; j++)
					{
						if(g_tClients[j].clID == cToid && g_tClients[j].clState == CL_Connected)
						{
							send(g_tClients[j].clSocketID, (const char *)chatMsg.GetBuffer(), len, 0);
							break;
						}
					}

					printf("%s>To:%s>%s\nJChat>", 
					cl->clName, 
					g_tClients[j].clName,
					(char *)(chatMsg.GetBuffer() + (sizeof(short) + sizeof(byte) + sizeof(char))));

				}

				fflush(stdout);
			}
			break;
			
			default:
			break;
		}
	}
	
	return true;
}

void eatline(void)
{
	while(cin.get() != '\n') continue;	
}

