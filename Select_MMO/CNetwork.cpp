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
		wprintf(L"[ServerOn] -> socket() error\n");
		return (false);
	}
	wprintf(L"NetSocket\n");
	return (true);
}
