// ciupServerTest.cpp : CIUP server test application

#include "stdafx.h"
#include "../w32_udp_socket_test/w32_udp_socket.h"
#include "../ciupClientDll/ciupCommon.h"
#include "ciupServer.h"
#include <iostream>

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

DWORD WINAPI datasend_thread(LPVOID lpParam) {

	//w32_socket *s = (w32_socket*)lpParam;
	SOCKET s = w32_udp_socket_create(0);

	if (s <= 0) {
		std::cerr << "Cannot create output socket";
		return -1;
	}

	ciupDataPoint d;
	void *msg;

	while (run) {

		d.Acurr = (float)rand() / RAND_MAX;
		d.AHcap = (float)rand() / RAND_MAX;
		d.Ktemp = (float)rand() / RAND_MAX;
		d.Vdiff = (float)rand() / RAND_MAX;

		msg = ciupBuildMessage(CIUP_MSG_DATAPOINT, &d, sizeof(d));

		// TODO: send data

		Sleep(500);

		// TODO: read ack as watchdog

		delete[] msg;
	}

	return 0;
}

void print_usage(const char *exe) {

	std::cerr << "Cloud Instruments Unified Protocol server test application" << std::endl;
	std::cerr << "usage: " << exe << " port" << std::endl;
	// TODO: send sleep ms 
}

int main(int argc, char **argv)
{
	int expected_argc = 2;

	// Validate the parameters
	if (argc < expected_argc) {
		print_usage(argv[0]);
		return -1;
	}

	// open socket
	unsigned short port = atoi(argv[argc - 1]);
	SOCKET s = w32_udp_socket_create(port);
	if (s <= 0) {
		std::cerr << "Cannot open UDP port " << port << std::endl;
		return -1;
	}

	std::cerr << "Listening un UDP port " << port << "[ctrl][c] to stop" << std::endl;
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	// while run listen on socket
	while (run) {

		// read socket 

		// reply to serverinfo

		// on start start senddata thread for addr:port
		// HANDLE ch1Thread = CreateThread(0, 0, datasend_thread, thread_data, 0, NULL);

		// on stop stop senddata thread on addr:port
		//if (WaitForSingleObject(ch1Thread, 2000) == WAIT_TIMEOUT) {
		//	std::cerr << "ch1Thread timeout" << std::endl;
		//	TerminateThread(ch1Thread, 0);
		//}

	}


	closesocket(s);
    return 0;
}

