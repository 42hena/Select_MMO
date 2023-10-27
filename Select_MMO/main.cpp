#pragma comment(lib, "winmm.lib")

#include <unordered_map>
#include <Windows.h>

#include "Session.h"
#include "Character.h"
#include "CNetwork.h"

std::unordered_map<SOCKET, st_Session*> g_sessionMap;
std::unordered_map<DWORD, st_Character *> g_characterMap;
std::list< st_Character* > g_sector[6400 / 150 + 1][6400 / 150 + 1];

void RunProgram()
{
	CNetwork network;
// -----

	while (1)
	{
		network.NetworkIO();
	}
}

int main()
{
	// Ÿ�̸� �ػ� ����
	timeBeginPeriod(1);
	
	// 
	RunProgram();

	// Ÿ�̸� �ػ� �ǵ�����
	timeEndPeriod(1);
}