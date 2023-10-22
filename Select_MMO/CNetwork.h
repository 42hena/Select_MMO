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
	bool NetBind();
	bool NetSetBlockMode();
	bool NetLinger();
	bool NetListen();
private:
	void InitSockAddr(SOCKADDR_IN* addr, DWORD ip_addr, WORD port);

public:
	WSADATA wsa;

	SOCKET g_listenSocket;
	u_short port;

};

#endif
