// w32_tcp_socket_test.cpp
// test exe for win32_tcp_socket.cpp/.h
// (c)2017 Matteo Lucarelli
// 20170510 v0.1 : initial version

#include "stdafx.h"
#include "win32_tcp_socket.h"
#include <string>
#include <iostream>

bool show_recv(w32_socket *s, bool hex) {

	char buff[256];
	int ret;
	
	ret = w32_tcp_socket_read(s, buff, 255);

	if (ret == 0) {

		std::cout << "peer disconnected" << std::endl;
		return false;

	} else if (ret<0) {

		std::cerr << "w32_tcp_socket_read error" << std::endl;
		return false;

	} else {

		if (hex) {

			printf("received:\n");
			for (int i = 0; i < ret; i++) {

				printf("%02X ", (unsigned char)buff[i]);
				if ((i + 1) % 8 == 0) printf("\n");
			}
			printf("\n");

		} else {

			*(buff + ret) = 0;
			std::cout << "received:" << buff << std::endl;

		}
	}

	return true;
}

bool get_send(w32_socket *s) {

	std::string buff;
	std::cout << "string to send ('q' to stop):";
	std::cin >> buff;

	if (buff=="q") return false;

	// write
	int ret = w32_tcp_socket_write(s, buff.c_str(), buff.size());

	if (ret<0) {

		std::cerr << "w32_tcp_socket_write error" << std::endl;
		return false;

	} else if (ret != buff.size()) {

		std::cerr << "uncomplete write, required:" << buff.size() << "wrote:" << ret << std::endl;
	}

	return true;
}

void print_usage(const char *exe) {

	std::cerr << "usage: " << exe << " [-c server-ip] [-k] [-s] port" << std::endl; 
	std::cerr << "-c server-ip : connect as client to server-ip:port" << std::endl;
	std::cerr << "-k : enable keepalive" << std::endl;
	std::cerr << "-s : run in send mode" << std::endl;
	std::cerr << "-b : HEX output mode (for listen mode)" << std::endl;
	std::cerr << "default : open server on port in listen mode " << std::endl;
}

int main(int argc, char **argv)
{
	std::string server="";
	int expected_argc = 2;
	bool enable_keepalive = false;
	bool enable_send = false;
	bool hex_output = false;

	// client flag
	for (int i = 1; i < argc; i++) {

		if (!strcmp(argv[i], "-c")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			server = argv[i + 1];
			expected_argc += 2;
		}

		if (!strcmp(argv[i], "-k")) {
			enable_keepalive = true;
			expected_argc++;
		}

		if (!strcmp(argv[i], "-s")) {
			enable_send = true;
			expected_argc++;
		}

		if (!strcmp(argv[i], "-b")) {
			hex_output = true;
			expected_argc++;
		}
	}

	// Validate the parameters
	if (argc < expected_argc) {
		print_usage(argv[0]);
		return -1;
	}

	unsigned short port = atoi(argv[argc - 1]);

	if (server!="") {

		w32_socket *sock;

		std::cout << "connecting server " << server << ":" << port << std::endl;
		sock = w32_tcp_socket_client_create(server.c_str(), port);

		if (!sock->lasterr.empty()) {

			std::cerr << "ERROR w32_tcp_socket_client_create:" << sock->lasterr << std::endl;

		} else {

			if (enable_keepalive) {
				if (w32_tcp_socket_keepalive(sock, 5, 1) == 0) {
					std::cout << "keepalive enabled" << std::endl;
				} else {
					std::cerr << "ERROR w32_tcp_socket_keepalive:" << sock->lasterr << std::endl;
				}
			}

			if (!enable_send) std::cout << "server connected" << std::endl;

			bool repeat = true;
			while (repeat) {

				if (enable_send) repeat = get_send(sock);
				else repeat = show_recv(sock,hex_output);
			}

			w32_tcp_socket_close(&sock);
		}

	} else {

		w32_socket *sock, *c_sock;

		std::cout << "opening server on port:" << port << std::endl;

		sock = w32_tcp_socket_server_create(port);

		if (!sock->lasterr.empty() ) {

			std::cerr << "w32_tcp_socket_server_create error" << std::endl;

		} else {

			std::cout << "SOCKET:" << sock->sock << std::endl;

			while (true) {

				std::cout << "waiting new client" << std::endl;

				// accept
				c_sock = w32_tcp_socket_server_wait(sock);

				if (c_sock!=NULL) {

					std::cout << "client connected" << std::endl;
					std::cout << "SOCKET:" << c_sock->sock << std::endl;

					if (enable_keepalive) {
						if (w32_tcp_socket_keepalive(c_sock, 5, 1) == 0) {
							std::cout << "keepalive enabled" << std::endl;
						}
						else {
							std::cerr << "w32_tcp_socket_keepalive error" << std::endl;
						}
					}

					bool repeat = true;
					while (repeat) {

						if (enable_send) repeat = get_send(c_sock);
						else repeat = show_recv(c_sock,hex_output);
					}

					w32_tcp_socket_close(&c_sock);
				}
			}
		}

		w32_tcp_socket_close(&sock);
	}

    return 0;
}

