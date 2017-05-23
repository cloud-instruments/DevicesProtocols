// w32_udp_socket_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "w32_udp_socket.h"

#include <string>
#include <iostream>

void print_usage(const char *exe) {

	std::cerr << "usage: " << exe << " [-c server-ip] port" << std::endl;
	std::cerr << "-c ip : send to server-ip:port" << std::endl;

	// TODO: recv timeout, send sleep

	std::cerr << "default : receive on port" << std::endl;
}

int main(int argc, char **argv)
{
	std::string addr = "";
	int expected_argc = 2;

	// client flag
	for (int i = 1; i < argc; i++) {

		if (!strcmp(argv[i], "-c")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			addr = argv[i + i];
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

		char buff[32];
		int ret;
		unsigned short fromPort;
		char fromAddr[INET_ADDRSTRLEN];

		while (true) {

			ret = w32_udp_socket_read(s, buff, 16, fromAddr, &fromPort, 0);
			if (ret > 0) {
				std::cout << ret << " bytes from " << fromAddr << ":" << fromPort << std::endl;
			}
			else {
				std::cerr << "ret:" << ret << std::endl;
			}
		}
	}
	else {

		// send mode
		s = w32_udp_socket_create(0);

		char buff[32] = "Test w32_udp_socket";
		int ret;

		while (true) {

			ret = w32_udp_socket_write(s, buff, strlen(buff), addr.c_str(), port);
			if (ret > 0) {
				std::cout << ret << " bytes wrote to " << addr << ":" << port << std::endl;
			}
			else {
				std::cerr << "ret:" << ret << std::endl;
			}

			Sleep(1000);
		}
	}

	w32_wsa_cleanup();
    return 0;
}

