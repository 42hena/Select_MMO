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
		wprintf(L"[NetLinger] -> setsockopt() error\n");
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
		wprintf(L"[NetListen] -> listen() error\n");
		return (false);
	}
	wprintf(L"NetListen\n");
	return (true);
}

bool CNetwork::NetworkAccept()
{
	st_Session* newSession;
	st_Character* newCharacter;

	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	int client_len;

	client_len = sizeof(SOCKADDR_IN);
	clientSocket = accept(g_listenSocket, (SOCKADDR*)&clientAddr, &client_len);
	if (clientSocket == INVALID_SOCKET)
	{
		wprintf(L"[NetworkAccept] -> accept() Error\n");
		return (false);
	}

	//
	newSession = new st_Session(clientSocket, 0, timeGetTime());
	g_sessionMap.insert({ clientSocket, newSession });
	//
	wprintf(L"newSession socket[%d\n", clientSocket);
	wprintf(L"Accept Success\n");
	return true;
}

void CNetwork::NetworkRecv(SOCKET socket)
{
	st_Session* session;
	int recv_ret;
// -----

	Session_Type iter = g_sessionMap.find(socket);
	wprintf(L"Input socket[%d\n", socket);
	if (iter == g_sessionMap.end())
	{
		wprintf(L"[NetworkRecv] -> Find error\n");
		return ;
	}

	// session 값 바
	session = iter->second;

	session->lastRecvTime = timeGetTime();

	//session->recv_Q.GetUseSize();
	recv_ret = recv(session->socket, session->recvQ.GetRearBufferPtr(), session->recvQ.DirectEnqueueSize(), 0);
	if (recv_ret == 0)
	{
#ifdef DEBUG
		wprintf(L"정상 종료Game id[%d]\n", session->session_id);
#endif
		session->disconnectFlag = true;
		return;
	}
	else if (recv_ret == SOCKET_ERROR)
	{
		int err_code = GetLastError();
		if (err_code == WSAEWOULDBLOCK)
		{
			return;
		}
		else if (err_code == 10054)
		{
#ifdef DEBUG
			printf("recv Error[%d]\n", err_code);
#endif
		}
		else
		{
			wprintf(L"another recv errorGame id[%d] code[%d]\n", session->sessionID, err_code);
		}
		session->disconnectFlag = true;
		return;
	}

	if (recv_ret > 0)
	{
		// Move pointer
		wprintf(L"recv_ret[%d]\n", recv_ret);
		int ret = session->recvQ.MoveRear(recv_ret);
		wprintf(L"moveRear[%d]\n", ret);
	}
}


void CNetwork::NetworkSelect(SOCKET* socketTable, FD_SET* readSet, FD_SET* writeSet)
{
	int socketCount;
	timeval time;
	int errCode;

	time.tv_sec = 0;
	time.tv_usec = 0;

	
	socketCount = select(0, readSet, writeSet, nullptr, &time);
	if (socketCount > 0)
	{
		wprintf(L"Socket Cnt[%d]\n", socketCount);
		for (int i = 0 ; i < 64 ; ++i)
		{
			bool flag = true;
			if (socketTable[i] == 0)
			{
				// wprintf(L"Socket X[%d]\n", i);
				continue;
			}

			if (FD_ISSET(socketTable[i], writeSet))
			{

			}

			if (FD_ISSET(socketTable[i], readSet))
			{
				if (flag)
				{
					if (socketTable[i] == g_listenSocket)
						NetworkAccept();
					else
						NetworkRecv(socketTable[i]);
				}
			}
		}
	}
	
	// select 함수 Error 체크
	if (socketCount == SOCKET_ERROR)
	{
		errCode = GetLastError();
		wprintf(L"select ERROR[%d]\n", errCode);
	}
}

void CNetwork::NetworkIO()
{
	st_Session* session;

	SOCKET socketTable[FD_SETSIZE] = { INVALID_SOCKET, };
	int socketCount = 0;

	FD_SET readSet;
	FD_SET writeSet;

	// fd 초기화 
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);

	socketTable[socketCount++] = g_listenSocket;
	FD_SET(g_listenSocket, &readSet);

	for (Session_Type iter = g_sessionMap.begin(); iter != g_sessionMap.end(); ++iter)
	{
		session = iter->second;

		socketTable[socketCount++] = session->socket;

		// Read는 무조건 넣기
		FD_SET(session->socket, &readSet);
		if (session->sendQ.GetUseSize() > 0)
		{
			FD_SET(session->socket, &writeSet);
		}

		if (FD_SETSIZE <= socketCount)
		{
			NetworkSelect(socketTable, &readSet, &writeSet);
			FD_ZERO(&readSet);
			FD_ZERO(&writeSet);


			ZeroMemory(socketTable, sizeof(socketTable));

			FD_SET(g_listenSocket, &readSet);
			socketTable[0] = g_listenSocket;

			socketCount = 1;
		}
	}

	if (socketCount > 0)
		NetworkSelect(socketTable, &readSet, &writeSet);
}
