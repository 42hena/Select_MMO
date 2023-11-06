#pragma comment(lib, "ws2_32")

#include <iostream>
#include <WinSock2.h>

#include "Session.h"
#include "Character.h"
#include "CNetwork.h"
#include "SerializationBuffer.h"
#include "SendPacket.h"
#include "CreatePacket.h"
#include "Util.h"
#include "PROTOCOL.h"

extern std::unordered_map<SOCKET, st_Session*> g_sessionMap;
extern std::unordered_map<DWORD, st_Character*> g_characterMap;
extern std::list< st_Character* > g_sector[RANGE_MOVE_BOTTOM / SECTOR_MAX_Y][RANGE_MOVE_RIGHT / SECTOR_MAX_X];
extern DWORD g_sendCnt, g_recvCnt, g_acceptCnt, g_selectCnt;

#define Session_Type std::unordered_map<SOCKET, st_Session*>::iterator 
#define Character_Type std::unordered_map<DWORD, st_Character*>::iterator 



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
	InitSockAddr(&addr, INADDR_ANY, 10201);

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
	static DWORD id;

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
	g_acceptCnt++;
	int newID = id++;

	newSession = new st_Session(clientSocket, newID, timeGetTime());
	g_sessionMap.insert({ clientSocket, newSession });

	//
	newCharacter = new st_Character(newSession, newID, 8, 0, 100);


	// 순서 체크해보기.
	g_characterMap.insert({ newID, newCharacter });
	g_sector[newCharacter->sector.sec_y][newCharacter->sector.sec_x].push_back(newCharacter);
	
	// 접속 후 작업
	PushCreateMyCharacterJob(newSession, newID, newCharacter->direction, newCharacter->x, newCharacter->y, newCharacter->hp);
	PushCreateOtherCharacterToMeJob(newSession, newID, newCharacter->direction, newCharacter->x, newCharacter->y, newCharacter->hp);
	PushCreateMyCharacterToOthersJob(newSession, newID, newCharacter->direction, newCharacter->x, newCharacter->y, newCharacter->hp);
	PushCreateOtherMoveStartTomeJob(newSession, newID, newCharacter->direction, newCharacter->x, newCharacter->y, newCharacter->hp);

	return true;
}

void CNetwork::NetworkRecv(SOCKET socket)
{
	st_Session* session;
	st_Character* character;
	
	Session_Type siter;
	Character_Type citer;

	int recvRet;
	int errCode;
// -----

	siter = g_sessionMap.find(socket);
	if (siter == g_sessionMap.end())
	{
		//wprintf(L"[NetworkRecv] -> Find error[%llu]\n", socket);
		return ;
	}

	// session 값
	session = siter->second;

	session->lastRecvTime = timeGetTime();

	recvRet = recv(session->socket, session->recvQ.GetRearBufferPtr(), session->recvQ.DirectEnqueueSize(), 0);
	if (recvRet == 0 || recvRet == SOCKET_ERROR)
	{
		errCode = GetLastError();
		if (errCode == WSAEWOULDBLOCK)
		{
			return;
		}
		else if (errCode == 0 || errCode == 10054)
		{
			;
		}
		else
		{
			wprintf(L"another recv errorGame socket[%llu] id[%d] code[%d]\n", socket, session->sessionID, errCode);
		}

		citer = g_characterMap.find(session->sessionID);
		if (citer == g_characterMap.end())
		{
			wprintf(L"need Test\n");
		}

		character = citer->second;
		DeleteCharacterAndSession(character);
		return;
	}
	session->recvQ.MoveRear(recvRet);
	g_recvCnt++;

	// 패킷 마샬링 작업
	while (PacketMarshall(session))
		;
}

bool CNetwork::NetworkSend(SOCKET socket)
{
	st_Session* session;
	st_Character* character;
	int errCode;

	Session_Type sIter;
	Character_Type cIter;
// -----

	sIter = g_sessionMap.find(socket);
	if (sIter == g_sessionMap.end())
	{
		wprintf(L"[NetworkSend] -> Find error\n");
		return (false);
	}
	session = sIter->second;

	int send_ret = send(socket, session->sendQ.GetFrontBufferPtr(), session->sendQ.DirectDequeueSize(), 0);
	if (send_ret == SOCKET_ERROR)
	{
		errCode = GetLastError();
		if (errCode == WSAEWOULDBLOCK)
		{
			return (false);
		}
		else if (errCode == 10054)
		{
			;
		}
		else
		{
			wprintf(L"[NetworkSend] errorGame socket[%llu] id[%d] code[%d]\n", socket, session->sessionID, errCode);
		}
		CSerialization buffer;

		CreatePacketDeleteCharacter(buffer, session->sessionID);
		SendPacketSectorAroundCast(session, &buffer);

		g_sessionMap.erase(sIter);

		cIter = g_characterMap.find(session->sessionID);
		if (cIter == g_characterMap.end())
		{
			wprintf(L"need Test\n");
		}

		
		character = cIter->second;
		g_characterMap.erase(session->sessionID);
		g_sector[character->sector.sec_y][character->sector.sec_x].remove(character);

		delete character;
		delete session;
		closesocket(socket);

		return (false);
	}
	session->sendQ.MoveFront(send_ret);
	g_sendCnt++;
	return (true);
}

void CNetwork::NetworkSelect(SOCKET* socketTable, FD_SET* readSet, FD_SET* writeSet)
{
	int socketCount;
	timeval time;
	int errCode;
	int i;

	time.tv_sec = 0;
	time.tv_usec = 0;

	
	socketCount = select(0, readSet, writeSet, nullptr, &time);
	g_selectCnt++;
	if (socketCount > 0)
	{
		for (i = 0 ; i < 64 ; ++i)
		{
			if (socketTable[i] == 0)
			{
				break;
			}

			if (FD_ISSET(socketTable[i], writeSet))
			{
				NetworkSend(socketTable[i]);
			}

			if (FD_ISSET(socketTable[i], readSet))
			{
				if (socketTable[i] == g_listenSocket)
					NetworkAccept();
				else
					NetworkRecv(socketTable[i]);
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

	for (Session_Type iter = g_sessionMap.begin(); iter != g_sessionMap.end();)
	{
		session = iter->second;

		socketTable[socketCount++] = session->socket;

		// Read는 무조건 넣기
		FD_SET(session->socket, &readSet);
		if (session->sendQ.GetUseSize() > 0)
		{
			FD_SET(session->socket, &writeSet);
		}

		++iter;

		if (FD_SETSIZE <= socketCount)
		{
			NetworkSelect(socketTable, &readSet, &writeSet);

			// fd 초기화
			FD_ZERO(&readSet);
			FD_ZERO(&writeSet);

			// socket Table 초기화
			ZeroMemory(socketTable, sizeof(socketTable));

			// listen socket 등록
			FD_SET(g_listenSocket, &readSet);
			socketTable[0] = g_listenSocket;

			socketCount = 1;
		}
	}

	if (socketCount > 0)
		NetworkSelect(socketTable, &readSet, &writeSet);
}
