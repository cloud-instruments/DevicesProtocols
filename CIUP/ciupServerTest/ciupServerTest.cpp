// ciupServerTest.cpp : CIUP server test application

#include "stdafx.h"
#include "../w32_udp_socket_test/w32_udp_socket.h"
#include "../ciupClientDll/ciupCommon.h"
#include "ciupServer.h"
#include <iostream>
#include <vector>

#define SENDER_SLEEP_DEFAULT 1000
int senderSleep = SENDER_SLEEP_DEFAULT;

typedef struct ciupSenderData_t{

	char addr[INET_ADDRSTRLEN];
	unsigned short port;
	SOCKET sock;
	bool run;
	HANDLE handle;

	ciupSenderData_t(const char *a, unsigned short p, SOCKET s){
		strncpy_s(addr, INET_ADDRSTRLEN, a, INET_ADDRSTRLEN);
		port = p;
		sock = s;
		run = true;
	}

}ciupSenderData;

// list of running threads
std::vector<ciupSenderData*> ciupSenderList;

int findSender(const char *addr, unsigned short port) {

	for (std::vector<ciupSenderData*>::iterator it = ciupSenderList.begin(); it != ciupSenderList.end(); ++it) {
		if ( !strcmp(addr, (*it)->addr) && (port == (*it)->port) ){
			return std::distance(ciupSenderList.begin(), it);
		}
	}
	return -1;
}

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

DWORD WINAPI ciupSenderThread(LPVOID lpParam) {

	ciupSenderData *data = (ciupSenderData*)lpParam;

	ciupDataPoint point;
	void *msg;
	int wcount;
	BYTE ack[CIUP_MSG_SIZE(0)];
	int errcount = 0;
	int size;

	while (data->run) {

		point.Acurr = (float)rand() / RAND_MAX;
		point.AHcap = (float)rand() / RAND_MAX;
		point.Ktemp = (float)rand() / RAND_MAX;
		point.Vdiff = (float)rand() / RAND_MAX;

		msg = ciupBuildMessage(CIUP_MSG_DATAPOINT, &point, sizeof(point));
		wcount = w32_udp_socket_write(data->sock, msg, CIUP_MSG_SIZE(sizeof(point)), data->addr, data->port);
		delete[] msg;

		std::cout << "T" << data->handle << " Wrote " << wcount << " bytes to " << data->addr << ":" << data->port << std::endl;

		// read ack as watchdog
		size = w32_udp_socket_read(data->sock, ack, CIUP_MSG_SIZE(0), NULL, NULL, CIUP_ANS_TIMEOUT_MS);

		if (size < 0) {

			std::cerr << "T" << data->handle << " Socket read ack err:" << UDP_SOCK_RET_DESCR(size) << std::endl;
			errcount++;
		}
		else if (size != CIUP_MSG_SIZE(0)) {

			 std::cerr << "T" << data->handle << " Wrong ack size, expected:" << CIUP_MSG_SIZE(0) << " received:" << size << std::endl;
			 errcount++;
		}
		else if (ciupCheckMessageSyntax(ack, size) != CIUP_NO_ERR) {

			std::cerr << "T" << data->handle << " Wrong ack syntax" << std::endl;
			errcount++;
		}
		else {
			errcount = 0;
		}

		if (errcount >= CIUP_SERVER_ERROR_LIMIT) {
			std::cerr << "T" << data->handle << " Error limit reached, stopping sender" << std::endl;
			data->run = false;
		}

		Sleep(senderSleep);
	}

	return 0;
}

int startSender(SOCKET s, const char* addr, unsigned short port) {

	std::cout << "Starting sender for " << addr << ":" << port << std::endl;

	void *ack = ciupBuildMessage(CIUP_MSG_START);
	w32_udp_socket_write(s, ack, CIUP_MSG_SIZE(0), addr, port);
	delete[] ack;

	// verify if sender exists
	if (findSender(addr, port) < 0) {

		// TODO Error control
		ciupSenderData *d = new ciupSenderData(addr, port, w32_udp_socket_create(0));
		ciupSenderList.push_back(d);
		d->handle = CreateThread(0, 0, ciupSenderThread, d, 0, NULL);
	}

	return CIUP_NO_ERR;
}

int stopSender(SOCKET s, const char* addr, unsigned short port) {

	std::cout << "Stopping sender for " << addr << ":" << port << std::endl;

	void *ack = ciupBuildMessage(CIUP_MSG_STOP);
	w32_udp_socket_write(s, ack, CIUP_MSG_SIZE(0), addr, port);
	delete[] ack;

	// on stop stop senddata thread on addr:port
	int index = findSender(addr, port);
	
	if (index>=0){

		ciupSenderData *d = ciupSenderList[index];
		ciupSenderList.erase(ciupSenderList.begin() + index);

		d->run = false;

		// join thread
		if (WaitForSingleObject(d->handle, CIUP_STOP_THREAD_TIMEOUT_MS) == WAIT_TIMEOUT) {
			TerminateThread(d->handle, 0);
		}

		closesocket(d->sock);
		delete d;

		std::cout << "Done" << std::endl;
	}
	else {
		std::cerr << "No server to stop" << std::endl;
	}
	return CIUP_NO_ERR;
}

int sendServerInfo(SOCKET s, const char* addr, unsigned short port) {

	ciupServerInfo d;
	d.status = CIUP_ST_WORKING;

	void *ans = ciupBuildMessage(CIUP_MSG_SERVERINFO, &d, sizeof(d));
	w32_udp_socket_write(s, ans, CIUP_MSG_SIZE(sizeof(d)), addr, port);
	delete[] ans;

	return CIUP_NO_ERR;
}

void print_usage(const char *exe) {

	std::cerr << "Cloud Instruments Unified Protocol server test application" << std::endl;
	std::cerr << "usage: " << exe << " port" << std::endl;
	std::cerr << " -s ms : set sender sleep, default:" << SENDER_SLEEP_DEFAULT << "mS" << std::endl;
}

int main(int argc, char **argv)
{
	int expected_argc = 2;

	// parse arguments
	for (int i = 1; i < argc; i++) {

		// set sender sleep
		if (!strcmp(argv[i], "-s")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			senderSleep = atoi(argv[i + 1]);
			expected_argc += 2;
		}
	}

	// Validate the parameters
	if (argc < expected_argc) {
		print_usage(argv[0]);
		return -1;
	}

	w32_wsa_startup();

	// open socket
	unsigned short port = atoi(argv[argc - 1]);
	SOCKET s = w32_udp_socket_create(port);
	if (s <= 0) {
		std::cerr << "Cannot open UDP port " << port << std::endl;
		return -1;
	}

	std::cerr << "Listening un UDP port " << port << ", [ctrl][c] to stop" << std::endl;
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	BYTE cmd[CIUP_MAX_MSG_SIZE];
	void *ans;
	char inAddr[INET_ADDRSTRLEN];
	unsigned short inPort;

	// while run listen on socket
	while (run) {

		// read socket 
		int size = w32_udp_socket_read(s, cmd, CIUP_MAX_MSG_SIZE, inAddr, &inPort, 1000);

		if (size < 0) {

			if (size != UDP_SOCK_RET_TIMEOUT) std::cerr << "Socket err:" << UDP_SOCK_RET_DESCR(size) << std::endl;
		}
		else if (size != CIUP_MSG_SIZE(0)) {

			std::cerr << "Wrong size for incoming cmd, expected:" << CIUP_MSG_SIZE(0) << " received:" << size << std::endl;
		}
		else if (ciupCheckMessageSyntax(cmd,size)!=CIUP_NO_ERR){

			std::cerr << "Wrong syntax for incoming cmd" << std::endl;
		}
		else {

			// parse command

			switch (*(cmd + CIUP_TYPE_POS)) {

			case CIUP_MSG_SERVERINFO:
				std::cout << "Received SERVERINFO command from" << inAddr << ":" << inPort << std::endl;
				sendServerInfo(s, inAddr, inPort);
				break;

			case CIUP_MSG_START:
				std::cout << "Received START command from " << inAddr << ":" << inPort << std::endl;
				startSender(s, inAddr, inPort);
				break;

			case CIUP_MSG_STOP:
				std::cout << "Received STOP command from " << inAddr << ":" << inPort << std::endl;
				stopSender(s, inAddr, inPort);
				break;

			default:
				std::cerr << "Unknown message type:" << *(cmd + CIUP_TYPE_POS) << std::endl;
				break;
			}
		}
	}

	closesocket(s);
	w32_wsa_cleanup();
    return 0;
}

