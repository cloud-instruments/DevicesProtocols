// A3P-device-simulator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include "../A3P-1/win32_tcp_socket.h"
#include "../A3P-1/Arbin3pp.h"

SOCKET sch1 = INVALID_SOCKET;
SOCKET sch2 = INVALID_SOCKET;
bool doRun = true;

void cleanup()
{
	w32_tcp_socket_close(sch1);
	w32_tcp_socket_close(sch2);
}

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {

	switch (dwCtrlType)
	{
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

	SOCKET s = *((SOCKET*)lpParam);

	std::cout << s << std::endl;

	while (doRun) {

		char buff[2096];
		int ret;

		ret = w32_tcp_socket_read(s, buff, 2096);

		if (ret == 0) {

			std::cout << "peer disconnected" << std::endl;
			return -1;

		}
		else if (ret<0) {

			std::cerr << "w32_tcp_socket_read error" << std::endl;
			return -2;

		}
		else {

			// TODO: show channel
			// TODO: mutex to avoid message superimpositions

			printf("Received %i bytes:\n", ret);
			for (unsigned int i = 0; i < ret; i++) {

				printf("%02X ", buff[i]);
				if ((i + 1) % 8 == 0) printf("\n");
			}
		}
	}

	return 0;
}

int main()
{
	// intercept CTRL-C
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	std::cout << "opening server on port:" << A3P_CH1_PORT << std::endl;
	sch1 = w32_tcp_socket_server_create(A3P_CH1_PORT);

	if (sch1 == INVALID_SOCKET) {
		std::cerr << "w32_tcp_socket_server_create error" << std::endl;
		cleanup();
		return -1;
	}

	std::cout << "opening server on port:" << A3P_CH2_PORT << std::endl;
	sch2 = w32_tcp_socket_server_create(A3P_CH2_PORT);

	if (sch2 == INVALID_SOCKET) {
		std::cerr << "w32_tcp_socket_server_create error" << std::endl;
		cleanup();
		return -1;
	}

	// run two thread to receive data from both channels
	HANDLE ch1Thread;
	HANDLE ch2Thread;

	while (doRun) {

		// TODO: wait connection
		// print messages

		/*HANDLE ch1Thread; = CreateThread(0, 0, thread_funct, &sch1, 0, NULL);
		HANDLE ch2Thread; = CreateThread(0, 0, thread_funct, &sch2, 0, NULL);
		WaitForSingleObject(ch1Thread, 2000);
		WaitForSingleObject(ch2Thread, 2000);*/
	}

	/*if (WaitForSingleObject(ch1Thread, 2000) == WAIT_TIMEOUT) {
		std::cerr << "ch1Thread timeout" << std::endl;
		TerminateThread(ch1Thread, 0);
	}
	if (WaitForSingleObject(ch2Thread, 2000) == WAIT_TIMEOUT) {
		std::cerr << "ch2Thread timeout" << std::endl;
		TerminateThread(ch1Thread, 0);
	}*/

	cleanup();
    return 0;
}

