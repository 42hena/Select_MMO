#include <WinSock2.h>
#include <iostream>
#include <list>
#include <unordered_map>

#include "PROTOCOL.h"
#include "SerializationBuffer.h"
#include "Session.h"
#include "Character.h"
#include "Util.h"

extern std::list<st_Character*> g_sector[RANGE_MOVE_BOTTOM / SECTOR_MAX_Y][RANGE_MOVE_RIGHT / SECTOR_MAX_X];
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
		// TODO:: ����.
	}
}

void SendPacketSector(int secY, int secX, CSerialization* packet, st_Character* exception)
{
	Character_Type citer;
	Sector_Type siter;
	st_Character* character;
	std::list<st_Character*> sectorList;
// -----

	// ���ϴ� ��ġ�� sector List �˻�
	sectorList = g_sector[secY][secX];

	// List�� ��ȸ�ϸ�, �ɸ��Ϳ��� ����.
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

void SendPacketSectorAroundCast(st_Session* session, CSerialization* packet, bool sendMe)
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

	// �ɸ��� ����
	character = citer->second;
	GetAroundSector(character->sector.sec_y, character->sector.sec_x, &aroundSector);

	if (sendMe)
	{
		for (i = 0; i < aroundSector.count; ++i)
		{
			SendPacketSector(aroundSector.around[i].sec_y, aroundSector.around[i].sec_x, packet, nullptr);
		}
	}
	else
	{
		for (i = 0; i < aroundSector.count; ++i)
		{
			SendPacketSector(aroundSector.around[i].sec_y, aroundSector.around[i].sec_x, packet, character);
		}
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
