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
	// Ÿ�̸� �ػ� ����
	timeBeginPeriod(1);
	
	// 
	RunProgram();

	// Ÿ�̸� �ػ� �ǵ�����
	timeEndPeriod(1);
}