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

void __stdcall dataCb(int msgtype, const char* json, int recvId) {

	std::cout << recvId << ": (" << msgtype << ") " << json << std::endl;
	if (plog) *plog << recvId << ": (" << msgtype << ") " << json << std::endl;
}

void __stdcall errorCb(int errcode, const char* descr, int recvId) {

	std::cerr << recvId << ": " << "code:" << errcode << " " << descr << std::endl;
	if (plog) *plog << recvId << ": " << "code:" << errcode << " " << descr << streamlog::error << std::endl;
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

	std::cout << "Connecting " << addr << ":" << port << std::endl;
	if (plog) *plog << "Connecting  " << addr << ":" << port << argv[0] << std::endl;

	int id = ciupcConnect(addr, port, dataCb, errorCb);
	if (id < 0) {

		std::cerr << "Cannot connect" << std::endl;
		if (plog) *plog << "Cannot connect" << streamlog::error << std::endl;
		return -1;
	}

	std::cout << "Connection id: " << id << std::endl;
	if (plog) *plog << "Connection id: " << id << std::endl;

	// sleep until CTRL-C
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	while (run) {
		Sleep(5000);

		//ciupcStop(id);
		ciupcInfo(id);
		//ciupcStart(id);
	}

	ciupcStop(id);
	ciupcDisconnect(id);

	if (plog) delete plog;
	if (logStream) delete logStream;

    return 0;
}

