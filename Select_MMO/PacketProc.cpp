#include <stdio.h>
#include <list>
#include <unordered_map>
#include <Windows.h>

#include "SerializationBuffer.h"
#include "CreatePacket.h"
#include "Session.h"
#include "Sector.h"
#include "Character.h"
#include "SendPacket.h"
#include "Util.h"
#include "PROTOCOL.h"

extern std::list<st_Character*> g_sector[6400 / 150 + 1][6400 / 150 + 1];
extern std::unordered_map<DWORD, st_Character*> g_characterMap;

void MoveStart(st_Session* session, CSerialization* packet)
{
	BYTE dir;
	short x, y;
	st_Character* character;
// -----

	*packet >> dir >> x >> y;

	character = g_characterMap[session->sessionID];
	if (character == NULL)
	{
		wprintf(L"warning\n");
		return ;
	}


	// sync pass

	character->action = dir;
	character->direction = dir;
	switch (dir)
	{
	case MOVE_DIR_RR:
	case MOVE_DIR_RU:
	case MOVE_DIR_RD:
		dir = MOVE_DIR_RR;
		break;
	case MOVE_DIR_LL:
	case MOVE_DIR_LU:
	case MOVE_DIR_LD:
		dir = MOVE_DIR_LL;
		break;

	default:
		break;

	}
	character->direction = dir;
	character->x = x;
	character->y = y;
	//Update
	printf("POS: %d %d dir: %d\n", x, y, dir);
	packet->Clear();
	CreatePacketMoveStart(*packet, session->sessionID, dir, x, y);
	SendPacketSectorAroundCast(session, packet);
}

void MoveStop(st_Session* session, CSerialization* packet)
{
	BYTE dir;
	short x, y;
	st_Character* character;
// -----

	*packet >> dir >> x >> y;

	character = g_characterMap[session->sessionID];
	if (character == NULL)
	{
		wprintf(L"warning\n");
		return ;
	}

	// sync pass
	character->action = 8;
	character->direction = dir;
	character->x = x;
	character->y = y;
	//Update
	packet->Clear();
	CreatePacketMoveStop(*packet, session->sessionID, dir, x, y);
	SendPacketSectorAroundCast(session, packet);
}


void Attack1Packet(st_Session* session, CSerialization* packet)
{
	BYTE dir;
	short x;
	short y;
	st_Character* character;
	st_Character* victim;
	st_SECTOR_AROUND aroundSector;
// -----

	*packet >> dir >> x >> y;

	character = g_characterMap[session->sessionID];

	// nullptr || dir Error
	if (character == nullptr)
	{
		// 예외 처리
		wprintf(L"[Attack1] -> nullptr\n");
		return ;
	}


	packet->Clear();
	CreatePacketAttack1(*packet, character->characterId, character->direction, character->x, character->y);
	SendPacketSectorAroundCast(session, packet);

	character->sector.sec_y = character->y / 150;
	character->sector.sec_x = character->x / 150;

	short now_x = character->x;
	short now_y = character->y;

	short sec_y;
	short sec_x;


	// 데미지 먹이는 곳. 

	sec_y = now_y / 150;
	sec_x = now_x / 150;

	GetAroundSector(sec_y, sec_x, &aroundSector);
	if (dir == MOVE_DIR_LL)
	{
		for (int i = 0; i < aroundSector.count; ++i)
		{
			sec_y = aroundSector.around->sec_y;
			sec_x = aroundSector.around->sec_x;

			for (auto it = g_sector[sec_y][sec_x].begin(); it != g_sector[sec_y][sec_x].end(); it++)
			{
				victim = *it;
				if ((now_y - dfATTACK1_RANGE_Y) <= victim->y && victim->y <= now_y &&
					(now_x - dfATTACK1_RANGE_X) <= victim->x && victim->x <= now_x)
				{
					victim->hp = max(0, (int)victim->hp - dfATTACK1_DAMAGE);
					packet->Clear();
					CreatePacketDamage(*packet, character->characterId, victim->characterId, victim->hp);

					SendPacketSectorAroundCast(victim->session, packet, true);
					
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < aroundSector.count; ++i)
		{
			sec_y = aroundSector.around->sec_y;
			sec_x = aroundSector.around->sec_x;

			for (auto it = g_sector[sec_y][sec_x].begin(); it != g_sector[sec_y][sec_x].end(); it++)
			{
				victim = *it;
				if ((now_y - dfATTACK1_RANGE_Y) <= victim->y && victim->y <= now_y &&
					(now_x + dfATTACK1_RANGE_X) >= victim->x && victim->x >= now_x)
				{
					packet->Clear();
					victim->hp = max(0, (int)victim->hp - dfATTACK1_DAMAGE);
					CreatePacketDamage(*packet, character->characterId, victim->characterId, victim->hp);

					SendPacketSectorAroundCast(victim->session, packet, true);
				}
			}
		}
	}
}

void Echo(st_Session* session, CSerialization* packet)
{
	DWORD time;
// -----
	*packet >> time;

	packet->Clear();
	CreatePacketEcho(*packet, time);
	SendPacketUniCast(session, packet);
}
