// ciupServer.cpp : CIUP server application

#include "stdafx.h"
#include "../ciupClientTest/streamlog.h"
#include "ciupServerEmulator.h"
#include "ciupServerMaccor.h"
#include "ciupServerArbin.h"

#include <iostream>
#include <queue>
#include <string>
#include <sstream>

// log globals - WARN: streamlog is not thread safe
static std::string logpath;
static std::ofstream *logStream = NULL;
static streamlog *plog = NULL;

#define SENDER_SLEEP_DEFAULT 1000
#define SIMULATOR_CH_DEFAULT 1

static int gSleep = SENDER_SLEEP_DEFAULT;
static bool gRun = true;

// running mode
static ciupServerRunMode gRunMode = RM_EMULATOR;

// [CTRL][c] handler
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {

	switch (dwCtrlType) {
	case CTRL_C_EVENT:

		std::cout << "[Ctrl]+C" << std::endl;
		if (plog) *plog << "[Ctrl]+C" << std::endl;

		gRun = false;
		if (gRunMode == RM_EMULATOR) serverEmulatorStop();
		else if (gRunMode == RM_MACCOR) serverMaccorStop();
		else if (gRunMode == RM_ARBIN) serverArbinStop();

		return TRUE;
	default:
		// Pass signal on to the next handler
		return FALSE;
	}
}

void print_usage(const char *exe) {

	std::cerr << "Cloud Instruments Unified Protocol server test application" << std::endl;
	std::cerr << "usage: " << exe << " PORT" << std::endl;
	std::cerr << "Running mode modifiers: " << std::endl;
	std::cerr << "  -0 : Emulator mode (default)" << std::endl;
	std::cerr << "  -1 : Maccor mode" << std::endl;
	std::cerr << "  -2 : Arbin mode" << std::endl;
	std::cerr << "Emulator mode modifiers: " << std::endl;
	std::cerr << "  -c N : run N channels (default:" << SIMULATOR_CH_DEFAULT << ")" << std::endl;
	std::cerr << "  -s MS : set sender sleep, default:" << SENDER_SLEEP_DEFAULT << "mS" << std::endl;
	std::cerr << "Global modifiers:" << std::endl;
	std::cerr << "  -l PATH : enable logfile" << std::endl;
	std::cerr << "  -f X : filter logfile (E:errors, W:warnings, T:trace, D:debug)" << std::endl;
	std::cerr << "  -p : performance mode output" << std::endl;
}

int main(int argc, char **argv)
{
	int expected_argc = 2;
	int chCount = SIMULATOR_CH_DEFAULT;
	bool performance = false;
	streamlog::level logFilter = streamlog::debug;

	// parse arguments
	for (int i = 1; i < argc; i++) {

		if (!strcmp(argv[i], "-h")) {

			print_usage(argv[0]);
			return 0;
		}

		// Emulator mode //////////////////////////////////////////////////////

		// enable emulator mode
		if (!strcmp(argv[i], "-0")) {
			gRunMode = RM_EMULATOR;
			expected_argc++;
		}

		// set sender sleep
		if (!strcmp(argv[i], "-s")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			gSleep = atoi(argv[i + 1]);
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

		// Maccor mode /////////////////////////////////////////////////////////

		// enable maccor mode
		if (!strcmp(argv[i], "-1")) {
			gRunMode = RM_MACCOR;
			expected_argc++;
		}

		// Arbin mode /////////////////////////////////////////////////////////

		// enable arbin mode
		if (!strcmp(argv[i], "-2")) {
			gRunMode = RM_ARBIN;
			expected_argc++;
		}

		// All modes ///////////////////////////////////////////////////////////

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
				logFilter = streamlog::error;
				break;
			case 'W':
				logFilter = streamlog::warning;
				break;
			case 'T':
				logFilter = streamlog::trace;
				break;
			case 'D':
				logFilter = streamlog::debug;
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

	// init log file
	if (!logpath.empty()) {
		logStream = new std::ofstream(logpath, std::ios::app);
		plog = new streamlog(*logStream, logFilter);
		std::cout << "Logging to " << logpath << std::endl;
	}

	// get server listening port
	unsigned short port = atoi(argv[argc - 1]);
	std::cout << "Listening on port " << port << std::endl;
	if (plog) *plog << "Listening on port " << port << streamlog::trace << std::endl;

	// CRTL-C handler
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	ciupServerSetLogFilter(logFilter);

	// set server info
	setServerInfo(CIUP_ST_IDLE, "ciupServer", gRunMode);

	// start the server
	if (ciupServerStart(port) != 0) {
		ciupLog log;
		while (ciupServerGetLog(&log) == 0) {
			if (plog) *plog << "ciupServer: " << log.descr << log.level << std::endl;
			if (log.level >= logFilter) std::cout << "ciupServer: " << log.descr << std::endl;
		}
		return -1;
	}

	// start channels
	if (gRunMode == RM_EMULATOR) {

		std::cout << "Starting emulator mode" << std::endl;
		if (plog) *plog <<  "Starting emulator mode"  << streamlog::trace << std::endl;

		serverEmulatorStart(chCount, gSleep);
	}
	else if (gRunMode == RM_MACCOR) {

		std::cout << "Starting Maccor mode" << std::endl;
		if (plog) *plog << "Starting Maccor mode" << streamlog::trace << std::endl;
		
		maccorSetLogFilter(logFilter);
		serverMaccorStart();
	}
	else if (gRunMode == RM_ARBIN) {

		// TODO

		//std::cout << "Starting Arbin mode" << std::endl;
		//if (plog) *plog << "Starting Arbin mode" << streamlog::trace << std::endl;

		std::cerr << "Arbin mode not yet implemented" << std::endl;
		if (plog) *plog << "Arbin mode not yet implemented" << streamlog::trace << std::endl;

	}

	int mlogN;
	int slogN;
	ciupLog mlog;
	ciupLog slog;

	// performance vars
	DWORD tPrev = GetTickCount();
	DWORD cPrev = 0;
	DWORD tNow;
	double mS;

	setServerStatus(CIUP_ST_WORKING);

	while (gRun) {

		// get ciupServerLog
		while (((slogN=ciupServerGetLog(&slog)) == 0) || ((mlogN = maccorGetLog(&mlog)) == 0))
		{
			if (slogN == 0) {
				if (plog) *plog << slog.descr << slog.level << std::endl;
				if (slog.level >= logFilter) std::cout << slog.descr << std::endl;
			}

			if (mlogN == 0) {
				if (plog) *plog << mlog.descr << mlog.level << std::endl;
				if (mlog.level >= logFilter) std::cout << mlog.descr << std::endl;
			}

			Sleep(10);
		}

		// output performance data 
		if (performance) {

			tNow = GetTickCount();
			if (tNow - tPrev > 1000) {

				int c = ciupServerDatapointIndex();
				int cDiff = c - cPrev;
				if (cDiff< 0) cDiff += CIUP_POINT_MAX_STORE;

				mS = cDiff / ((tNow - tPrev) / 1000.0);

				std::cout << "Generating " << mS << " msg/s " << " (" << cDiff << "msg in " << tNow - tPrev << " mS)" << std::endl;
				if (plog) *plog << "Generating " << mS << " msg/s " << " (" << cDiff << "msg in " << tNow - tPrev << " mS)" << streamlog::trace << std::endl;

				// qIndex from connections
				for (size_t i = 0; i < ciupServerConnectionCount(); i++) {
					int qI = ciupServerQueueIndex(i);
					std::cout << "Connection " << i << " qIndex " << qI << std::endl;
					if (plog) *plog << "Connection " << i << " qIndex " << qI << streamlog::trace << std::endl;
				}
				
				tPrev = GetTickCount();
				cPrev = c;
			}
		}

		Sleep(100);
	}

	Sleep(500);

	// TODO
	//ciupServerStop();
    return 0;
}

