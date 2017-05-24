// ciupClientTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

// Need to link with ciupClientDll.dll
#pragma comment(lib, "ciupClientDll.lib")
#include "../ciupClientDll/ciupClientDll.h"

bool run = true;
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


void __stdcall ciupCb(const char* json, int recvId) {

	std::cout << recvId << ": " << json << std::endl;

}

void print_usage(const char *exe) {

	std::cerr << "Cluod Instruments Unified Protocol client test application" << std::endl;
	std::cerr << "usage: " << exe << " ip port" << std::endl;
}

int main(int argc, char **argv)
{
	int expected_argc = 3;

	// parse arguments
	/*for (int i = 1; i < argc; i++) {

		if (!strcmp(argv[i], "-s")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			addr = argv[i + 1];
			expected_argc += 2;
		}

		if (!strcmp(argv[i], "-t")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			msSleep = atol(argv[i + 1]);
			std::cout << "Sleep set to " << msSleep << "mS" << std::endl;
			expected_argc += 2;
		}

		if (!strcmp(argv[i], "-o")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			timeout = atol(argv[i + 1]);
			std::cout << "Timeout set to " << timeout << "mS" << std::endl;
			expected_argc += 2;
		}
	}*/

	// Validate the parameters
	if (argc < expected_argc) {
		print_usage(argv[0]);
		return -1;
	}

	char *addr = argv[argc - 2];
	unsigned short port = atoi(argv[argc - 1]);

	char json[2048];

	if (ciupcGetServerInfo(addr, port, json, 2048) == 0) {

		std::cout << "server info : " << json << std::endl;
	}
	else {

		std::cerr << "cannot get server info" << std::endl;
		return -1;
	}


	if (ciupcStartReceiver(addr, port, ciupCb) == 0) {

		std::cout << "started receiver" << std::endl;
	}
	else {

		std::cerr << "cannot start receiver";
		return -1;
	}

	// sleep until CTRL-C
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	while (run) Sleep(1000);

	ciupcStopAllReceivers();
    return 0;
}

