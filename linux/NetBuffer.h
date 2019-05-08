#ifndef _NETBUFFER_H_
#define _NETBUFFER_H_

#ifndef MAX_DATA
#define MAX_DATA 512
#endif

#ifndef NAME_LEN
#define NAME_LEN 31
#endif

typedef unsigned char byte;

class CNetBuffer
{
	private:
		char m_szBuffer[MAX_DATA];
		int m_nPosition;

	public:
		CNetBuffer(void);
		~CNetBuffer(void);

		inline char *GetBuffer(void){return m_szBuffer;}
		inline int GetPosition(void){return m_nPosition;}
		inline void SetPosition(int nPos)
		{
			if(nPos < MAX_DATA)
				m_nPosition = nPos;
		}

		byte	ReadByte(void);
		char	ReadChar(void);
		short	ReadShort(void);
		void	ReadString(char *outBuf, int size);
		int		ReadInt(void);
		float	ReadFloat(void);

		void WriteByte(byte c);
		void WriteChar(char ch);
		void WriteShort(short val);
		void WriteString(const char *str);
		void WriteInt(int val);
		void WriteFloat(float val);
};

#endif //#ifndef _NETBUFFER_H_

