#pragma comment(lib, "winmm.lib")

#include <Windows.h>

#include "Session.h"
#include "CNetwork.h"

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
	// 타이머 해상도 변경
	timeBeginPeriod(1);
	
	// 
	RunProgram();

	// 타이머 해상도 되돌리기
	timeEndPeriod(1);
}