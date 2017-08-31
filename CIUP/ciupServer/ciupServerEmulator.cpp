#include "stdafx.h"
#include "ciupServerEmulator.h"

static bool gRun = false;
static int gSleep;

DWORD WINAPI serverEmulatorChannelThread(LPVOID lpParam) {

	int ch = (int)lpParam;

	ciupLog log;
	ciupDataPoint point;
	USHORT counter = 0;
	ULONGLONG sTime = GetTickCount64();

	while (gRun) {

		point.counter = counter;
		point.channel = ch;
		point.Stime = (float)(GetTickCount64() - sTime) / 1000;
		point.Acurr = (float)rand() / RAND_MAX;
		point.AHcap = (float)rand() / RAND_MAX;
		point.Ktemp = (float)rand() / RAND_MAX;
		point.Vdiff = (float)rand() / RAND_MAX;

		ciupServerEnqueueDatapoint(point);

		counter = ++counter%USHRT_MAX;
		Sleep(gSleep);
	}
	return 0;
}

int serverEmulatorStart(int chCount, int sleep)
{
	if (gRun) return 0;

	// Sleep at least 1 (minimum for windows)
	gSleep = sleep>0?sleep:1;

	gRun = true;

	// start channels emulators threads
	for (int i = 0; i < chCount; i++) {
		HANDLE hThread = CreateThread(0, 0, serverEmulatorChannelThread, (LPVOID)i, 0, NULL);
		if (hThread == NULL) {
			// error 
		}
		else {
			CloseHandle(hThread);
		}
	}

	return 0;
}

int serverEmulatorStop()
{
	if (!gRun) return 0;

	gRun = false;
	return 0;
}