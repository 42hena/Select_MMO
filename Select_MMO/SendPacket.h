#ifndef __SEND_PACKET_H__
#define __SEND_PACKET_H__

void SendPacket_Unicast();
void SendPacket_Sector();
void SendPacket_Around();
void SendPacket_BroadCast(st_Session*, CSerialization *packet);


#endif
