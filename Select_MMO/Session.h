#ifndef __SESSION_H__
#define __SESSION_H__

//#include "CRingBuffer.h"

struct st_Session
{
	SOCKET		socket;
	DWORD		session_id;
	/*CRingBuffer	send_Q;
	CRingBuffer	recv_Q;*/
	DWORD		last_recv_time;
	bool		disconnect_flag;
	st_Session()
		//: send_Q(2000), recv_Q(2000)
	{}
};

#endif
