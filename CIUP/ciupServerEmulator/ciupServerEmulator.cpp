// ciupServerEmulator.cpp : CIUP server test application

#include "stdafx.h"
#include "../ciupClientTest/streamlog.h"
#include "ciupServerCommon.h"

#include <iostream>
#include <queue>
#include <string>
#include <sstream>

// log globals - WARN: streamlog is not thread safe
std::string logpath;
std::ofstream *logStream = NULL;
streamlog *plog = NULL;

#define SENDER_SLEEP_DEFAULT 1000
#define SENDER_CH_DEFAULT 1

int senderSleep = SENDER_SLEEP_DEFAULT;
streamlog::level l = streamlog::debug;

bool run = true;

DWORD WINAPI channelThread(LPVOID lpParam) {

	int ch = (int)lpParam;

	ciupLog log;
	ciupDataPoint point;
	USHORT counter = 0;
	ULONGLONG sTime = GetTickCount64();

	while (run) {

		point.counter = counter;
		point.channel = ch;
		point.Stime = (float)(GetTickCount64() - sTime) / 1000;
		point.Acurr = (float)rand() / RAND_MAX;
		point.AHcap = (float)rand() / RAND_MAX;
		point.Ktemp = (float)rand() / RAND_MAX;
		point.Vdiff = (float)rand() / RAND_MAX;

		ciupEnqueueDatapoint(point);

		counter = ++counter%USHRT_MAX;
		Sleep(senderSleep>0 ? senderSleep : 1); // Sleep at least 1 (minimum for windows)
	}
	return 0;
}

// [CTRL][c] handler
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {

	switch (dwCtrlType) {
	case CTRL_C_EVENT:
		std::cout << "[Ctrl]+C" << std::endl;
		if (plog) *plog << "[Ctrl]+C" << std::endl;
		run = false;
		return TRUE;
	default:
		// Pass signal on to the next handler
		return FALSE;
	}
}

void print_usage(const char *exe) {

	std::cerr << "Cloud Instruments Unified Protocol server test application" << std::endl;
	std::cerr << "usage: " << exe << " port" << std::endl;
	std::cerr << "  -s MS : set sender sleep, default:" << SENDER_SLEEP_DEFAULT << "mS" << std::endl;
	std::cerr << "  -l PATH : enable logfile" << std::endl;
	std::cerr << "  -f X : filter logfile (E:errors, W:warnings, T:trace, D:debug)" << std::endl;
	std::cerr << "  -p : performance mode" << std::endl;
	std::cerr << "  -c N : run N channels (default:" << SENDER_CH_DEFAULT << ")" << std::endl;
}

int main(int argc, char **argv)
{
	int expected_argc = 2;
	int chCount = SENDER_CH_DEFAULT;
	bool performance = false;

	// parse arguments
	for (int i = 1; i < argc; i++) {

		// set sender sleep
		if (!strcmp(argv[i], "-s")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			senderSleep = atoi(argv[i + 1]);
			expected_argc += 2;
		}

		// set channels count
		if (!strcmp(argv[i], "-c")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			chCount = atoi(argv[i + 1]);
			expected_argc += 2;
		}

		// enable log file
		if (!strcmp(argv[i], "-l")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			logpath = argv[i + 1];
			expected_argc += 2;
		}

		// set loglevel filter
		if (!strcmp(argv[i], "-f")) {
			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			switch (*(argv[i + 1])) {
			case 'E':
				l = streamlog::error;
				break;
			case 'W':
				l = streamlog::warning;
				break;
			case 'T':
				l = streamlog::trace;
				break;
			case 'D':
				l = streamlog::debug;
				break;
			default:
				print_usage(argv[0]);
				return -1;
			}
			expected_argc += 2;
		}

		// performance mode
		if (!strcmp(argv[i], "-p")) {
			performance = true;
			expected_argc += 1;
		}
	}

	// Validate the parameters
	if (argc < expected_argc) {
		print_usage(argv[0]);
		return -1;
	}

	if (!logpath.empty()) {
		// open log file
		logStream = new std::ofstream(logpath, std::ios::app);
		plog = new streamlog(*logStream, l);

		std::cout << plog << " " << logpath << std::endl;
	}

	unsigned short port = atoi(argv[argc - 1]);

	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	// start the sender server
	ciupServerInit();
	if (ciupServerStart(port) != 0) {
		ciupLog log;
		while (ciupGetLog(&log) == 0) {
			if (plog) *plog << log.descr << log.level << std::endl;
			if (log.level >= l) std::cout << log.descr << std::endl;
		}
		return -1;
	}

	// start channels
	for (int i = 0; i < chCount; i++) {
		CreateThread(0, 0, channelThread, (LPVOID)i, 0, NULL); // TODO: error control
	}

	ciupLog log;

	DWORD tPrev = GetTickCount();
	DWORD cPrev = 0;
	DWORD tNow;
	double mS;

	while (run) {

		while (ciupGetLog(&log) == 0) {
			if (plog) *plog << log.descr << log.level << std::endl;
			std::cout << log.descr << std::endl;
			Sleep(10);
		}

		if (performance) {

			tNow = GetTickCount();
			if (tNow - tPrev > 1000) {

				int c = ciupDatapointIndex();
				int cDiff = c - cPrev;
				if (cDiff< 0) cDiff += CIUP_POINT_MAX_STORE;

				mS = cDiff / ((tNow - tPrev) / 1000.0);

				std::cout << "Generating " << mS << " msg/s " << " (" << cDiff << "msg in " << tNow - tPrev << " mS)" << std::endl;
				if (plog) *plog << "Generating " << mS << " msg/s " << " (" << cDiff << "msg in " << tNow - tPrev << " mS)" << streamlog::trace << std::endl;

				// qIndex from connections
				for (size_t i = 0; i < ciupConnectionCount(); i++) {
					std::cout << "Connection " << i << " qIndex " << ciupQueueIndex(i) << std::endl;
					if (plog) *plog << "Connection " << i << " qIndex " << ciupQueueIndex(i)<< "Connection " << i << " qIndex " << ciupQueueIndex(i) << streamlog::trace << std::endl;
				}
				
				tPrev = GetTickCount();
				cPrev = c;
			}
		}

		Sleep(100);
	}

	Sleep(500);

	ciupServerStop();
    return 0;
}

