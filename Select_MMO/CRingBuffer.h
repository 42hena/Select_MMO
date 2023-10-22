#pragma once
class CRingBuffer {
public:
	// ������
	CRingBuffer(void);
	CRingBuffer(int iBufferSize);

	// �Ҹ���
	~CRingBuffer();

	// Get total buffer size
	int		GetBufferSize(void);

	int		GetUseSize(void);
	int		GetFreeSize(void);

	int		DirectEnqueueSize(void);
	int		DirectDequeueSize(void);

	int		Enqueue(char* chpData, int iSize);
	int		Dequeue(char* chpDest, int iSize);

	int		Peek(char* chpDest, int iSize);

	int		MoveRear(int iSize);
	int		MoveFront(int iSize);
	void	ClearBuffer(void);
	char* GetFrontBufferPtr(void);
	char* GetRearBufferPtr(void);

	void	Resize(int size);
private:
	int		buffer_size;
	int		free_size;
	int		use_size;
	char* buff;

	int	front_index;
	int	rear_index;
};
