// ciupClientDll.cpp : Defines the functions for DLL application.
#include "stdafx.h"

#include "ciupClientDll.h"
#include <string>
#include <sstream>
#include <vector>
#include "../w32_udp_socket_test/w32_udp_socket.h"

// error management ////////////////////////////////////////////////////////////

// TODO: thread safe version

std::string gLastErrDescr;
int gLastErrCode;

template< typename ... Args > void ciupSetError(int code, Args const& ... args)
{
	std::ostringstream stream;
	using List = int[];
	(void)List { 0, ((void)(stream << args), 0) ...};

	gLastErrDescr = stream.str();
	gLastErrCode = code;
}

__declspec(dllexport) int __stdcall ciupcGetLastError(char *descr, int maxlen) {

	if (descr) strncpy_s(descr, maxlen, gLastErrDescr.c_str(), maxlen);
	return gLastErrCode;
}

// json serialization functions ////////////////////////////////////////////////

// convert ciupServerInfo to json string
void ciupJsonSerialize(ciupServerInfo d, std::string &ret) {

	std::ostringstream oss;

	oss << "{";
	oss << "\"status\":\"" << CIUP_STATUS_DESCR(d.status) << "\",";
	oss << "}";

	ret = oss.str();
}

// convert ciupDataPoint to json string
void ciupJsonSerialize(ciupDataPoint d, std::string &ret) {

	std::ostringstream oss;

	oss << "{";
	oss << "\"Ktemp\":\"" << d.Ktemp << "\",";
	oss << "\"Acurr\":\"" << d.Acurr << "\",";
	oss << "\"Vdiff\":\"" << d.Vdiff << "\",";
	oss << "\"AHcap\":\"" << d.AHcap << "\",";
	oss << "}";

	ret = oss.str();
}

// convert payload of msg to jason string
int ciupJsonSerialize(const BYTE* msg, std::string &res) {

	int ret = CIUP_NO_ERR;

	switch (*(msg + 4)) {

		case CIUP_MSG_SERVERINFO: 
		{
			ciupServerInfo d;

			// check payload size
			if (CIUP_PAYLOAD_SIZE(msg) != sizeof(d)) {
				ciupSetError(CIUP_ERR_SIZE_MISMATCH, "Wrong payload size for MSG_SERVERINFO expected:", sizeof(d), " received:", CIUP_PAYLOAD_SIZE(msg));
				ret = CIUP_ERR_SIZE_MISMATCH;
			}

			memcpy_s(&d, sizeof(d), msg + CIUP_PAYLOAD_POS, sizeof(d));
			ciupJsonSerialize(d, res);
		}
		break;

		case CIUP_MSG_DATAPOINT: 
		{
			ciupDataPoint d;

			// check payload size
			if (CIUP_PAYLOAD_SIZE(msg) != sizeof(d)) {
				ciupSetError(CIUP_ERR_SIZE_MISMATCH, "Wrong payload size for MSG_DATAPOINT expected:", sizeof(d), " received:", CIUP_PAYLOAD_SIZE(msg));
				ret = CIUP_ERR_SIZE_MISMATCH;
			}

			memcpy_s(&d, sizeof(d), msg + CIUP_PAYLOAD_POS, sizeof(d));
			ciupJsonSerialize(d,res);
		}
		break;

		default:
			ciupSetError(CIUP_ERR_UNKNOWN_TYPE, "Unknown message type:", *(msg + 4));
			ret = CIUP_ERR_UNKNOWN_TYPE;
			break;
	}

	return ret;
}

// message function ////////////////////////////////////////////////////////////

int ciupSendCommand(SOCKET sock, BYTE command, const char *addr, unsigned short port, BYTE* ans, int ans_size) {

	int ret = CIUP_NO_ERR;

	// build command
	void *cmd = ciupBuildMessage(command);

	// send command
	w32_udp_socket_write(sock, cmd, CIUP_MSG_SIZE(0), addr, port);

	// read answer
	int size = w32_udp_socket_read(sock, ans, ans_size, NULL, NULL, CIUP_ANS_TIMEOUT_MS);

	if (size < 0) {
		
		ciupSetError(CIUP_ERR_SOCKET, "Socket err:", UDP_SOCK_RET_DESCR(size), "for cmd:", command);
		ret = CIUP_ERR_SOCKET;

	}
	else if (size != ans_size) {

		ciupSetError(CIUP_ERR_SIZE_MISMATCH, "Wrong answer size for cmd:", command," expected:", ans_size, " received:", size);
		ret = CIUP_ERR_SIZE_MISMATCH;
	}
	else if (size == CIUP_MSG_SIZE(0)){

		// ack expected 
		if (memcmp(ans, cmd, CIUP_MSG_SIZE(0))) {

			ciupSetError(CIUP_ERR_ACK, "Wrong ack for cmd:", command);
			ret = CIUP_ERR_ACK;
		}
	}
	else {

		if (ciupCheckMessageSyntax(ans, size) != CIUP_NO_ERR) {

			ciupSetError(CIUP_ERR_SYNTAX, "Wrong answer syntax for cmd:", command);
			ret = CIUP_ERR_SYNTAX;
		}
	}

	delete[] cmd;
	return ret;
}

// to be used with commands that implies only ACK answer
int ciupSendCommand(SOCKET sock, BYTE command, const char *addr, unsigned short port)
{
	BYTE ack[CIUP_MSG_SIZE(0)];
	return ciupSendCommand(sock, command, addr, port, ack, CIUP_MSG_SIZE(0));
}

// receiver thread /////////////////////////////////////////////////////////////

typedef struct ciupThreadData_t{

	int id;
	ciupDataCb dataCb;
	ciupErrorCb errorCb;
	bool run;
	SOCKET sock;
	std::string addr;
	unsigned short port;
	HANDLE hThread;

	ciupThreadData_t(int i, ciupDataCb c, ciupErrorCb e, SOCKET s, const char* a, unsigned short p)
		:id(i)
		,dataCb(c)
		,errorCb(e)
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
	std::string json;

	while (d->run) {

		ret = w32_udp_socket_read(d->sock, buff, CIUP_MAX_MSG_SIZE, NULL, NULL, 1000);
		if (ret > 0) {

			// TODO: ack as watchdog
			
			if (ciupCheckMessageSyntax(buff, ret) == CIUP_NO_ERR) {

				if (ciupJsonSerialize(buff, json) == CIUP_NO_ERR) {

					// send message to data calback
					d->dataCb(json.c_str(), d->id);
				}
				else {
					d->errorCb(CIUP_ERR_SYNTAX, "Cannot serialize incoming message", d->id);
				}
			}
			else {
				
				// send error to error callback
				d->errorCb(CIUP_ERR_SYNTAX, "Syntax error in incoming message", d->id);
			}
		}
	}

	return 0;
}

int ciupcStopReceiverWhithIndex(int index) {

	ciupThreadData *d = ciupThreadDataList[index];
	ciupThreadDataList.erase(ciupThreadDataList.begin() + index);

	int ret = ciupSendCommand(d->sock, CIUP_MSG_STOP, d->addr.c_str(), d->port);

	d->run = false;

	// join thread
	if (WaitForSingleObject(d->hThread, CIUP_STOP_THREAD_TIMEOUT_MS) == WAIT_TIMEOUT) {
		TerminateThread(d->hThread, 0);
	}

	closesocket(d->sock);
	w32_wsa_cleanup();

	delete d;
	return ret;
}

// exported functions //////////////////////////////////////////////////////////

__declspec(dllexport) int __stdcall ciupcStartReceiver(const char *addr, unsigned short port, ciupDataCb dataCb, ciupErrorCb errorCb) {

	w32_wsa_startup();

	int id = getFreeId();

	// TODO: that's not thread safe
	if (id < 0) {
		ciupSetError(CIUP_ERR_MAX_RECEIVERS, "Cannot allocate more receviers");
		return CIUP_ERR_MAX_RECEIVERS;
	}

	SOCKET s = w32_udp_socket_create(0);

	if (s <= 0) {
		ciupSetError(CIUP_ERR_SOCKET, "Cannot create socket");
		return CIUP_ERR_SOCKET;
	}

	int ret = ciupSendCommand(s, CIUP_MSG_START, addr, port);
	if (ret != CIUP_NO_ERR) return ret;

	ciupThreadData *d = new ciupThreadData(id, dataCb, errorCb, s, addr, port);
	ciupThreadDataList.push_back(d);
	d->hThread = CreateThread(0, 0, ciupThreadFunction, d, 0, NULL);

	return id;
}

// return 0 on success
__declspec(dllexport) int __stdcall ciupcStopReceiver(int id) {

	int ret = CIUP_NO_ERR;

	int index = findId(id);
	if (index >= 0) {
		ret = ciupcStopReceiverWhithIndex(index);
	}
	else {		
		ciupSetError(CIUP_ERR_ID, "Cannot stop receiver, id:", id, " not in list");
		ret = CIUP_ERR_ID;
	}
	return ret;
}

__declspec(dllexport) void __stdcall ciupcStopAllReceivers() {

	for (int i = ciupThreadDataList.size() - 1; i >= 0; i--) {
		ciupcStopReceiverWhithIndex(i);
	}
}

__declspec(dllexport) int __stdcall ciupcGetServerInfo(const char *addr, unsigned short port, char* json, int jsonmaxsize) {

	SOCKET s = w32_udp_socket_create(0);

	// TODO: error descr
	if (s <= 0) return CIUP_ERR_SOCKET;

	BYTE ans[CIUP_MSG_SIZE(sizeof(ciupServerInfo))];

	int ret = ciupSendCommand(s, CIUP_MSG_SERVERINFO, addr, port, ans, CIUP_MSG_SIZE(sizeof(ciupServerInfo)));

	// TODO error descr
	if (ret == CIUP_NO_ERR) {

		std::string json_ret;
		ret = ciupJsonSerialize(ans,json_ret);
		if (ret == CIUP_NO_ERR) {
			strncpy_s(json, jsonmaxsize, json_ret.c_str(), jsonmaxsize);
		}
	}

	return ret;
}