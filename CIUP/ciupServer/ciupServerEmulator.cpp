#include "stdafx.h"
#include "ciupServerEmulator.h"

static bool gRun = true;
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

		ciupEnqueueDatapoint(point);

		counter = ++counter%USHRT_MAX;
		Sleep(gSleep);
	}
	return 0;
}

int serverEmulatorStart(int chCount, int sleep)
{
	// Sleep at least 1 (minimum for windows)
	gSleep = sleep>0?sleep:1;

	// start channels emulators threads
	for (int i = 0; i < chCount; i++) {
		CreateThread(0, 0, serverEmulatorChannelThread, (LPVOID)i, 0, NULL); // TODO: error control
	}

	return 0;
}

int serverEmulatorStop()
{
	gRun = false;

	// TODO: thread join

	return 0;
}