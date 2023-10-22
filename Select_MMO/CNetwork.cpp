#pragma comment(lib, "ws2_32")

#include <iostream>
#include <WinSock2.h>

#include "CNetwork.h"

CNetwork::CNetwork()
	: g_listenSocket(INVALID_SOCKET)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		wprintf(L"[CNetwork] -> WSAStartup()\n");
		return ;
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
