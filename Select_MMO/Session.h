#ifndef __SESSION_H__
#define __SESSION_H__

#include "CRingBuffer.h"

struct st_Session
{
	st_Session()
		: socket(0), sessionID(0), sendQ(2000), recvQ(2000), lastRecvTime(0), disconnectFlag(0)
	{}
	st_Session(SOCKET sock, DWORD sessionID, DWORD lastTime)
		: socket(sock), sessionID(sessionID), sendQ(2000), recvQ(2000), lastRecvTime(lastTime), disconnectFlag(0)
	{}

// variable
	SOCKET		socket;
	DWORD		sessionID;
	CRingBuffer	sendQ;
	CRingBuffer	recvQ;
	DWORD		lastRecvTime;
	bool		disconnectFlag;
};

#endif
