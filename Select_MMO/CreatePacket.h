#ifndef __CREATE_PACKET_H__
#define __CREATE_PACKET_H__

void CreatePacketCreateMyCharacter(CSerialization& buffer, DWORD id, BYTE dir, short x, short y, BYTE hp);
void CreatePacketCreateOtherCharacter(CSerialization& buffer, DWORD id, BYTE dir, short x, short y, BYTE hp);
void CreatePacketDeleteCharacter(CSerialization& buffer, DWORD id);
void CreatePacketMoveStart(CSerialization& buffer, DWORD id, BYTE dir, short x, short y);
void CreatePacketMoveStop(CSerialization& buffer, DWORD id, BYTE dir, short x, short y);
void CreatePacketAttack1(CSerialization& buffer, DWORD id, BYTE dir, short x, short y);
void CreatePacketAttack2(CSerialization& buffer, DWORD id, BYTE dir, short x, short y);
void CreatePacketAttack3(CSerialization& buffer, DWORD id, BYTE dir, short x, short y);
void CreatePacketDamage(CSerialization& buffer, DWORD attacker_id, DWORD victim_id, BYTE victim_hp);
void CreatePacketSync(CSerialization& buffer, DWORD id, short x, short y);
void CreatePacketEcho(CSerialization& buffer, DWORD time);

#endif
