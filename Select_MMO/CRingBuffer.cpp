#include <memory.h>
#include "CRingBuffer.h"


// ##################################################
// #                  Constructor                   #
// ##################################################

CRingBuffer::CRingBuffer(void)
	: buffer_size(5 + 1),
	free_size(buffer_size - 1),
	use_size(0), front_index(0), rear_index(0)
{
	buff = new char[buffer_size];
	// 예외를 던져야 할까?
}

CRingBuffer::CRingBuffer(int iBufferSize)
	: buffer_size(iBufferSize + 1),
	free_size(iBufferSize),
	use_size(0), front_index(0), rear_index(0)
{
	buff = new char[buffer_size];
}

// ##################################################
// #                   Destructor                   #
// ##################################################

CRingBuffer::~CRingBuffer()
{
	delete[] buff;
}

// ------------------------------------------------------

// ##################################################
// #          Buffer의 총 크기를 얻는 함수          #
// ##################################################

int		CRingBuffer::GetBufferSize(void)
{
	return (buffer_size);
}

// ##################################################
// #          Buffer에서 현재 사용되고 있는 크기    #
// ##################################################

int		CRingBuffer::GetUseSize(void)
{
	return (use_size);
}

// ##################################################
// #          Buffer에서 남아있는 크기              #
// ##################################################

int		CRingBuffer::GetFreeSize(void)
{
	return (free_size);
}

void	CRingBuffer::ClearBuffer(void)
{
	front_index = rear_index = 0;
	free_size = buffer_size - 1;
	use_size = 0;
}

int	CRingBuffer::DirectEnqueueSize(void)
{
	if (front_index == 0)
		return buffer_size - rear_index - 1;
	else
		return buffer_size - rear_index;
}

int	CRingBuffer::DirectDequeueSize(void)
{
	if (rear_index >= front_index)
		return rear_index - front_index;
	else
		return buffer_size - front_index;
}

char* CRingBuffer::GetFrontBufferPtr(void)
{
	return buff + front_index;
}

char* CRingBuffer::GetRearBufferPtr(void)
{
	return buff + rear_index;
}


int		CRingBuffer::MoveRear(int iSize)
{
	if (buff + buffer_size - 1 < buff + rear_index + iSize)
	{
		rear_index = iSize - DirectEnqueueSize();
		//direct_enqueue_size = free_size - iSize;
	}
	else
	{
		rear_index += iSize;
		//direct_enqueue_size -= iSize;
	}
	use_size += iSize;
	free_size -= iSize;

	return iSize;
}

int	CRingBuffer::MoveFront(int iSize)
{
	if (buff + buffer_size - 1 < buff + front_index + iSize)
	{
		front_index = iSize - DirectDequeueSize();
		//direct_dequeue_size = use_size - iSize;
	}
	else
	{
		front_index += iSize;
		//direct_dequeue_size -= iSize;
	}
	use_size -= iSize;
	free_size += iSize;

	return iSize;
}

int		CRingBuffer::Enqueue(char* chpData, int iSize)
{
	// 용량 체크
	if (iSize <= free_size)
	{
		if (DirectEnqueueSize() < iSize)
		{
			memcpy(buff + rear_index, chpData, DirectEnqueueSize());
			memcpy(buff, chpData + DirectEnqueueSize(), iSize - DirectEnqueueSize());
		}
		else
		{
			memcpy(buff + rear_index, chpData, iSize);
		}
		MoveRear(iSize);
		return (iSize);
	}
	else
		return (0);
}

int		CRingBuffer::Dequeue(char* chpDest, int iSize)
{
	// 용량 체크
	iSize = (use_size < iSize) ? use_size : iSize;

	if (DirectDequeueSize() < iSize)
	{
		memcpy(chpDest, buff + front_index, DirectDequeueSize());
		memcpy(chpDest + DirectDequeueSize(), buff, iSize - DirectDequeueSize());
	}
	else
	{
		memcpy(chpDest, GetFrontBufferPtr(), iSize);
	}
	MoveFront(iSize);
	return (iSize);
}

int		CRingBuffer::Peek(char* chpDest, int iSize)
{
	// 용량 체크
	iSize = (use_size < iSize) ? use_size : iSize;

	if (DirectDequeueSize() < iSize)
	{
		memcpy(chpDest, buff + front_index, DirectDequeueSize());
		memcpy(chpDest + DirectDequeueSize(), buff, iSize - DirectDequeueSize());
	}
	else
	{
		memcpy(chpDest, GetFrontBufferPtr(), iSize);
	}
	return (iSize);
}