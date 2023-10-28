#include <Windows.h>

#include "PROTOCOL.h"
#include "SerializationBuffer.h"
#include "CreatePacket.h"

void CreatePacketCreateMyCharacter(CSerialization& buffer, DWORD id, BYTE dir, short x, short y, BYTE hp)
{
	SHORT size;
	SHORT type;
// -----

	size = 12;
	type = SERVER_TO_CLIENT_CREATE_MY_CHARACTER;

	// HeaderInfo
	buffer << size << type;;

	// Payload Info
	buffer << id << dir << x << y << hp;
}

void CreatePacketCreateOtherCharacter(CSerialization& buffer, DWORD id, BYTE dir, short x, short y, BYTE hp)
{
	SHORT size;
	SHORT type;
// -----

	size = 12; // 2 + 10
	type = SERVER_TO_CLIENT_CREATE_OTHER_CHARACTER;
	// HeaderInfo
	buffer << size << type;;

	// Payload Info
	buffer << id << dir << x << y << hp;
}

void CreatePacketDeleteCharacter(CSerialization& buffer, DWORD id)
{
	SHORT size;
	SHORT type;
// -----

	size = 6; // 2 + 4
	type = SERVER_TO_CLIENT_DELETE_CHARACTER;

	// HeaderInfo
	buffer << size << type;;

	// Payload Info
	buffer << id;
}

void CreatePacketMoveStart(CSerialization& buffer, DWORD id, BYTE dir, short x, short y)
{
	SHORT size;
	SHORT type;
// -----
	
	size = 11; // 2 + 9
	type = SERVER_TO_CLIENT_MOVE_START;
	
	// HeaderInfo
	buffer << size << type;;

	// Payload Info
	buffer << id << dir << x << y;
}

void CreatePacketMoveStop(CSerialization& buffer, DWORD id, BYTE dir, short x, short y)
{
	SHORT size;
	SHORT type;
// -----
	
	size = 11; // 2 + 9
	type = SERVER_TO_CLIENT_MOVE_STOP;

	// HeaderInfo
	buffer << size << type;;

	// Payload Info
	buffer << id << dir << x << y;
}

void CreatePacketAttack1(CSerialization& buffer, DWORD id, BYTE dir, short x, short y)
{
	SHORT size;
	SHORT type;
// -----

	size = 11; // 2 + 9
	type = SERVER_TO_CLIENT_ATTACK1;

	// HeaderInfo
	buffer << size << type;;

	// Payload Info
	buffer << id << dir << x << y;
}

void CreatePacketAttack2(CSerialization& buffer, DWORD id, BYTE dir, short x, short y)
{
	SHORT size;
	SHORT type;
// -----
	
	size = 11; // 2 + 9
	type = SERVER_TO_CLIENT_ATTACK2;

	// HeaderInfo
	buffer << size << type;;

	// Payload Info
	buffer << id << dir << x << y;
}

void CreatePacketAttack3(CSerialization& buffer, DWORD id, BYTE dir, short x, short y)
{
	SHORT size;
	SHORT type;
// -----

	size = 11; // 2 + 9
	type = SERVER_TO_CLIENT_ATTACK3;

	// HeaderInfo
	buffer << size << type;;

	// Payload Info
	buffer << id << dir << x << y;
}

void CreatePacketDamage(CSerialization& buffer, DWORD attacker_id, DWORD victim_id, BYTE victim_hp)
{
	SHORT size;
	SHORT type;
// -----

	size = 11; // 2 + 9
	type = SERVER_TO_CLIENT_DAMAGE;

	// HeaderInfo
	buffer << size << type;

	// Payload Info
	buffer << attacker_id << victim_id << victim_hp;
}

void CreatePacketSync(CSerialization& buffer, DWORD id, short x, short y)
{
	SHORT size;
	SHORT type;
// -----

	size = 10; // 2 + 8
	type = SERVER_TO_CLIENT_SYNC;

	// HeaderInfo
	buffer << size << type;

	// Payload Info
	buffer << id << x << y;
}

void CreatePacketEcho(CSerialization& buffer, DWORD time)
{
	SHORT size;
	SHORT type;
// -----

	size = 6; // 2 + 4	
	type = SERVER_TO_CLIENT_ECHO;

	// HeaderInfo
	buffer << size << type;

	// Payload Info
	buffer << time;
}
