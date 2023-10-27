#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "Session.h"
#include "Sector.h"

struct st_Character
{
	st_Character(st_Session* newSession, DWORD	newId, BYTE action, BYTE dir, char hp)
		: session(newSession), characterId(newId), action(action), direction(dir), hp(hp), y(100), x(100), attackTime(0)
	{
		sector = { y / 150, x / 150 };
		prevSector = sector;
	}


	st_Session* session; // 8
	DWORD		characterId;// 4
	BYTE		action; // 1
	BYTE		direction; // 1
	char		hp;
	int			y;
	int			x;

	st_Sector	sector; //4
	st_Sector	prevSector;// 4
	DWORD		attackTime;
};

#endif
