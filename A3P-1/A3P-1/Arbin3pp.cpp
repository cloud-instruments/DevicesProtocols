// Arbin 3rd party protocol functions
// (c)2017 Matteo Lucarelli

#include "stdafx.h"
#include "Arbin3pp.h"

#include <ctime>
#include <queue>

// GLOBAL VARS /////////////////////////////////////////////////////////////////

// TODO: struct for multiple devices
static char gAddr[INET_ADDRSTRLEN];
static std::queue<a3p_msg*> gCh1OutQueue;
static std::queue<a3p_msg*> gCh1InQueue;
static std::queue<a3p_msg*> gCh2InQueue;
static bool gDoThreadCh1;
static HANDLE gThreadCh1;
static bool gDoThreadCh2;
static HANDLE gThreadCh2;
static std::queue<std::string> gMessageQueue;

// CH1 /////////////////////////////////////////////////////////////////////////

int a3p_send(a3p_msg msg) {

	// TODO
	return -1;
}

// NOTE: message must be deleted after use
int a3p_get_ch1(a3p_msg *msg) {

	// TODO
	return -1;
}

DWORD WINAPI a3p_ch1_thread_funct(LPVOID lpParam) {

	// connect device
	SOCKET sock = w32_tcp_socket_client_create(gAddr, A3P_CH1_PORT);
	if (sock == INVALID_SOCKET) {
		// TODO manage error
	}

	// enable keepalive 
	w32_tcp_socket_keepalive(sock, A3P_KEEPALIVE_TIMEOUT_S, 1);

	// TODO: send 3RD_SDU
	// TODO: read MP_CONFIRM

	time_t prevTime=0;

	while (gDoThreadCh1) {

		// each 5s send CMD_SET_SYSTEMTIME
		if (time(NULL)-prevTime >= 5){

			// TODO: send CMD_SET_SYSTEMTIME

			// TODO: recv 0x06

			prevTime = time(NULL);
		}


		//   send data fron queue
		//   read answer in queue


		// TODO: define or parameter
		Sleep(100);

	}

	// TODO: send 3RD_SDU
	// TODO: read MP_CONFIRM

	w32_tcp_socket_close(sock);
	return 0;
}

int a3p_start_ch1_thread() {

	if (gDoThreadCh1) return 0;
	gDoThreadCh1 = true;
	gThreadCh1 = CreateThread(0, 0, a3p_ch1_thread_funct, NULL, 0, NULL);
	return 1;
}

int a3p_stop_ch1_thread() {

	if (!gDoThreadCh1) return 0;
	gDoThreadCh1 = false;

	// TODO: error msg
	if (WaitForSingleObject(gThreadCh1, 2000) == WAIT_TIMEOUT) {
		TerminateThread(gThreadCh1, 0);
	}
	return 1;
}

// CH2 ////////////////////////////////////////////////////////////////////////

// NOTE: message must be deleted after use
int a3p_get_ch2(a3p_msg *msg) {
	// TODO
	return -1;
}

DWORD WINAPI a3p_ch2_thread_funct(LPVOID lpParam) {

	// connect device
	SOCKET sock = w32_tcp_socket_client_create(gAddr, A3P_CH2_PORT);
	if (sock == INVALID_SOCKET) {
		// TODO manage error
	}

	while (gDoThreadCh2) {

		//   receive data from ch2
		//   OBSOLETE: gU16Token msg updates gU16Token value
		//   other msg are put in incoming queue
	}

	w32_tcp_socket_close(sock);
	return 0;
}

int a3p_start_ch2_thread() {

	if (gDoThreadCh2) return 0;
	gDoThreadCh2 = true;
	gThreadCh2 = CreateThread(0, 0, a3p_ch2_thread_funct, NULL, 0, NULL);
	return 1;
}

int a3p_stop_ch2_thread() {

	if (!gDoThreadCh2) return 0;
	gDoThreadCh2 = false;

	// TODO: error msg
	if (WaitForSingleObject(gThreadCh2, 2000) == WAIT_TIMEOUT) {
		TerminateThread(gThreadCh2, 0);
	}
	return 1;
}

// PUBLICS ////////////////////////////////////////////////////////////////////

int a3p_connect(const char* addr) {

	strncpy_s(gAddr, INET6_ADDRSTRLEN, addr, INET6_ADDRSTRLEN);

	// start threads
	a3p_start_ch1_thread();
	a3p_start_ch2_thread();

	return 0;
}

int a3p_disconnect() {

	//( stop threads
	a3p_stop_ch2_thread();
	a3p_stop_ch1_thread();

	return 0;
}