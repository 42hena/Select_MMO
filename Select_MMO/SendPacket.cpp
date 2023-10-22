#include <WinSock2.h>
#include <iostream>
#include <list>
#include <unordered_map>

#include "SerializationBuffer.h"
#include "PROTOCOL.h"
#include "Character.h"
#include "limits.h"

extern std::list<st_Character*> g_sector[6400 / 150 + 1][6400 / 150 + 1];
extern std::unordered_map<DWORD, st_Character*> g_characterMap;
extern std::unordered_map<SOCKET, st_Session*> g_sessionMap;

#define Character_Type std::unordered_map<DWORD, st_Character*>::iterator
#define Sector_Type std::list<st_Character*>::iterator
#define Session_Type std::unordered_map<SOCKET, st_Session*>::iterator

void SendPacketUniCast(st_Session* session, CSerialization *packet)
{
	int enqueueSize;
	int msgSize;
// -----

	msgSize = packet->GetDataSize();
	enqueueSize = session->sendQ.Enqueue(packet->GetBufferPtr(), msgSize);
	if (enqueueSize != msgSize)
	{
		wprintf(L"UniCast Error\n");
		// TODO:: 삭제.
	}
}

void SendPacketSector(int secY, int secX, CSerialization* packet, st_Character* exception)
{
	Character_Type citer;
	Sector_Type siter;
	st_Character* character;
	std::list<st_Character*> sectorList;
	
// -----

	// 원하는 위치의 sector List 검색
	sectorList = g_sector[secY][secX];

	// List를 순회하며, 케릭터에게 보냄.
	for (siter = sectorList.begin() ; siter != sectorList.end() ;)
	{
		character = *siter;
		++siter;
		if (character != exception)
		{
			SendPacketUniCast(character->session, packet);
		}
	}
}

void GetAroundSector(int secY, int secX, st_SECTOR_AROUND *around)
{
	int i;
	int j;
// -----

	--secY;
	--secX;

	for (i = 0; i < 3; ++i)
	{
		if (secY + i < 0 || secY + i >= (6400 / 150 + 1))
			continue ;
		for (j = 0; j < 3; ++j)
		{
			if (secX + j < 0 || secX + j >= (6400 / 150 + 1))
				continue ;
			around->around[around->count].sec_y = secY + i;
			around->around[around->count].sec_x = secX + j;
			++around->count;
		}
	}
}

void SendPacketSectorAroundCast(st_Session* session, CSerialization* packet, bool sendMe = false)
{
	Character_Type citer;
	st_Character* character;
	st_SECTOR_AROUND aroundSector;
	int i;
// -----

	citer = g_characterMap.find(session->sessionID);
	if (citer == g_characterMap.end())
	{
		wprintf(L"SendPacket_SectorAroundCast ?????\n");
		return;
	}

	// 케릭터 지정
	character = citer->second;
	GetAroundSector(character->sector.sec_y, character->sector.sec_x, &aroundSector);

	for (i = 0 ; i < aroundSector.count ; ++i)
	{
		SendPacketSector(aroundSector.around[i].sec_y, aroundSector.around[i].sec_x, packet, character);
	}
}

void SendPacketBroadCast(st_Session* session, CSerialization *packet)
{
	Session_Type siter;
// -----

	for (siter = g_sessionMap.begin() ; siter != g_sessionMap.end() ; ++siter)
	{
		SendPacketUniCast(siter->second, packet);
	}
}
