/*
Copyright(c) <2018> <University of Washington>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// A3P-device-simulator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <iomanip>
#include "..\w32_tcp_socket_test\win32_tcp_socket.h"
#include "..\A3P-1\Arbin3pp.h"

w32_socket *sch1 = NULL;
w32_socket *sch2 = NULL;
bool doRun = true;

void cleanup(){

	w32_tcp_socket_close(&sch1);
	w32_tcp_socket_close(&sch2);
}

// [CTRL][c] handler
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {

	switch (dwCtrlType){
		case CTRL_C_EVENT:
			printf("[Ctrl]+C\n");
			doRun = false;
			return TRUE;
		default:
			// Pass signal on to the next handler
			return FALSE;
	}
}

DWORD WINAPI thread_funct(LPVOID lpParam) {

	w32_socket *s = (w32_socket*)lpParam;

	std::cout << "waiting for client" << std::endl;
	w32_socket *sock = w32_tcp_socket_server_wait(s);

	if (sock!=NULL) {

		std::cout << "client connected on socket " << sock->sock << std::endl;

		while (doRun) {

			char buff[4096];
			int ret;

			ret = w32_tcp_socket_read(sock, buff, 4096);

			if (ret == 0) {

				std::cout << "client disconnected on socket " << sock->sock << std::endl;
				break;

			}
			else if (ret < 0) {

				std::cerr << "w32_tcp_socket_read error on socket "  << sock->sock << std::endl;
				break;

			} else {

				// set system time answer
				if (ret == 44) {
					char b[1];
					*b = 0x06;
					if (w32_tcp_socket_write(sock, b, 1 )==1) std::cout << " 0x06 sent" << std::endl;
					else std::cerr << " cannot send 0x06" << std::endl;
				}


				// TODO: mutex to avoid message superimpositions


				std::cout << "socket " << sock->sock << " received " << std::dec << ret << " bytes" << std::endl;

				for (int i = 0; i < ret; i++) {

					std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)(unsigned char)buff[i] << " ";
					if ((i + 1) % 8 == 0) std::cout << std::endl;
				}
				std::cout << std::endl;
			}
		}
	}
	else {
		std::cerr << "w32_tcp_socket_server_wait error" << std::endl;
	}

	w32_tcp_socket_close(&sock);
	return 0;
}

void print_usage(const char *exe)
{
	std::cerr << "Device simulator for Arbin 3rd party protocol" << std::endl;
	std::cerr << "Open server tcp ports " << A3P_CH1_PORT << " " << A3P_CH2_PORT << std::endl;
	std::cerr << "usage: " << exe << std::endl;
	std::cerr << "  -h: show this help" << std::endl;
}

int main(int argc, char **argv)
{
	for (int i = 1; i < argc; i++) {

		if (!strcmp(argv[i], "-h")) {
			print_usage(argv[0]);
			return 0;
		}
	}

	// intercept CTRL-C
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	std::cout << "opening ch1 server on port:" << A3P_CH1_PORT << std::endl;
	sch1 = w32_tcp_socket_server_create(A3P_CH1_PORT);

	if (!sch1->lasterr.empty()) {
		std::cerr << "w32_tcp_socket_server_create error" << std::endl;
		cleanup();
		return -1;
	}

	std::cout << "opening ch2 server on port:" << A3P_CH2_PORT << std::endl;
	sch2 = w32_tcp_socket_server_create(A3P_CH2_PORT);

	if (!sch2->lasterr.empty()) {
		std::cerr << "w32_tcp_socket_server_create error" << std::endl;
		cleanup();
		return -1;
	}

	HANDLE ch1Thread = CreateThread(0, 0, thread_funct, sch1, 0, NULL);
	HANDLE ch2Thread = CreateThread(0, 0, thread_funct, sch2, 0, NULL);

	while (doRun) Sleep(1000);

	if (WaitForSingleObject(ch1Thread, 2000) == WAIT_TIMEOUT) {
		std::cerr << "ch1Thread timeout" << std::endl;
		TerminateThread(ch1Thread, 0);
	}
	if (WaitForSingleObject(ch2Thread, 2000) == WAIT_TIMEOUT) {
		std::cerr << "ch2Thread timeout" << std::endl;
		TerminateThread(ch1Thread, 0);
	}

	cleanup();
    return 0;
}

