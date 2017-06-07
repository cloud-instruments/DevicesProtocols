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
	std::cerr << "  -s ms : set sender sleep, default:" << SENDER_SLEEP_DEFAULT << "mS" << std::endl;
	std::cerr << "  -l path : enable logfile" << std::endl;
}

int main(int argc, char **argv)
{
	int expected_argc = 2;

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
	}

	// Validate the parameters
	if (argc < expected_argc) {
		print_usage(argv[0]);
		return -1;
	}

	if (!logpath.empty()) {
		// open log file
		logStream = new std::ofstream(logpath, std::ios::app);
		plog = new streamlog(*logStream, streamlog::debug);
	}

	unsigned short port = atoi(argv[argc - 1]);

	ciupServerStart(port);

	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	// while run print logs
	ciupLog log;
	while (run) {
		if (ciupGetLog(&log) == 0) {
			if (plog) *plog << log.descr << log.level << std::endl;
			std::cout << log.descr << std::endl;
		}
		Sleep(10);

		// TODO populate point queue
	}

	ciupServerStop();
    return 0;
}

