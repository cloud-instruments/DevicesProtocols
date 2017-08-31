// ciupClientTest.cpp : test application for ciupClientDll

#include "stdafx.h"
#include <iostream>
#include "streamlog.h"

// Need to link with ciupClientDll.dll
#pragma comment(lib, "ciupClientDll.lib")
#include "../ciupClientDll/ciupClientDll.h"

bool run = true;
bool performance = false;

// log globals  - WARN: streamlog is not thread safe
std::string logpath;
std::ofstream *logStream = NULL;
streamlog *plog = NULL;

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

void __stdcall dataCb(int msgtype, const char* json, int connId) {

	static DWORD tPrev = GetTickCount();
	static DWORD cPrev = 0;
	static DWORD msgcount = 0;

	if (performance) {

		DWORD tNow = GetTickCount();
		msgcount++;

		if (tNow - tPrev > 1000) {

			// FIXME: don't care overflow
			
			double mS = (msgcount - cPrev) / ((tNow - tPrev) / 1000.0);

			std::cout << connId << ": " << mS << " msg/s " << " (" << msgcount - cPrev << "msg in " << tNow - tPrev << " mS)" << std::endl;
			if (plog) *plog << connId << ": " << mS << " msg/s " << " (" << msgcount - cPrev << "msg in " << tNow - tPrev << " mS)" << streamlog::trace << std::endl;

			tPrev = GetTickCount();
			cPrev = msgcount;
		}
	}
	else {
		std::cout << connId << ": (" << msgtype << ") " << json << std::endl;
		if (plog) *plog << connId << ": (" << msgtype << ") " << json << streamlog::trace << std::endl;
	}
}

void __stdcall errorCb(int errcode, const char* descr, int connId) {

	std::cerr << connId << ": " << "code:" << errcode << " " << descr << std::endl;
	if (plog) *plog << connId << ": " << "code:" << errcode << " " << descr << streamlog::error << std::endl;
}

void print_usage(const char *exe) {

	std::cerr << "Cloud Instruments Unified Protocol client test application" << std::endl;
	std::cerr << "usage: " << exe << " ip port" << std::endl;
	std::cerr << "  -l PATH : enable logfile" << std::endl;
	std::cerr << "  -f X : filter logfile (E:errors, W:warnings, T:trace, D:debug)" << std::endl;
	std::cerr << "  -p : performance mode" << std::endl;
}

int main(int argc, char **argv)
{
	int expected_argc = 3;
	streamlog::level l = streamlog::debug;

	// parse arguments
	for (int i = 1; i < argc; i++) {

		// print help
		if (!strcmp(argv[i], "-h")) {

			print_usage(argv[0]);
			return 0;
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

	char *addr = argv[argc - 2];
	unsigned short port = atoi(argv[argc - 1]);

	std::cout << "Connecting " << addr << ":" << port << std::endl;
	if (plog) *plog << "Connecting  " << addr << ":" << port << argv[0] << streamlog::trace << std::endl;

	int id = ciupcConnect(addr, port, dataCb, errorCb);
	if (id < 0) {

		std::cerr << "Cannot connect" << std::endl;
		if (plog) *plog << "Cannot connect" << streamlog::error << std::endl;
		return -1;
	}

	std::cout << "Connection id: " << id << std::endl;
	if (plog) *plog << "Connection id: " << id << streamlog::trace << std::endl;

	// sleep until CTRL-C
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	while (run) {
		Sleep(5000);

		// ask server info every 5s
		ciupcInfo(id);
	}

	ciupcStop(id);
	ciupcDisconnect(id);

	if (plog) delete plog;
	if (logStream) delete logStream;

    return 0;
}

