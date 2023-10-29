#include <stdio.h>
#include <list>
#include <unordered_map>
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
extern std::list<st_Character*> g_sector[6400 / 150 + 1][6400 / 150 + 1];
extern std::unordered_map<DWORD, st_Character*> g_characterMap;
extern std::list< st_Character* > g_sector[6400 / 150 + 1][6400 / 150 + 1];
extern DWORD g_frameTime, totalTime;

#define Charcter_Type std::unordered_map<DWORD, st_Character*>::iterator

void GetAroundSector(int secY, int secX, st_SECTOR_AROUND* around)
{
	int i;
	int j;
	// -----

	--secY;
	--secX;

	for (i = 0; i < 3; ++i)
	{
		if (secY + i < 0 || secY + i >= (6400 / 150 + 1))
			continue;
		for (j = 0; j < 3; ++j)
		{
			if (secX + j < 0 || secX + j >= (6400 / 150 + 1))
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
	if (y < 0 || y > 6400 || x < 0 || x > 6400)
		return (false);
	return (true);
}
DWORD Log;
DWORD frame;
void Update()
{
	st_Character* character;
	SOCKET sock;
	DWORD now;
// -----

	now = timeGetTime();
	totalTime += now - g_frameTime;
	Log += now - g_frameTime;;
	g_frameTime = now;
	if (totalTime < 40)
		return;
	frame++;
	totalTime -= 40;
	for (Charcter_Type cIter = g_characterMap.begin(); cIter != g_characterMap.end(); )
	{
		character = cIter->second;
		++cIter;
		if (character->hp <= 0)
		{
			CSerialization buffer;
			sock = character->session->socket;
			//printf("hp socket:[%llu] sid:[%d] cid:[%d]\n", sock, character->session->sessionID, character->characterId);
			CreatePacketDeleteCharacter(buffer, character->session->sessionID);
			SendPacketSectorAroundCast(character->session, &buffer);
			g_sessionMap.erase(sock);
			g_characterMap.erase(character->session->sessionID);
			g_sector[character->sector.sec_y][character->sector.sec_x].remove(character);
			delete character->session;
			delete character;
			closesocket(sock);
		}
		else
		{
			if (timeGetTime() - character->session->lastRecvTime > dfNETWORK_PACKET_RECV_TIMEOUT)
			{
				CSerialization buffer;
				sock = character->session->socket;
				//printf("Time socket:[%llu] sid:[%d] cid:[%d]\n", sock, character->session->sessionID, character->characterId);
				CreatePacketDeleteCharacter(buffer, character->session->sessionID);
				SendPacketSectorAroundCast(character->session, &buffer);
				g_sessionMap.erase(sock);
				g_characterMap.erase(character->session->sessionID);
				g_sector[character->sector.sec_y][character->sector.sec_x].remove(character);
				delete character->session;
				delete character;
				closesocket(sock);
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
			}// SWITCH¹®
			if (character->action <= MOVE_DIR_LD)
			{
				character->sector.sec_x = character->x / 150;
				character->sector.sec_y = character->y / 150;
				if (IsCharacterSectorUpdate(character))
				{
					CharacterSectorUpdatePacket(character);
				}
			}
			character->prevSector = character->sector;
		}
	}
}

#include <time.h>
void PrintLog()
{
	FILE* fp;
	char LogBuf[512];

	if (Log >= 1000)
	{
		fopen_s(&fp, "Select_MMO_LOG.txt", "ab+");
		int cnt = 0;
		Log -= 1000;
		for (int i = 0; i < 6400 / 150 + 1; ++i)
		{
			for (int j = 0; j < 6400 / 150 + 1; ++j)
			{
				cnt += g_sector[i][j].size();
			}
		}


		time_t curTime = time(NULL);

		// Convert the current time 
		struct tm pLocal;
#if defined(_WIN32) || defined(_WIN64) 
		localtime_s(&pLocal, &curTime);
#else 
		localtime_r(&curTime, pLocal);
#endif 
		sprintf_s(LogBuf, "%04d-%02d-%02dT%02d:%02d:%02d\n", pLocal.tm_year + 1900, pLocal.tm_mon + 1, pLocal.tm_mday,
			pLocal.tm_hour, pLocal.tm_min, pLocal.tm_sec);
		printf("%s", LogBuf);
		fwrite(LogBuf, 1, strlen(LogBuf), fp);
		// Print the current time 

		sprintf_s(LogBuf, "Server Frame: %d Session cnt: %d  character cnt: %d sector: %d\n", 
			frame, g_sessionMap.size(), g_characterMap.size(), cnt);
		printf("%s", LogBuf);
		fwrite(LogBuf, 1, strlen(LogBuf), fp);
		frame = 0;
		fclose(fp);
	}
}