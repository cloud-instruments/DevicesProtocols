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

#include "w32_log.h"

bool gRepeat = true;

std::ofstream *wlog_file;
w32_log *pwlog;

void print_usage(const char *exe)
{
	std::cerr << "usage: " << exe << " [-0|-1|-2] device-ip-addr" << std::endl;
	std::cerr << "-0: (DEFAULT) connect and show incoming messages" << std::endl;
	std::cerr << "-1: 0 + send CMD_SET_SYSTEMTIME every 5s" << std::endl;
	std::cerr << "-2: 1 + change device third party mode" << std::endl;
}

// ctrl-c handler
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {

	switch (dwCtrlType){
		case CTRL_C_EVENT:
			*pwlog << W32_LOG_FINFO << "CTRL-c" << w32_log::trace << std::endl;
			printf("[Ctrl]+C\n");
			gRepeat = false;
			return TRUE;
		default:
			return FALSE;
	}
}

void show_buff(a3p_msg msg){

	for (unsigned int i = 0; i <  msg.size; i++) {

		*pwlog << std::setw(2) << std::setfill('0') << std::hex << (int)msg.buff[i] << " ";

		std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)msg.buff[i] << " ";
		if ((i + 1) % 8 == 0) std::cout << std::endl;
	}

	std::cout << std::endl;
	*pwlog << w32_log::trace << std::endl;
}

int main(int argc, char **argv)
{
	// intercept ctrl-c to gracefully close
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	// ionit log
	wlog_file = new std::ofstream("A3P-1.log", std::ios::app);
	pwlog = new w32_log(*wlog_file);
	*pwlog << W32_LOG_FINFO << "starting:" << argv[0] << w32_log::trace << std::endl;

	bool sst = false;
	bool sdu = false;
	int expected_argc = 2;

	for (int i = 1; i < argc; i++){

		if (!strcmp(argv[i], "-0")) {

			*pwlog << W32_LOG_FINFO << "-0" << w32_log::trace << std::endl;
			expected_argc++;
		}

		if (!strcmp(argv[i], "-1")) {

			*pwlog << W32_LOG_FINFO << "-1" << w32_log::trace << std::endl;
			sst = true;
			expected_argc++;
		}

		if (!strcmp(argv[i], "-2")) {

			*pwlog << W32_LOG_FINFO << "-2" << w32_log::trace << std::endl;
			sst = true;
			sdu = true;
			expected_argc++;
		}
	}

	// Validate parameters
	if ( argc < expected_argc ) {
		*pwlog << W32_LOG_FINFO << "No IPADDR" << w32_log::error << std::endl;
		print_usage(argv[0]);
		return 1;
	}

	*pwlog << W32_LOG_FINFO << argv[argc - 1] << w32_log::trace << std::endl;

	std::cout << "connecting " << argv[argc-1] << std::endl;
	a3p_init(argv[argc - 1]);

	if (a3p_connect(sst,sdu) != 0) {

		*pwlog << W32_LOG_FINFO << "Connection refused" << w32_log::error << std::endl;

		std::string msg;
		while (a3p_get_message(&msg) == 0) {

			*pwlog << W32_LOG_FINFO << "a3p:" << msg << w32_log::warning << std::endl;
			std::cout << msg << std::endl;
			Sleep(100);
		}

		std::cerr << "Connection refused: control your IP connection" << std::endl;
		return -1;
	}

	// connection test
	std::cout << "[CTRL][C] to gracefully disconnect" << std::endl;
	while (gRepeat) {

		std::string msg;
		while (a3p_get_message(&msg) == 0) {

			*pwlog << W32_LOG_FINFO << "a3p:" << msg << w32_log::warning << std::endl;
			std::cout << msg << std::endl;
			Sleep(100);
		}

		a3p_msg mch1;
		if (a3p_get_ch1(&mch1) == 0) {

			*pwlog << W32_LOG_FINFO << "ch1 received " << std::dec << mch1.size << " bytes" << w32_log::trace << std::endl;
			std::cout << "ch1 received " << std::dec << mch1.size << " bytes" << std::endl;
			show_buff(mch1);
		}

		a3p_msg mch2;
		if (a3p_get_ch2(&mch2) == 0) {
			*pwlog << W32_LOG_FINFO << "ch2 received " << std::dec << mch1.size << " bytes" << w32_log::trace << std::endl;
			std::cout << "ch2 received " << std::dec << mch2.size << " bytes" << std::endl;
			show_buff(mch2);
		}

		Sleep(100);
	}

	*pwlog << W32_LOG_FINFO << "disconnecting" << w32_log::trace << std::endl;

	std::cout << "disconnecting" << std::endl;
	a3p_disconnect();
	a3p_delete();

	delete pwlog;
	delete wlog_file;

    return 0;
}

