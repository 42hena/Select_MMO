#ifndef __CLASS_NETWORK_H__
#define __CLASS_NETWORK_H__

class CNetwork
{
public:
	// ctor
	CNetwork();
	// dtor
	~CNetwork();

	bool NetSocket();

private:
	

public:
	WSADATA wsa;

	SOCKET g_listenSocket;
};

#endif
