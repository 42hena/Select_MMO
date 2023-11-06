#include <stdio.h>
#include <list>
#include <unordered_map>
#include <time.h>
#include <Windows.h>

#include "SerializationBuffer.h"
#include "CreatePacket.h"
#include "Session.h"
#include "Sector.h"
#include "Character.h"
#include "PacketProc.h"
#include "SendPacket.h"
#include "PROTOCOL.h"

extern std::unordered_map<SOCKET, st_Session*> g_sessionMap;
extern std::list<st_Character*> g_sector[RANGE_MOVE_BOTTOM / SECTOR_MAX_Y][RANGE_MOVE_RIGHT / SECTOR_MAX_X];
extern std::unordered_map<DWORD, st_Character*> g_characterMap;
extern DWORD g_frameTime, totalTime;
extern DWORD g_sendCnt, g_recvCnt, g_acceptCnt;
extern DWORD g_whileCnt, g_selectCnt;
extern DWORD g_minFrame, g_maxFrame, g_avgFrame, g_prevFrame;
extern DWORD g_syncCnt;
extern DWORD Log;
extern DWORD frame;
extern DWORD prevFrame;

#define Charcter_Type std::unordered_map<DWORD, st_Character*>::iterator

st_Character* FindCharacter(DWORD sessionID)
{
	Charcter_Type cIter;
	// -----

	cIter = g_characterMap.find(sessionID);
	if (cIter != g_characterMap.end())
		return (cIter->second);
	return (nullptr);
}
void DeleteCharacterAndSession(st_Character * character)
{
	CSerialization buffer;
	SOCKET sock;
// -----

	sock = character->session->socket;
	CreatePacketDeleteCharacter(buffer, character->session->sessionID);
	SendPacketSectorAroundCast(character->session, &buffer);
	g_sessionMap.erase(sock);
	g_characterMap.erase(character->session->sessionID);
	g_sector[character->sector.sec_y][character->sector.sec_x].remove(character);
	delete character->session;
	delete character;
	closesocket(sock);
}


void GetAroundSector(int secY, int secX, st_SECTOR_AROUND* around)
{
	int i;
	int j;
	// -----

	--secY;
	--secX;

	for (i = 0; i < 3; ++i)
	{
		if (secY + i < 0 || secY + i >= (RANGE_MOVE_BOTTOM / SECTOR_MAX_Y))
			continue;
		for (j = 0; j < 3; ++j)
		{
			if (secX + j < 0 || secX + j >= (RANGE_MOVE_RIGHT / SECTOR_MAX_X))
				continue;
			around->around[around->count].sec_y = secY + i;
			around->around[around->count].sec_x = secX + j;
			++around->count;
		}
	}
}

void PushCreateMyCharacterJob(st_Session *session, DWORD id, BYTE dir, short x, short y, BYTE hp)
{
	CSerialization packet;

	CreatePacketCreateMyCharacter(packet, id, dir, x, y, hp);

	SendPacketUniCast(session, &packet);
}

void PushCreateOtherCharacterToMeJob(st_Session* session, DWORD id, BYTE dir, short x, short y, BYTE hp)
{
	st_Character* character;
	st_Character* myCharacter;
	st_SECTOR_AROUND mySector;
	int secY;
	int secX;

	CSerialization packet;

	myCharacter = g_characterMap[session->sessionID];
	

	GetAroundSector(myCharacter->sector.sec_y, myCharacter->sector.sec_x, &mySector);

	for (int i = 0; i < mySector.count; ++i)
	{
		secY = mySector.around[i].sec_y;
		secX = mySector.around[i].sec_x;
		for (auto iter = g_sector[secY][secX].begin(); iter != g_sector[secY][secX].end(); ++iter)
		{

			character = *iter;
			if (character != myCharacter)
			{
				CreatePacketCreateOtherCharacter(packet, character->characterId, character->direction, character->x, character->y, character->hp);
				SendPacketUniCast(session, &packet);
				packet.Clear();
			}
		}
	}
}

void PushCreateMyCharacterToOthersJob(st_Session* session, DWORD id, BYTE dir, short x, short y, BYTE hp)
{
	CSerialization packet;

	CreatePacketCreateOtherCharacter(packet, id, dir, x, y, hp);

	SendPacketSectorAroundCast(session, &packet);
}

void PushCreateOtherMoveStartTomeJob(st_Session* session, DWORD id, BYTE dir, short x, short y, BYTE hp)
{
	CSerialization packet;

	st_Character* myCharacter;
	st_SECTOR_AROUND mySector;
	st_Character* character;
	
	myCharacter = g_characterMap[session->sessionID];
	int secY;
	int secX;

	GetAroundSector(myCharacter->sector.sec_y, myCharacter->sector.sec_x, &mySector);

	for (int i = 0; i < mySector.count; ++i)
	{
		secY = mySector.around[i].sec_y;
		secX = mySector.around[i].sec_x;
		for (auto iter = g_sector[secY][secX].begin(); iter != g_sector[secY][secX].end(); ++iter)
		{
			character = *iter;
			if (character->action != 8)
			{
				CreatePacketMoveStart(packet, character->characterId, character->action, character->x, character->y);
				SendPacketUniCast(session, &packet);
				packet.Clear();
			}
		}
	}
}

void PacketProc(st_Session* session, CSerialization* packet)
{
	SHORT type;
// -----

	*packet >> type;
	
	switch (type)
	{
	case CLIENT_TO_SERVER_MOVE_START:
	{
		MoveStart(session, packet);
		break;
	}
	case CLIENT_TO_SERVER_MOVE_STOP:
	{
		MoveStop(session, packet);
		break;
	}
	case CLIENT_TO_SERVER_ATTACK1:
	case CLIENT_TO_SERVER_ATTACK2:
	case CLIENT_TO_SERVER_ATTACK3:
	{
		Attack1Packet(session, packet);
		break;
	}
	case CLIENT_TO_SERVER_ECHO:
	{
		Echo(session, packet);
		break;
	}
	}
}

bool PacketMarshall(st_Session* session)
{
	const int HEADER_SIZE = 2;
	int recvSize;
	CSerialization packet;
	int peekRet;

	// Header
	SHORT size;
	// Contents

// -----

	recvSize = session->recvQ.GetUseSize();
	if (HEADER_SIZE > recvSize)
		return (false);

	peekRet = session->recvQ.Peek(packet.GetBufferPtr(), HEADER_SIZE);

	packet >> size; 
	if (size + HEADER_SIZE > session->recvQ.GetUseSize())
	{
		return (false);
	}

	session->recvQ.MoveFront(HEADER_SIZE);
	session->recvQ.Dequeue(packet.GetBufferPtr() + HEADER_SIZE, size);

	PacketProc(session, &packet);
	return (true);
}

// -------
void AddCharacterToSector(st_Character* character)
{
	int sec_y = character->sector.sec_y;
	int sec_x = character->sector.sec_x;

	g_sector[sec_y][sec_x].push_back(character);
}


void RemoveCharacterFromSector(st_Character* character)
{
	int sec_y = character->prevSector.sec_y;
	int sec_x = character->prevSector.sec_x;

	
	for (std::list<st_Character*>::iterator it = g_sector[sec_y][sec_x].begin();
		it != g_sector[sec_y][sec_x].end();
		++it)
	{
		if ((*it) == character)
		{
			g_sector[sec_y][sec_x].erase(it);
			break;
		}
	}
}

bool IsCharacterSectorUpdate(st_Character *character)
{
	if (character->sector != character->prevSector)
	{
		AddCharacterToSector(character);
		RemoveCharacterFromSector(character);
		return (true);
	}

	return (false);
}

void	GetUpdateSectorAround
(st_Character* character, st_SECTOR_AROUND* remove_sector, st_SECTOR_AROUND* add_sector)
{
	st_SECTOR_AROUND prevSector, curSector;
	int prev, cur;
	bool find;

	// cur old sector
	GetAroundSector(character->prevSector.sec_y, character->prevSector.sec_x, &prevSector);
	GetAroundSector(character->sector.sec_y, character->sector.sec_x, &curSector);

	for (prev = 0; prev < prevSector.count; ++prev)
	{
		find = false;
		for (cur = 0; cur < curSector.count; ++cur)
		{
			if (prevSector.around[prev].sec_x == curSector.around[cur].sec_x &&
				prevSector.around[prev].sec_y == curSector.around[cur].sec_y)
			{
				find = true;
				break;
			}
		}
		if (find == false)
		{
			remove_sector->around[remove_sector->count] = prevSector.around[prev];
			remove_sector->count++;
		}
	}

	for (cur = 0; cur < curSector.count; ++cur)
	{
		find = false;
		for (prev = 0; prev < prevSector.count; ++prev)
		{
			if (prevSector.around[prev].sec_x == curSector.around[cur].sec_x &&
				prevSector.around[prev].sec_y == curSector.around[cur].sec_y)
			{
				find = true;
				break;
			}
		}
		if (find == false)
		{
			add_sector->around[add_sector->count] = curSector.around[cur];
			add_sector->count++;
		}
	}
}


void CharacterSectorUpdatePacket(st_Character* character)
{
	st_SECTOR_AROUND deleteSector, newSector;
	st_Character* exist_character;

	std::list<st_Character*>* sectorList;
	std::list<st_Character*>::iterator liter;

	CSerialization buffer;
	int i;
	// Get remove add sector 
	GetUpdateSectorAround(character, &deleteSector, &newSector);

	// remove sector - delete Character
	CreatePacketDeleteCharacter(buffer, character->characterId);
	
	// my info -> remove sector
	for (i = 0; i < deleteSector.count; ++i)
	{
		SendPacketSector(deleteSector.around[i].sec_y, deleteSector.around[i].sec_x, &buffer, NULL);
	}
	buffer.Clear();

	// remove sector info -> me
	for (i = 0; i < deleteSector.count; ++i)
	{
		sectorList = &g_sector[deleteSector.around[i].sec_y][deleteSector.around[i].sec_x];

		for (liter = sectorList->begin() ; liter != sectorList->end(); ++liter)
		{
			CreatePacketDeleteCharacter(buffer, (*liter)->characterId);
			SendPacketUniCast(character->session, &buffer);
			buffer.Clear();
		}
	}

	// add sector - create Character and MoveStart
	CreatePacketCreateOtherCharacter(buffer, character->characterId, character->direction, character->x, character->y, character->hp);

	// my info -> add sector
	for (i = 0; i < newSector.count; ++i)
	{
		SendPacketSector(newSector.around[i].sec_y, newSector.around[i].sec_x, &buffer, NULL);
	}
	buffer.Clear();

	// add sector -> me
	CreatePacketMoveStart(buffer, character->characterId, character->action, character->x, character->y);
	for (i = 0; i < newSector.count; ++i)
	{
		SendPacketSector(newSector.around[i].sec_y, newSector.around[i].sec_x, &buffer, NULL);
	}
	buffer.Clear();


	// 4
	for (i = 0; i < newSector.count; ++i)
	{

		sectorList = &g_sector[newSector.around[i].sec_y][newSector.around[i].sec_x];

		for (liter = sectorList->begin(); liter != sectorList->end(); ++liter)
		{
			exist_character = *liter;

			if (exist_character != character)
			{
				CreatePacketCreateOtherCharacter(buffer, exist_character->characterId, exist_character->direction, exist_character->x, exist_character->y, exist_character->hp);
				
				if (character->action <= 7)
				{
					CreatePacketMoveStart(buffer, exist_character->characterId, exist_character->action, exist_character->x, exist_character->y);
				}
				SendPacketUniCast(character->session, &buffer);
				buffer.Clear();
			}
		}
	}
}

bool CheckCharacterMove(int y, int x)
{
	if (y < 0 || y >= RANGE_MOVE_BOTTOM || x < 0 || x >= RANGE_MOVE_RIGHT)
		return (false);
	return (true);
}

void Update()
{
	st_Character* character;
	DWORD now;
	DWORD frameTime;
// -----

	now = timeGetTime();
	totalTime += now - g_frameTime;
	Log += now - g_frameTime;;
	g_frameTime = now;
	while (totalTime >= 40)
	{
		now = timeGetTime();
		frameTime = now - g_prevFrame;
		g_minFrame = min(g_minFrame, frameTime);
		g_maxFrame = max(g_maxFrame, frameTime);
		g_avgFrame += frameTime;
		g_prevFrame = now;
		frame++;
		totalTime -= 40;
		for (Charcter_Type cIter = g_characterMap.begin(); cIter != g_characterMap.end(); )
		{
			character = cIter->second;
			++cIter;
			if (character->hp <= 0)
			{
				DeleteCharacterAndSession(character);
			}
			else
			{
				if (timeGetTime() - character->session->lastRecvTime > ERROR_NETWORK_PACKET_RECV_TIMEOUT)
				{
					DeleteCharacterAndSession(character);
					continue;
				}
				switch (character->action)
				{
				case MOVE_DIR_LL:
				{
					if (CheckCharacterMove(character->y, character->x - SPEED_PLAYER_X))
					{
						character->x -= SPEED_PLAYER_X;
					}
					break;
				}
				case MOVE_DIR_LU:
				{
					if (CheckCharacterMove(character->y - SPEED_PLAYER_Y, character->x - SPEED_PLAYER_X))
					{
						character->y -= SPEED_PLAYER_Y;
						character->x -= SPEED_PLAYER_X;
					}
					break;
				}
				case MOVE_DIR_UU:
				{
					if (CheckCharacterMove(character->y - SPEED_PLAYER_Y, character->x))
					{
						character->y -= SPEED_PLAYER_Y;
					}
					break;
				}
				case MOVE_DIR_RU:
				{
					if (CheckCharacterMove(character->y - SPEED_PLAYER_Y, character->x + SPEED_PLAYER_X))
					{
						character->y -= SPEED_PLAYER_Y;
						character->x += SPEED_PLAYER_X;
					}
					break;
				}

				case MOVE_DIR_RR:
				{
					if (CheckCharacterMove(character->y, character->x + SPEED_PLAYER_X))
					{
						character->x += SPEED_PLAYER_X;
					}
					break;
				}

				case MOVE_DIR_RD:
				{
					if (CheckCharacterMove(character->y + SPEED_PLAYER_Y, character->x + SPEED_PLAYER_X))
					{
						character->y += SPEED_PLAYER_Y;
						character->x += SPEED_PLAYER_X;
					}
					break;
				}

				case MOVE_DIR_DD:
				{
					if (CheckCharacterMove(character->y + SPEED_PLAYER_Y, character->x))
					{
						character->y += SPEED_PLAYER_Y;
					}
					break;
				}

				case MOVE_DIR_LD:
				{
					if (CheckCharacterMove(character->y + SPEED_PLAYER_Y, character->x - SPEED_PLAYER_X))
					{
						character->y += SPEED_PLAYER_Y;
						character->x -= SPEED_PLAYER_X;
					}
					break;
				}
				}// SWITCH문
				if (character->action <= MOVE_DIR_LD)
				{
					character->sector.sec_x = character->x / SECTOR_MAX_X;
					character->sector.sec_y = character->y / SECTOR_MAX_Y;
					if (IsCharacterSectorUpdate(character))
					{
						CharacterSectorUpdatePacket(character);
					}
				}
				character->prevSector = character->sector;
			}
		}
	}
}

void PrintLog()
{
	FILE* fp;
	WCHAR LogBuf[512];
	int sectorTotal;
	int i, j;
	time_t curTime;
	struct tm pLocal;
// -----

	if (Log >= 1000)
	{
		fopen_s(&fp, "Select_MMO_LOG.txt", "ab+");
		if (fp == nullptr)
		{
			wprintf(L"can't open file\n");
			return;
		}
		sectorTotal = 0;
		Log -= 1000;
		for (i = 0; i < RANGE_MOVE_BOTTOM / SECTOR_MAX_Y; ++i)
		{
			for (j = 0; j < RANGE_MOVE_RIGHT / SECTOR_MAX_X; ++j)
			{
				sectorTotal += static_cast<int>(g_sector[i][j].size());
			}
		}

		curTime = time(NULL);

		// Convert the current time 
		localtime_s(&pLocal, &curTime);

		// 문자열로 치환 및 파일 쓰기
		wsprintf(LogBuf, L"%04d-%02d-%02d %02d:%02d:%02d\nSession cnt: %d\nCharacter cnt: %d\nsector: %d\nselect:%d acc:%d recv:%d send:%d\n"
			"Server Frame: %d/%d Avg Frame: %d Min Frame: %d Max Frame: %d\n"
			"Sync count: %d\n\n", pLocal.tm_year + 1900, pLocal.tm_mon + 1, pLocal.tm_mday,
			pLocal.tm_hour, pLocal.tm_min, pLocal.tm_sec,
			static_cast<int>(g_sessionMap.size()), static_cast<int>(g_characterMap.size()), sectorTotal, g_selectCnt, g_acceptCnt, g_recvCnt, g_sendCnt,
			frame, g_whileCnt, g_avgFrame / frame, g_minFrame, g_maxFrame,
			g_syncCnt);
		wprintf(L"%s", LogBuf);
		fwrite(LogBuf, 2, wcslen(LogBuf), fp);

		// 초기화
		frame = 0;
		g_selectCnt = g_whileCnt = g_acceptCnt = g_recvCnt = g_sendCnt = 0;
		g_minFrame = 1000000000;
		g_avgFrame = g_maxFrame = 0;
		fclose(fp);
	}
}