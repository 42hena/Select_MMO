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
	character->sector.sec_x = x / 150;
	character->sector.sec_y = y / 150;
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

	// packet���� ������ �̱�(����, x��ǥ, y��ǥ)
	*packet >> dir >> x >> y;

	// �ɸ��� ���� �˻�
	character = g_characterMap[session->sessionID];
	if (character == NULL)
	{
		wprintf(L"warning\n"); // TODO: �α׸� ���Ϸ� ������ ��.
		return ;
	}

	// ������ Ŭ���̾�Ʈ ��ũ ���߱�.
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
	character->sector.sec_x = x / 150;
	character->sector.sec_y = y / 150;
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

	// ��Ŷ�� �ӽ� ����
	BYTE dir;
	short x;
	short y;


	short secY;
	short secX;
	int nowY;
	int nowX;
// -----

	*packet >> dir >> x >> y;
	character = g_characterMap[session->sessionID];

	// nullptr || dir Error
	if (character == nullptr)
	{
		// ���� ó��
		wprintf(L"[Attack1] -> nullptr\n");
		return ;
	}

	// ������ Ŭ���̾�Ʈ ��ũ ���߱�.
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
	character->sector.sec_y = character->y / 150;
	character->sector.sec_x = character->x / 150;

	nowX = character->x;
	nowY = character->y;

	


	// ������ ���̴� ��. 
	secY = nowY / 150;
	secX = nowX / 150;

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
				if ((nowY- dfATTACK1_RANGE_Y) <= victim->y && victim->y <= nowY &&
					(nowX - dfATTACK1_RANGE_X) <= victim->x && victim->x < nowX)
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
			secY = aroundSector.around[i].sec_y;
			secX = aroundSector.around[i].sec_x;

			for (auto it = g_sector[secY][secX].begin(); it != g_sector[secY][secX].end(); it++)
			{
				victim = *it;
				if ((nowY - dfATTACK1_RANGE_Y) <= victim->y && victim->y <= nowY &&
					(nowX + dfATTACK1_RANGE_X) >= victim->x && victim->x > nowX)
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

	// packet���� ������ �̱�(time)
	*packet >> time;

	// packet�� �����ϱ� ���� �ʱ�ȭ
	packet->Clear();

	// Echo ��Ŷ �����
	CreatePacketEcho(*packet, time);

	// �ڽſ��� ���� ��û
	SendPacketUniCast(session, packet);
}
