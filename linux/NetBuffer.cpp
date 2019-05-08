#include "NetBuffer.h"
#include <string.h>
#include <memory.h>

CNetBuffer::CNetBuffer(void)
{
	memset(m_szBuffer, 0, MAX_DATA);
	m_nPosition = 0;
}

CNetBuffer::~CNetBuffer(void)
{

}

byte CNetBuffer::ReadByte(void)
{
	if(m_nPosition >= MAX_DATA)
		return 0;

	return (byte)m_szBuffer[m_nPosition++];
}

char CNetBuffer::ReadChar(void)
{
	if(m_nPosition >= MAX_DATA)
		return 0;

	return (char)m_szBuffer[m_nPosition++];
}

short CNetBuffer::ReadShort(void)
{
	if((m_nPosition + (int)sizeof(short)) >= MAX_DATA)
		return 0;

	short temp = 0;
	memcpy(&temp, &m_szBuffer[m_nPosition], sizeof(short));

	m_nPosition += (int)sizeof(short);

	return temp;
}

void CNetBuffer::ReadString(char *outBuf, int size)
{
	int i;
	for(i = 0; i < size; i++)
	{
		if(m_szBuffer[m_nPosition + i] > 0)
		{
			outBuf[i] = m_szBuffer[m_nPosition + i];
		}
		else
		{
			outBuf[i] = m_szBuffer[m_nPosition + i];
			break;
		}
	}

	m_nPosition += (i + 1);
}

int CNetBuffer::ReadInt(void)
{
	if((m_nPosition + (int)sizeof(int)) >= MAX_DATA)
		return 0;

	int temp = 0;
	memcpy(&temp, &m_szBuffer[m_nPosition], sizeof(int));

	m_nPosition += (int)sizeof(int);

	return temp;
}

float CNetBuffer::ReadFloat(void)
{
	if((m_nPosition + (int)sizeof(float)) >= MAX_DATA)
		return 0;

	float temp = 0;
	memcpy(&temp, &m_szBuffer[m_nPosition], sizeof(float));

	m_nPosition += (int)sizeof(float);

	return temp;
}

void CNetBuffer::WriteByte(byte c)
{
	if(m_nPosition >= MAX_DATA)
		return;

	m_szBuffer[m_nPosition] = c;

	m_nPosition += 1;
}

void CNetBuffer::WriteChar(char ch)
{
	if(m_nPosition >= MAX_DATA)
		return;

	m_szBuffer[m_nPosition] = ch;

	m_nPosition += 1;
}

void CNetBuffer::WriteShort(short val)
{
	if((m_nPosition + (int)sizeof(short)) >= MAX_DATA)
		return;

	memcpy(&m_szBuffer[m_nPosition], &val, sizeof(short));

	m_nPosition += (int)sizeof(short);
}

void CNetBuffer::WriteString(const char *str)
{
	int len = (int)strlen(str);

	if((len + 1) > NAME_LEN)
		return;

	if((m_nPosition + (len + 1)) >= MAX_DATA)
		return;

	strncpy((char *)&m_szBuffer[m_nPosition], str, len);
	m_nPosition += len;
}

void CNetBuffer::WriteInt(int val)
{
	if((m_nPosition + (int)sizeof(int)) >= MAX_DATA)
		return;

	memcpy(&m_szBuffer[m_nPosition], &val, sizeof(int));

	m_nPosition += (int)sizeof(int);
}

void CNetBuffer::WriteFloat(float val)
{
	if((m_nPosition + (int)sizeof(float)) >= MAX_DATA)
		return;

	memcpy(&m_szBuffer[m_nPosition], &val, sizeof(float));

	m_nPosition += (int)sizeof(float);
}

