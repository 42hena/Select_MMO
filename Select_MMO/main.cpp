#pragma comment(lib, "winmm.lib")

#include <unordered_map>
#include <Windows.h>

#include "Session.h"
#include "Character.h"
#include "CNetwork.h"
#include "Util.h"

std::unordered_map<SOCKET, st_Session*> g_sessionMap;
std::unordered_map<DWORD, st_Character *> g_characterMap;
std::list< st_Character* > g_sector[6400 / 150 + 1][6400 / 150 + 1];
DWORD g_sendCnt, g_recvCnt, g_acceptCnt;
DWORD g_frameTime;
DWORD totalTime;
DWORD g_selectCnt;
DWORD g_whileCnt;
DWORD g_minFrame = 1e9, g_maxFrame, g_avgFrame, g_prevFrame;
DWORD g_syncCnt;
void RunProgram()
{
	CNetwork network;
// -----
	g_frameTime = g_prevFrame = timeGetTime();
	while (1)
	{
		g_whileCnt++;
		network.NetworkIO();
		Update();
		PrintLog();
	}
}

int main()
{
	// 타이머 해상도 변경
	timeBeginPeriod(1);
	
	// 
	RunProgram();

	// 타이머 해상도 되돌리기
	timeEndPeriod(1);
}