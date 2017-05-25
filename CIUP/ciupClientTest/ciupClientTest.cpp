// ciupClientTest.cpp : Defines the entry point for the console application.
//

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
		printf("[Ctrl]+C\n");
		run = false;
		return TRUE;
	default:
		// Pass signal on to the next handler
		return FALSE;
	}
}

void __stdcall dataCb(const char* json, int recvId) {

	std::cout << recvId << ": " << json << std::endl;
	if (plog) *plog << recvId << ": " << json << std::endl;
}

void __stdcall errorCb(int errcode, const char* descr, int recvId) {

	if (plog) *plog << recvId << ": " << "error " << errcode << " " << descr << streamlog::error << std::endl;
	std::cerr << recvId << ": " << "error " << errcode << " " << descr << std::endl;
}

void printCiupError(const char* msg) {

	char errdescr[1024];
	int errcode = ciupcGetLastError(errdescr, 1024);
	std::cerr << msg << " - " << errcode << " - " << errdescr << std::endl;
	if (plog) *plog << msg << " - " << errcode << " - " << errdescr << streamlog::error << std::endl;
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

	std::cout << "Started " << argv[0] << std::endl;
	if (plog) *plog << "Started " << argv[0] << std::endl;

	char *addr = argv[argc - 2];
	unsigned short port = atoi(argv[argc - 1]);

	char json[2048];

	if (ciupcGetServerInfo(addr, port, json, 2048) == CIUP_NO_ERR ) {

		std::cout << "server info : " << json << std::endl;
		if (plog)  *plog << "server info : " << json << std::endl;
	}
	else {

		printCiupError("ciupcGetServerInfo");
		return -1;
	}

	int id = ciupcStartReceiver(addr, port, dataCb, errorCb);
	if ( id >= 0) {

		std::cout << "started receiver " << id << std::endl;
		if (plog)  *plog << "started receiver " << id << std::endl;

	}
	else {

		printCiupError("ciupcStartReceiver");
		return -1;
	}

	// sleep until CTRL-C
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	while (run) Sleep(1000);

	ciupcStopAllReceivers();

	if (plog) delete plog;
	if (logStream) delete logStream;

    return 0;
}

