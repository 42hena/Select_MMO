#pragma comment(lib, "winmm.lib")

#include <Windows.h>

int main()
{
	// Ÿ�̸� �ػ� ����
	timeBeginPeriod(1);
	
	// Ÿ�̸� �ػ� �ǵ�����
	timeEndPeriod(1);
}