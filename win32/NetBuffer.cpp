#include "NetBuffer.h"

NetBuffer::NetBuffer(void)
{
	memset(m_szBuffer, 0, MAX_DATA);
	m_nPosition = 0;
}

NetBuffer::~NetBuffer(void)
{

}

byte NetBuffer::ReadByte(void)
{
	if(m_nPosition >= MAX_DATA)
		return 0;

	//m_nPosition += 1;

	return (byte)m_szBuffer[m_nPosition++];
}

char NetBuffer::ReadChar(void)
{
	if(m_nPosition >= MAX_DATA)
		return 0;

	return (char)m_szBuffer[m_nPosition++];
}

short NetBuffer::ReadShort(void)
{
	if((m_nPosition + (int)sizeof(short)) >= MAX_DATA)
		return 0;

	short temp = 0;
	memcpy(&temp, &m_szBuffer[m_nPosition], sizeof(short));

	m_nPosition += (int)sizeof(short);

	return temp;
}

void NetBuffer::ReadString(char *outBuf, int size)
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

int NetBuffer::ReadInt(void)
{
	if((m_nPosition + (int)sizeof(int)) >= MAX_DATA)
		return 0;

	int temp = 0;
	memcpy(&temp, &m_szBuffer[m_nPosition], sizeof(int));

	m_nPosition += (int)sizeof(int);

	return temp;
}

float NetBuffer::ReadFloat(void)
{
	if((m_nPosition + (int)sizeof(float)) >= MAX_DATA)
		return 0;

	float temp = 0;
	memcpy(&temp, &m_szBuffer[m_nPosition], sizeof(float));

	m_nPosition += (int)sizeof(float);

	return temp;
}

void NetBuffer::WriteByte(byte c)
{
	if(m_nPosition >= MAX_DATA)
		return;

	m_szBuffer[m_nPosition] = c;

	m_nPosition += 1;
}

void NetBuffer::WriteChar(char ch)
{
	if(m_nPosition >= MAX_DATA)
		return;

	m_szBuffer[m_nPosition] = ch;

	m_nPosition += 1;
}

void NetBuffer::WriteShort(short val)
{
	if((m_nPosition + (int)sizeof(short)) >= MAX_DATA)
		return;

	memcpy(&m_szBuffer[m_nPosition], &val, sizeof(short));

	m_nPosition += (int)sizeof(short);
}

void NetBuffer::WriteString(const char *str)
{
	int len = (int)strlen(str);

	if((len + 1) > NAME_LEN)
		return;

	if((m_nPosition + (len + 1)) >= MAX_DATA)
		return;

	strncpy((char *)&m_szBuffer[m_nPosition], str, len);
	m_nPosition += len;
}

void NetBuffer::WriteInt(int val)
{
	if((m_nPosition + (int)sizeof(int)) >= MAX_DATA)
		return;

	memcpy(&m_szBuffer[m_nPosition], &val, sizeof(int));

	m_nPosition += (int)sizeof(int);
}

void NetBuffer::WriteFloat(float val)
{
	if((m_nPosition + (int)sizeof(float)) >= MAX_DATA)
		return;

	memcpy(&m_szBuffer[m_nPosition], &val, sizeof(float));

	m_nPosition += (int)sizeof(float);
}