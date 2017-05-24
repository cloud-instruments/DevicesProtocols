// w32_udp_socket_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "w32_udp_socket.h"

#include <string>
#include <iostream>

#define SLEEP_MS_DEFAULT 1000
#define BUFFER_SIZE 32

void print_usage(const char *exe) {

	std::cerr << "Basic send-receive test for UDP socket" << std::endl;
	std::cerr << "usage: " << exe << " [-c server-ip] port" << std::endl;
	std::cerr << "-s ip : send to ip:port" << std::endl;
	std::cerr << "-t ms : sleep mS between send (default:" << SLEEP_MS_DEFAULT << ")" << std::endl;
	std::cerr << "-o ms : set recv timeout mS (default blocking)" << std::endl;
	std::cerr << "default : receive on port" << std::endl;
}

int main(int argc, char **argv)
{
	std::string addr = "";
	int expected_argc = 2;
	long msSleep = SLEEP_MS_DEFAULT;
	long timeout = 0;

	// parse arguments
	for (int i = 1; i < argc; i++) {

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
	}

	// Validate the parameters
	if (argc < expected_argc) {
		print_usage(argv[0]);
		return -1;
	}
	unsigned short port = atoi(argv[argc - 1]);

	SOCKET s;
	w32_wsa_startup();

	if (addr.empty()) {

		// recv mode
		s = w32_udp_socket_create(port);
		if (s <= 0) {
			std::cerr << "Failed creating socket on port:" << port << std::endl;
			return -1;
		}

		char buff[BUFFER_SIZE];
		int ret;
		unsigned short fromPort;
		char fromAddr[INET_ADDRSTRLEN];

		std::cout << "Listening on port:" << port << ". Use [ctrl][c] to stop"<< std::endl;

		while (true) {

			ret = w32_udp_socket_read(s, buff, BUFFER_SIZE, fromAddr, &fromPort, timeout);
			if (ret > 0) {
				std::cout << ret << " bytes from " << fromAddr << ":" << fromPort << " : " << buff << std::endl;
			}
			else {
				if (ret== UDP_SOCK_RET_FAIL) std::cerr << "ret:" << ret << "(Fail)" << std::endl;
				else if (ret == UDP_SOCK_RET_TIMEOUT) std::cerr << "ret:" << ret << "(Timeout)" << std::endl;
				else std::cerr << "ret:" << ret << std::endl;
			}
		}
	}
	else {

		// send mode
		s = w32_udp_socket_create(0);
		if (s <= 0) {
			std::cerr << "Failed creating socket on automatic port" << std::endl;
			return -1;
		}

		char buff[BUFFER_SIZE];
		int ret;
		int counter = 0;

		std::cout << "Sending to " << addr << ":" << port << ". Use [ctrl][c] to stop" << std::endl;

		while (true) {

			snprintf(buff, BUFFER_SIZE, "%i", counter);
			counter++;
			if (counter >= 100000) counter = 0;

			ret = w32_udp_socket_write(s, buff, BUFFER_SIZE, addr.c_str(), port);
			if (ret > 0) {
				std::cout << "Wrote " << ret << " bytes to " << addr << ":" << port << std::endl;
			}
			else {
				std::cerr << "ret:" << ret << std::endl;
			}

			Sleep(msSleep);
		}
	}

	w32_wsa_cleanup();
    return 0;
}

