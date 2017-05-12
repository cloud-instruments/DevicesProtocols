// Test application for Arbin 3rd party protocol
// (c)2017 Matteo Lucarelli
// 20170510 - V0.1
// 20170511 - V0.2 : new Arbin protocol version (1.1)

#include "stdafx.h"
#include "Arbin3pp.h"

#include <Windows.h>

#include <string>
#include <iostream>
#include <iomanip>

bool gRepeat = true;

void print_usage(const char *exe)
{
	std::cerr << "usage: " << exe << " [-0|-1|-2] device-ip-addr" << std::endl;
	std::cerr << "-0: (DEFAULT) connect and show incoming messages" << std::endl;
	std::cerr << "-1: 0 + send CMD_SET_SYSTEMTIME every 5s" << std::endl;
	std::cerr << "-2: 1 + change device third party mode" << std::endl;

	// TODO: output to file 
}

// ctrl-c handler
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {

	switch (dwCtrlType){
		case CTRL_C_EVENT:
			printf("[Ctrl]+C\n");
			gRepeat = false;
			return TRUE;
		default:
			return FALSE;
	}
}

void show_buff(a3p_msg msg){

	for (unsigned int i = 0; i <  msg.size; i++) {

		std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)msg.buff[i] << " ";
		if ((i + 1) % 8 == 0) std::cout << std::endl;
	}
	std::cout << std::endl;
}

int main(int argc, char **argv)
{
	// intercept ctrl-c to gracefully close
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	bool sst = false;
	bool sdu = false;
	int expected_argc = 2;

	for (int i = 1; i < argc; i++){

		if (!strcmp(argv[i], "-0")) {

			expected_argc++;
		}

		if (!strcmp(argv[i], "-1")) {

			sst = true;
			expected_argc++;
		}

		if (!strcmp(argv[i], "-2")) {

			sst = true;
			sdu = true;
			expected_argc++;
		}
	}

	// Validate parameters
	if ( argc < expected_argc ) {
		print_usage(argv[0]);
		return 1;
	}

	std::cout << "connecting " << argv[argc-1] << std::endl;
	a3p_init(argv[argc - 1]);

	if (a3p_connect(sst,sdu) != 0) {

		std::cerr << "Connection refused" << std::endl;
		return -1;
	}

	// connection test
	std::cout << "[CTRL][C] to gracefully disconnect" << std::endl;
	while (gRepeat) {

		std::string msg;
		while (a3p_get_message(&msg) == 0) {
			std::cout << msg << std::endl;
			Sleep(100);
		}

		a3p_msg mch1;
		if (a3p_get_ch1(&mch1) == 0) {
			std::cout << "ch1 received " << std::dec << mch1.size << " bytes" << std::endl;
			show_buff(mch1);
		}

		a3p_msg mch2;
		if (a3p_get_ch2(&mch2) == 0) {
			std::cout << "ch2 received " << std::dec << mch2.size << " bytes" << std::endl;
			show_buff(mch2);
		}

		Sleep(100);
	}

	std::cout << "disconnecting" << std::endl;
	a3p_disconnect();
	a3p_delete();

    return 0;
}

