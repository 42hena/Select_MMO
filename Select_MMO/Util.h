#ifndef __UTIL_H__
#define __UTIL_H__

bool PacketMarshall(st_Session* session);
void GetAroundSector(int secY, int secX, st_SECTOR_AROUND* around);
void PushCreateMyCharacterJob(st_Session* session, DWORD id, BYTE dir, short x, short y, BYTE hp);
void PushCreateOtherCharacterToMeJob(st_Session* session, DWORD id, BYTE dir, short x, short y, BYTE hp);
void PushCreateMyCharacterToOthersJob(st_Session* session, DWORD id, BYTE dir, short x, short y, BYTE hp);
void PushCreateOtherMoveStartTomeJob(st_Session* session, DWORD id, BYTE dir, short x, short y, BYTE hp);

#endif
