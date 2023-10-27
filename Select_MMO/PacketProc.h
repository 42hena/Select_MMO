#ifndef __PACKET_PROC_H__
#define __PACKET_PROC_H__

bool MoveStart(st_Session* session, CSerialization* packet);
bool MoveStop(st_Session* session, CSerialization* packet);

bool Attack1Packet(st_Session* session, CSerialization* buffer);
//bool Attack2Packet(st_Session* session, CSerialization* buffer);
//bool Attack3Packet(st_Session* session, CSerialization* buffer);
bool Echo(st_Session* session, CSerialization* packet);

#endif