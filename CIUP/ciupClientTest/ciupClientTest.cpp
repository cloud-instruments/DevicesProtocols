// ciupClientTest.cpp : test application for ciupClientDll

#include "stdafx.h"
#include <iostream>
#include "streamlog.h"

// Need to link with ciupClientDll.dll
#pragma comment(lib, "ciupClientDll.lib")
#include "../ciupClientDll/ciupClientDll.h"

bool run = true;

// log globals
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

void __stdcall dataCb(const char* json, int recvId, const char *sendAddr, unsigned short sendPort) {

	std::cout << recvId << "(" << sendAddr << ":" <<sendPort << "): " << json << std::endl;
	if (plog) *plog << recvId << "(" << sendAddr << ":" << sendPort << "): " << json << std::endl;
}

void __stdcall errorCb(int errcode, const char* descr, int recvId) {

	std::cerr << recvId << ": " << "error:" << errcode << " " << descr << std::endl;
	if (plog) *plog << recvId << ": " << "error:" << errcode << " " << descr << streamlog::error << std::endl;
}

void printCiupError(const char* msg) {

	char errdescr[1024];
	int errcode = ciupcGetLastError(errdescr, 1024);
	std::cerr << msg << " error:" << errcode << " " << errdescr << std::endl;
	if (plog) *plog << msg << " error:" << errcode << " " << errdescr << streamlog::error << std::endl;
}

void print_usage(const char *exe) {

	std::cerr << "Cloud Instruments Unified Protocol client test application" << std::endl;
	std::cerr << "usage: " << exe << " ip port" << std::endl;
	std::cerr << "  -l path : enable logfile" << std::endl;
}

int main(int argc, char **argv)
{
	int expected_argc = 3;
	// TODO: log

	// parse arguments
	for (int i = 1; i < argc; i++) {

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

	char *addr = argv[argc - 2];
	unsigned short port = atoi(argv[argc - 1]);

	std::cout << "Starting" << argv[0] << " for " << addr << ":" << port << std::endl;
	if (plog) *plog << "Starting " << " for " << addr << ":" << port << argv[0] << std::endl;

	char json[2048];

	if (ciupcGetServerInfo(addr, port, json, 2048) == CIUP_NO_ERR ) {

		std::cout << "server info : " << json << std::endl;
		if (plog)  *plog << "server info : " << json << std::endl;
	}
	else {

		printCiupError("ciupcGetServerInfo");
		return -1;
	}

	std::cout << "starting receiver " << std::endl;
	if (plog)  *plog << "starting receiver " << std::endl;

	int id = ciupcStartReceiver(addr, port, dataCb, errorCb);
	if ( id < 0) {

		printCiupError("ciupcStartReceiver");
		return -1;
	}

	// sleep until CTRL-C
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	while (run) Sleep(100);

	ciupcStopAllReceivers();

	if (plog) delete plog;
	if (logStream) delete logStream;

    return 0;
}

