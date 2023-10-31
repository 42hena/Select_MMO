#ifndef  __CSerialization_H__
#define  __CSerialization_H__

class CSerialization
{
public:
	enum en_PACKET
	{
		e_BUFFER_DEFAULT = 1400		// ��Ŷ�� �⺻ ���� ������.
	};

	// ������
	CSerialization();
	CSerialization(int iBufferSize);

	// ��� ��� + �Ҹ���
	virtual ~CSerialization();

	// ####################################################################################################
	// #                   Utils                                                                          #
	// ####################################################################################################

		// ##################################################
		// # ���� ó�� ���·� �ǵ�����                      #
		// #                                                #
		// # Param  : (None)                                # 
		// # return : (None)                                # 
		// ##################################################
	void	Clear(void);

	// ##################################################
	// # �ִ� ������ ũ��                               #
	// #                                                #
	// # Param  : (None)                                # 
	// # return : (int : Max Buffer size)               # 
	// ##################################################
	int	GetBufferSize(void) { return iBufferSize; }

	// ##################################################
	// # ���� ���� ���� ������ ������                   #
	// #                                                #
	// # Param  : (None)                                #
	// # return : (int : using Buffer size)             #
	// ##################################################
	__int64		GetDataSize(void) { return pPush - pPop; }

	// ##################################################
	// # ������ ó�� ��ġ�� ��ȯ                        #
	// #                                                #
	// # Param  : (None)                                #
	// # return : (char * : buffer_ptr)                 #
	// ##################################################
	char* GetBufferPtr(void) { return buffer; }

	//////////////////////////////////////////////////////////////////////////
	// ���� Pos �̵�. (�����̵��� �ȵ�)
	// GetBufferPtr �Լ��� �̿��Ͽ� �ܺο��� ������ ���� ������ ������ ��� ���. 
	//
	// Parameters: (int) �̵� ������.
	// Return: (int) �̵��� ������.
	//////////////////////////////////////////////////////////////////////////
	int		MoveWritePos(int iSize);
	int		MoveReadPos(int iSize);


	//////////////////////////////////////////////////////////////////////////
	// ����Ÿ ���.
	//
	// Parameters: (char *)Dest ������. (int)Size.
	// Return: (int)������ ������.
	//////////////////////////////////////////////////////////////////////////
	int		GetData(char* chpDest, int iSize);

	//////////////////////////////////////////////////////////////////////////
	// ����Ÿ ����.
	//
	// Parameters: (char *)Src ������. (int)SrcSize.
	// Return: (int)������ ������.
	//////////////////////////////////////////////////////////////////////////
	int		PutData(char* chpSrc, int iSrcSize);


	/* ============================================================================= */
	// ������ �����ε�
	/* ============================================================================= */
	CSerialization& operator = (CSerialization& clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// �ֱ�.	�� ���� Ÿ�Ը��� ��� ����.
	//////////////////////////////////////////////////////////////////////////
	CSerialization& operator << (bool);

	CSerialization& operator << (char);
	CSerialization& operator << (unsigned char);

	CSerialization& operator << (short);
	CSerialization& operator << (unsigned short);

	CSerialization& operator << (int);
	CSerialization& operator << (unsigned int);

	CSerialization& operator << (long);
	CSerialization& operator << (unsigned long);

	CSerialization& operator << (__int64);
	CSerialization& operator << (unsigned __int64);

	CSerialization& operator << (float);
	CSerialization& operator << (double);

	//////////////////////////////////////////////////////////////////////////
	// ����.	�� ���� Ÿ�Ը��� ��� ����.
	//////////////////////////////////////////////////////////////////////////
	CSerialization& operator >> (bool&);

	CSerialization& operator >> (char&);
	CSerialization& operator >> (unsigned char&);

	CSerialization& operator >> (short&);
	CSerialization& operator >> (unsigned short&);

	CSerialization& operator >> (int&);
	CSerialization& operator >> (unsigned int&);

	CSerialization& operator >> (long&);
	CSerialization& operator >> (unsigned long&);

	CSerialization& operator >> (__int64&);
	CSerialization& operator >> (unsigned __int64&);

	CSerialization& operator >> (float&);
	CSerialization& operator >> (double&);

protected:
	char* pPush;
	char* pPop;
	char* buffer;
	int	iBufferSize;
};

#endif
