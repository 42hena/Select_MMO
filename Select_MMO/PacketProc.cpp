#include <stdio.h>
#include <list>
#include <unordered_map>
#include <Windows.h>

#include "PROTOCOL.h"
#include "SerializationBuffer.h"
#include "CreatePacket.h"
#include "Session.h"
#include "Sector.h"
#include "Character.h"
#include "SendPacket.h"
#include "Util.h"

extern std::list<st_Character*> g_sector[RANGE_MOVE_BOTTOM / SECTOR_MAX_Y + 1][RANGE_MOVE_RIGHT / SECTOR_MAX_X + 1];
extern std::unordered_map<DWORD, st_Character*> g_characterMap;

void MoveStart(st_Session* session, CSerialization* packet)
{
	BYTE dir;
	short x, y;
	st_Character* character;
// -----

	*packet >> dir >> x >> y;

	character = FindCharacter(session->sessionID);
	if (character == nullptr)
	{
		wprintf(L"character not Found\n");
		return ;
	}


	// sync pass
	if (abs(character->x - x) > ERROR_RANGE || abs(character->y - y) > ERROR_RANGE)
	{
		packet->Clear();
		x = character->x;
		y = character->y;
		CreatePacketSync(*packet, character->characterId, x, y);
		SendPacketSectorAroundCast(session, packet, true);
	}


	character->action = dir;
	switch (dir)
	{
	case MOVE_DIR_RR:
	case MOVE_DIR_RU:
	case MOVE_DIR_RD:
		character->direction = MOVE_DIR_RR;
		break;
	case MOVE_DIR_LL:
	case MOVE_DIR_LU:
	case MOVE_DIR_LD:
		character->direction = MOVE_DIR_LL;
		break;

	default:
		break;

	}
	character->x = x;
	character->y = y;
	character->sector.sec_x = x / SECTOR_MAX_X;
	character->sector.sec_y = y / SECTOR_MAX_Y;
	if (IsCharacterSectorUpdate(character))
	{
		CharacterSectorUpdatePacket(character);
	}
	character->prevSector = character->sector;
	
	//Update
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

	// packet에서 데이터 뽑기(방향, x좌표, y좌표)
	*packet >> dir >> x >> y;

	// 케릭터 존재 검색
	character = FindCharacter(session->sessionID);
	if (character == nullptr)
	{
		wprintf(L"character not Found\n");
		return;
	}

	// 서버와 클라이언트 싱크 맞추기.
	if (abs(character->x - x) > ERROR_RANGE || abs(character->y - y) > ERROR_RANGE)
	{
		packet->Clear();
		x = character->x;
		y = character->y;
		CreatePacketSync(*packet, character->characterId, x, y);
		SendPacketSectorAroundCast(session, packet, true);
	}


	character->action = 8;
	switch (dir)
	{
	case MOVE_DIR_LL:
	case MOVE_DIR_LU:
	case MOVE_DIR_LD:
		character->direction = MOVE_DIR_LL;
		break;
	case MOVE_DIR_RR:
	case MOVE_DIR_RU:
	case MOVE_DIR_RD:
		character->direction = MOVE_DIR_RR;
		break;
	}
	character->x = x;
	character->y = y;
	character->sector.sec_x = x / SECTOR_MAX_X;
	character->sector.sec_y = y / SECTOR_MAX_Y;
	if (IsCharacterSectorUpdate(character))
	{
		CharacterSectorUpdatePacket(character);
	}
	character->prevSector = character->sector;
	
	//Update
	packet->Clear();
	CreatePacketMoveStop(*packet, session->sessionID, dir, x, y);
	SendPacketSectorAroundCast(session, packet);
}


void Attack1Packet(st_Session* session, CSerialization* packet)
{
	st_Character* victim;
	st_Character* character;
	st_SECTOR_AROUND aroundSector;

	// 패킷의 임시 변수
	BYTE dir;
	short x;
	short y;


	short secY;
	short secX;
	int nowY;
	int nowX;
// -----

	*packet >> dir >> x >> y;
	character = FindCharacter(session->sessionID);
	if (character == nullptr)
	{
		wprintf(L"character not Found\n");
		return;
	}

	// 서버와 클라이언트 싱크 맞추기.
	if (abs(character->x - x) > ERROR_RANGE || abs(character->y - y) > ERROR_RANGE)
	{
		packet->Clear();
		x = character->x;
		y = character->y;
		CreatePacketSync(*packet, character->characterId, x, y);
		SendPacketSectorAroundCast(session, packet, true);
	}

	packet->Clear();
	CreatePacketAttack1(*packet, character->characterId, character->direction, character->x, character->y);
	SendPacketSectorAroundCast(session, packet);

	character->y = y;
	character->x = x;
	character->sector.sec_y = character->y / SECTOR_MAX_Y;
	character->sector.sec_x = character->x / SECTOR_MAX_X;

	nowX = character->x;
	nowY = character->y;

	


	// 데미지 먹이는 곳. 
	secY = nowY / SECTOR_MAX_Y;
	secX = nowX / SECTOR_MAX_X;

	GetAroundSector(secY, secX, &aroundSector);
	if (dir == MOVE_DIR_LL)
	{
		for (int i = 0; i < aroundSector.count; ++i)
		{
			secY = aroundSector.around[i].sec_y;
			secX = aroundSector.around[i].sec_x;

			for (auto it = g_sector[secY][secX].begin(); it != g_sector[secY][secX].end(); it++)
			{
				victim = *it;
				if ((nowY- ATTACK1_RANGE_Y) <= victim->y && victim->y <= nowY &&
					(nowX - ATTACK1_RANGE_X) <= victim->x && victim->x < nowX)
				{
					victim->hp = max(0, (int)victim->hp - ATTACK1_DAMAGE);
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
			secY = aroundSector.around[i].sec_y;
			secX = aroundSector.around[i].sec_x;

			for (auto it = g_sector[secY][secX].begin(); it != g_sector[secY][secX].end(); it++)
			{
				victim = *it;
				if ((nowY - ATTACK1_RANGE_Y) <= victim->y && victim->y <= nowY &&
					(nowX + ATTACK1_RANGE_X) >= victim->x && victim->x > nowX)
				{
					packet->Clear();
					victim->hp = max(0, (int)victim->hp - ATTACK1_DAMAGE);
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

	// packet에서 데이터 뽑기(time)
	*packet >> time;

	// packet을 재사용하기 위해 초기화
	packet->Clear();

	// Echo 패킷 만들기
	CreatePacketEcho(*packet, time);

	// 자신에게 전송 요청
	SendPacketUniCast(session, packet);
}
