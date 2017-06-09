// ciupServerEmulator.cpp : CIUP server test application

#include "stdafx.h"
#include "../ciupClientTest/streamlog.h"
#include "ciupServerCommon.h"

#include <iostream>

// log globals
std::string logpath;
std::ofstream *logStream = NULL;
streamlog *plog = NULL;

#define SENDER_SLEEP_DEFAULT 1000
int senderSleep = SENDER_SLEEP_DEFAULT;

bool run = true;

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
}

int main(int argc, char **argv)
{
	int expected_argc = 2;
	streamlog::level l = streamlog::debug;
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
	}

	unsigned short port = atoi(argv[argc - 1]);

	// start the sender server
	ciupServerStart(port);

	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	ciupLog log;
	ciupDataPoint point;
	USHORT counter = 0;
	ULONGLONG sTime = GetTickCount64();

	DWORD tPrev = GetTickCount();
	DWORD cPrev = 0;


	struct timeval tv;
	fd_set dummy;
	SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	while (run) {

		while (ciupGetLog(&log) == 0) {
			if (plog) *plog << log.descr << log.level << std::endl;
			if ( log.level >=l ) std::cout << log.descr << std::endl;
			Sleep(1);
		}

		point.counter = counter;
		point.channel = 0;
		point.Stime = (float)(GetTickCount() - sTime) / 1000;
		point.Acurr = (float)rand() / RAND_MAX;
		point.AHcap = (float)rand() / RAND_MAX;
		point.Ktemp = (float)rand() / RAND_MAX;
		point.Vdiff = (float)rand() / RAND_MAX;

		ciupEnqueueDatapoint(point);

		if (performance) {

			DWORD tNow = GetTickCount();

			if (tNow - tPrev > 1000) {

				// FIXME: don't care overflow

				double mS = (counter - cPrev) / ((tNow - tPrev) / 1000.0);

				std::cout << mS << " msg/s " << " (" << counter - cPrev << "msg in " << tNow - tPrev << " mS)" << std::endl;
				if (plog) *plog << mS << " msg/s " << " (" << counter - cPrev << "msg in " << tNow - tPrev << " mS)" << streamlog::trace << std::endl;

				tPrev = GetTickCount();
				cPrev = counter;
			}
		}
		else {
			if (plog) *plog << "Enqueued new point counter:" << counter << streamlog::debug << std::endl;
			if ( streamlog::debug>=l ) std::cout << "Enqueued new point counter:" << counter << std::endl;
		}

		counter = ++counter%USHRT_MAX;
		Sleep(senderSleep?senderSleep:1); // Sleep at least 1 (minimum for windows)
	}

	ciupServerStop();
    return 0;
}

