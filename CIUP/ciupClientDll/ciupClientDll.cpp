// ciupClientDll.cpp : Defines the functions for DLL application.
#include "stdafx.h"

#include "ciupClientDll.h"
#include <string>
#include <sstream>
#include <vector>
#include "../w32_udp_socket_test/w32_udp_socket.h"

// json serialization functions ////////////////////////////////////////////////

// convert ciupServerInfo to json string
const char *ciupJsonSerialize(ciupServerInfo d) {

	std::ostringstream oss;

	oss << "{";
	oss << "\"status\":\"" << CIUP_STATUS_DESCR(d.status) << "\",";
	oss << "}";

	return oss.str().c_str();
}

// convert ciupDataPoint to json string
const char *ciupJsonSerialize(ciupDataPoint d) {

	std::ostringstream oss;

	oss << "{";
	oss << "\"Ktemp\":\"" << d.Ktemp << "\",";
	oss << "\"Acurr\":\"" << d.Acurr << "\",";
	oss << "\"Vdiff\":\"" << d.Vdiff << "\",";
	oss << "\"AHcap\":\"" << d.AHcap << "\",";
	oss << "}";

	return oss.str().c_str();
}

// convert payload of msg to jason string
const char *ciupJsonSerialize(const BYTE* msg) {

	switch (*(msg + 4)) {

	case CIUP_MSG_SERVERINFO:{
			// TODO: check payload size == sizeof(d)

			ciupServerInfo d;
			memcpy_s(&d, sizeof(d), msg + 7, sizeof(d));
			return ciupJsonSerialize(d);
		}
		break;

	case CIUP_MSG_DATAPOINT: {
			// TODO: check payload size == sizeof(d)

			ciupDataPoint d;
			memcpy_s(&d, sizeof(d), msg + 7, sizeof(d));
			return ciupJsonSerialize(d);

		}
		break;

	default:
		// TODO: error control
		break;
	}

	return NULL;
}

// message function ////////////////////////////////////////////////////////////

int ciupSendCommand(SOCKET sock, BYTE command, const char *addr, unsigned short port, BYTE* ans, int ans_size) {

	// build command
	void *cmd = ciupBuildMessage(command);

	// send command
	w32_udp_socket_write(sock, cmd, CIUP_MSG_SIZE(0), addr, port);

	// read answer
	int ret = w32_udp_socket_read(sock, ans, ans_size, NULL, NULL, CIUP_ANS_TIMEOUT_MS);

	// socket error - TODO: error descr
	if (ret < 0) return ret;

	// unexpected size - TODO: error descr
	else if (ret != ans_size) return -100;

	// TODO: Control message syntax

	delete[] cmd;
	return 0;
}

// to be used with commands that implies only ACK answer
int ciupSendCommand(SOCKET sock, BYTE command, const char *addr, unsigned short port)
{
	BYTE ack[CIUP_MSG_SIZE(0)];

	int ret = ciupSendCommand(sock, command, addr, port, ack, CIUP_MSG_SIZE(0));

	// TODO: error descr
	if (ret < 0) return ret;

	// unexpected content - TODO: error descr
	//else if (memcmp(ack, cmd, CIUP_MSG_SIZE(0))) return -200;

	return 0;
}

// receiver thread /////////////////////////////////////////////////////////////

typedef struct ciupThreadData_t{
	int id;
	ciupDataCb cb;
	bool run;
	SOCKET sock;
	std::string addr;
	unsigned short port;
	HANDLE hThread;

	ciupThreadData_t(int i, ciupDataCb c, SOCKET s, const char* a, unsigned short p)
		:id(i)
		,cb(c)
		,run(true)
		,sock(s)
		,addr(a)
		,port(p)
	{}

}ciupThreadData;

std::vector<ciupThreadData*> ciupThreadDataList;

// returns the first id not currently used in ciupThreadDataList
int getFreeId() {

	if (ciupThreadDataList.size() >= CIUP_MAX_RECEIVER) return -1;

	bool m[CIUP_MAX_RECEIVER];

	for (std::vector<ciupThreadData*>::iterator it = ciupThreadDataList.begin(); it != ciupThreadDataList.end(); ++it) {
		m[(*it)->id] = true;
	}

	for (int i = 0; i < CIUP_MAX_RECEIVER; i++) {
		if (!m[i]) return i;
	}

	return -1;
}

// find the index in ciupThreadDataList with the required id
int findId(int id) {

	for (std::vector<ciupThreadData*>::iterator it = ciupThreadDataList.begin(); it != ciupThreadDataList.end(); ++it) {
		if ((*it)->id == id) {
			return std::distance(ciupThreadDataList.begin(), it);
		}
	}
	return -1;
}

DWORD WINAPI ciupThreadFunction(LPVOID lpParam) {

	ciupThreadData *d = (ciupThreadData*)lpParam;

	BYTE buff[CIUP_MAX_MSG_SIZE];
	int ret;

	while (d->run) {

		ret = w32_udp_socket_read(d->sock, buff, CIUP_MAX_MSG_SIZE, NULL, NULL, 1000);
		if (ret > 0) {
			
			// TODO: check msg correctness
			
			d->cb(ciupJsonSerialize(buff), d->id);
		}
	}

	return 0;
}

int ciupcStopReceiverWhithIndex(int index) {

	ciupThreadData *d = ciupThreadDataList[index];
	ciupThreadDataList.erase(ciupThreadDataList.begin() + index);

	// TODO: error control
	ciupSendCommand(d->sock, CIUP_MSG_STOP, d->addr.c_str(), d->port);

	d->run = false;

	// join thread
	if (WaitForSingleObject(d->hThread, 2000) == WAIT_TIMEOUT) {
		//std::cerr << "ch1Thread timeout" << std::endl;
		TerminateThread(d->hThread, 0);
	}

	closesocket(d->sock);
	w32_wsa_cleanup();

	delete d;
	return 0;
}

// exported functions //////////////////////////////////////////////////////////

__declspec(dllexport) int __stdcall ciupcStartReceiver(const char *addr, unsigned short port, ciupDataCb cb) {

	w32_wsa_startup();

	ciupThreadData *d = new ciupThreadData(getFreeId(), cb, w32_udp_socket_create(0), addr, port);

	// too much working receivers - TODO: error descr
	if (d->id < 0) return -1;

	// failed open socket - TODO: error descr
	if (d->sock <= 0) return -2;

	// TODO: error control
	ciupSendCommand(d->sock, CIUP_MSG_STOP, addr, port);

	d->hThread = CreateThread(0, 0, ciupThreadFunction, d, 0, NULL);
	ciupThreadDataList.push_back(d);

	return d->id;
}

// return 0 on success
__declspec(dllexport) int __stdcall ciupcStopReceiver(int id) {

	// TODO: error description

	int index = findId(id);
	if (index >= 0) ciupcStopReceiverWhithIndex(index);
	return 0;
}


__declspec(dllexport) void __stdcall ciupcStopAllReceivers() {

	for (int i = ciupThreadDataList.size() - 1; i >= 0; i--)
		ciupcStopReceiverWhithIndex(i);
}

__declspec(dllexport) int __stdcall ciupcGetServerInfo(const char *addr, unsigned short port, char* json, int jsonmaxsize) {

	SOCKET s = w32_udp_socket_create(0);

	// TODO: error descr
	if (s <= 0) return -2;

	BYTE ans[CIUP_MSG_SIZE(sizeof(ciupServerInfo))];

	int ret = ciupSendCommand(s, CIUP_MSG_SERVERINFO, addr, port, ans, CIUP_MSG_SIZE(sizeof(ciupServerInfo)));

	// TODO error descr
	if (ret < 0) return ret;

	strncpy_s(json, jsonmaxsize, ciupJsonSerialize(ans), jsonmaxsize);
	return 0;
}