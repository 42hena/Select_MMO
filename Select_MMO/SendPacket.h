#ifndef __SEND_PACKET_H__
#define __SEND_PACKET_H__

void SendPacketUniCast(st_Session* session, CSerialization* packet);
void SendPacketSector(int secY, int secX, CSerialization* packet, st_Character* exception);
void SendPacketSectorAroundCast(st_Session* session, CSerialization* packet, bool sendMe = false);
void SendPacketBroadCast(st_Session* session, CSerialization* packet);


#endif
