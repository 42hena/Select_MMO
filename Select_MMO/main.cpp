#pragma comment(lib, "winmm.lib")

#include <Windows.h>

int main()
{
	// 타이머 해상도 변경
	timeBeginPeriod(1);
	
	// 타이머 해상도 되돌리기
	timeEndPeriod(1);
}