#include <iostream>
#include <WinSock2.h>

#include "CNetwork.h"

CNetwork::CNetwork()
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
