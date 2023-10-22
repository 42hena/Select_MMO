#pragma comment(lib, "ws2_32")

#include <iostream>
#include <WinSock2.h>

#include "Session.h"
#include "CNetwork.h"


#define Session_Type std::unordered_map<SOCKET, st_Session*>::iterator 

CNetwork::CNetwork()
	: g_listenSocket(INVALID_SOCKET)
{
	int ret;
// -----

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		wprintf(L"[CNetwork] -> WSAStartup()\n");
		return ;
	}
	
	ret = NetSocket();
	if (ret == false)
	{
		return;
	}

	ret = NetBind();
	if (ret == false)
	{
		return;
	}

	ret = NetSetBlockMode();
	if (ret == false)
	{
		return;
	}

	ret = NetLinger();
	if (ret == false)
	{
		return;
	}

	ret = NetListen();
	if (ret == false)
	{
		return;
	}

	wprintf(L"CNetwork Ctor\n");
}

CNetwork::~CNetwork()
{
	if (WSACleanup() != 0)
	{
		wprintf(L"[~CNetwork] -> WSACleanup()\n");
		return ;
	}
	wprintf(L"~CNetwork Dtor\n");
}

bool CNetwork::NetSocket()
{
	g_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (g_listenSocket == INVALID_SOCKET)
	{
		wprintf(L"[NetSocket] -> socket() error\n");
		return (false);
	}
	wprintf(L"NetSocket\n");
	return (true);
}

void CNetwork::InitSockAddr(SOCKADDR_IN* addr, DWORD ip_addr, WORD port)
{
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = ip_addr;
	addr->sin_port = htons(port);
}

bool CNetwork::NetBind()
{
	SOCKADDR_IN addr;

	int bindRet;
// -----

	ZeroMemory(&addr, sizeof(addr));
	InitSockAddr(&addr, INADDR_ANY, 10801);

	bindRet = bind(g_listenSocket, (SOCKADDR*)&addr, sizeof(addr));
	if (bindRet == SOCKET_ERROR)
	{
		wprintf(L"[NetBind] -> bind() error\n");
		return (false);
	}
	wprintf(L"NetBind\n");
	return (true);
}

bool CNetwork::NetSetBlockMode()
{
	int ret = 0;
	u_long blockingMode = 0;
	u_long nonBlockingMode = 1;
// -----

	ret = ioctlsocket(g_listenSocket, FIONBIO, &nonBlockingMode);
	if (ret == SOCKET_ERROR)
	{
		wprintf(L"[NetSetBlockMode] -> ioctlsocket() error\n");
		return (false);
	}
	wprintf(L"NetSetBlockMode\n");
	return (true);
}

bool CNetwork::NetLinger()
{
	LINGER _linger;

	int setsocketoptRet;
// -----

	_linger.l_onoff = 1;
	_linger.l_linger = 0;

	setsocketoptRet = setsockopt(g_listenSocket, SOL_SOCKET, SO_LINGER, (char*)&_linger, sizeof(_linger));
	if (setsocketoptRet == SOCKET_ERROR)
	{
		printf("[NetLinger] -> setsockopt() error\n");
		return (false);
	}
	wprintf(L"NetLinger\n");
	return (true);
}

bool CNetwork::NetListen()
{
	int listenRet;
// -----

	listenRet = listen(g_listenSocket, SOMAXCONN_HINT(65535));
	if (listenRet == SOCKET_ERROR)
	{
		printf("[NetListen] -> listen() error\n");
		return (false);
	}
	wprintf(L"NetListen\n");
	return (true);
}

void CNetwork::NetworkIO()
{
	st_Session* session;

	SOCKET socketTable[FD_SETSIZE] = { INVALID_SOCKET, };
	int socketCount = 0;

	FD_SET readSet;
	FD_SET writeSet;

	// fd ÃÊ±âÈ­ 
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);

	socketTable[socketCount++] = g_listenSocket;
	FD_SET(g_listenSocket, &readSet);

	if (socketCount > 0)
		;
}
