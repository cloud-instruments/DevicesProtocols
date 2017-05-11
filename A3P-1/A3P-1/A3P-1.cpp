// Test application for Arbin 3rd party protocol
// (c)2017 Matteo Lucarelli
// 20170510 - V0.1
// 20170511 - V0.2 : new Arbin protocol version (1.1)

#include "stdafx.h"
#include "Arbin3pp.h"

#include <iostream>
#include <Windows.h>

bool gRepeat = true;

void print_usage(const char *exe)
{
	std::cerr << "usage: " << exe << " [-0|-1|-2] device-ip-addr" << std::endl;
	std::cerr << "-0: (DEFAULT) connect and show incoming messages" << std::endl;
	std::cerr << "-1: 0 + send CMD_SET_SYSTEMTIME every 5s" << std::endl;
	std::cerr << "-2: 1 + change device third party mode" << std::endl;
}

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {

	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
		printf("[Ctrl]+C\n");
		gRepeat = false;
		return TRUE;
	default:
		// Pass signal on to the next handler
		return FALSE;
	}
}

int main(int argc, char **argv)
{
	int runmode = 0;
	int expected_argc = 2;

	for (int i = 1; i < argc; i++){

		if (!strcmp(argv[i], "-0")) {

			runmode = 0;
			expected_argc++;
		}

		if (!strcmp(argv[i], "-1")) {

			runmode = 1;
			expected_argc++;
		}

		if (!strcmp(argv[i], "-2")) {

			runmode = 2;
			expected_argc++;
		}
	}

	// Validate parameters
	if ( argc < expected_argc ) {
		print_usage(argv[0]);
		return 1;
	}

	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	std::cout << "connecting " << argv[argc-1] << std::endl;
	a3p_connect(argv[argc-1]);

	if (runmode == 0) {

		// connection test
		std::cout << "[CTRL][C] to gracefully disconnect" << std::endl;
		while (gRepeat) {

			// TODO - get messages
			Sleep(1000);

		}

		// TODO: show all incoming messages

	} else {
		std::cerr << "MODE UNIMPLEMENTED" << std::endl;
		Sleep(1000);
	}

	std::cout << "disconnecting" << std::endl;
	a3p_disconnect();
    return 0;
}

