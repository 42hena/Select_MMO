#ifndef __PACKET_PROC_H__
#define __PACKET_PROC_H__

void MoveStart(st_Session* session, CSerialization* packet);
void MoveStop(st_Session* session, CSerialization* packet);

void Attack1Packet(st_Session* session, CSerialization* buffer);
//void Attack2Packet(st_Session* session, CSerialization* buffer);
//void Attack3Packet(st_Session* session, CSerialization* buffer);
void Echo(st_Session* session, CSerialization* packet);

#endif