#ifndef __CLASS_NETWORK_H__
#define __CLASS_NETWORK_H__

#include <unordered_map>

class CNetwork
{
public:
	// ctor
	CNetwork();
	// dtor
	~CNetwork();

	
	void NetworkIO();
private:
	bool NetSocket();
	void InitSockAddr(SOCKADDR_IN* addr, DWORD ip_addr, WORD port);
	bool NetBind();
	bool NetSetBlockMode();
	bool NetLinger();
	bool NetListen();
	void NetworkSelect(SOCKET* socketTable, FD_SET* readSet, FD_SET* writeSet);
	
	bool NetworkAccept();
	void NetworkRecv(SOCKET socket);

public:
	WSADATA wsa;

	SOCKET g_listenSocket;
	u_short port;

private:
	std::unordered_map<SOCKET, st_Session*> g_sessionMap;

};

#endif
