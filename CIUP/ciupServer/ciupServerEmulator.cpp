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
	int cycle = 0;
	while (gRun) {

		point.counter = counter;
		point.channel = ch;
		point.Cycle = cycle;
		point.TestTime = (float)(GetTickCount64() - sTime) / 1000;
		point.StepTime = (float)(GetTickCount64() - sTime) / 1000;
		point.Current = (float)rand() / RAND_MAX;
		point.Voltage = (float)rand() / RAND_MAX;
		point.Capacity = (float)rand() / RAND_MAX;
		point.LHCCapacity = (float)rand() / RAND_MAX;
		point.HCCapacity = (float)rand() / RAND_MAX;
		point.Energy = (float)rand() / RAND_MAX;
		point.HCEnergy = (float)rand() / RAND_MAX;
		point.LHCEnergy = (float)rand() / RAND_MAX;
		point.RF1 = (uint8)rand() / RAND_MAX;
		point.RF2 = (uint8)rand() / RAND_MAX;
		
		ciupServerEnqueueDatapoint(point);
		cycle++;
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