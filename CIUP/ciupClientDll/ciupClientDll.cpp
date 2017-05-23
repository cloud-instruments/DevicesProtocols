// ciupClientDll.cpp : Defines the functions for DLL application.
#include "stdafx.h"

#include "CIUPclientDLL.h"
#include <string>
#include <sstream>
#include <vector>

// json serialization functions ////////////////////////////////////////////////

const char *ciupJsonSerialize(ciupServerInfo d) {

	std::ostringstream oss;

	oss << "{";
	oss << "\"status\":\"" << CIUP_STATUS_DESCR(d.status) << "\",";
	oss << "}";

	return oss.str().c_str();
}

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

// exported functions //////////////////////////////////////////////////////////

__declspec(dllexport) int __stdcall ciupcGetServerInfo(const char *addr, unsigned short port, char* json, int jsonmaxsize) {

	ciupServerInfo info;

	// TODO: send CIUP_MSG_SERVERINFO
	// TODO: read answer
	// TODO: parse answer payload in info struct

	strncpy_s(json,jsonmaxsize, ciupJsonSerialize(info),jsonmaxsize);
	return 0;
}

// receiver thread /////////////////////////////////////////////////////////////

typedef struct ciupThreadData_t{
	int id;
	ciupDataCb cb;
	bool run;
	std::string addr;
	unsigned short port;
	HANDLE h;

	ciupThreadData_t(int i, ciupDataCb c, const char* a, unsigned short p)
		:id(i)
		,cb(c)
		,run(true)
		,addr(a)
		,port(p)
	{}

}ciupThreadData;

std::vector<ciupThreadData*> ciupThreadDataList;

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

	// TODO

	return 0;
}

__declspec(dllexport) int __stdcall ciupcStartReceiver(const char *addr, unsigned short port, ciupDataCb cb) {

	// TODO: send start command to server


	ciupThreadData *d=new ciupThreadData(getFreeId(),cb,addr,port);

	// too much working receivers
	if (d->id < 0) return -1;

	d->h = CreateThread(0, 0, ciupThreadFunction, d, 0, NULL);
	ciupThreadDataList.push_back(d);

	return d->id;
}

// return 0 on success
__declspec(dllexport) int __stdcall ciupcStopReceiver(int id) {

	// TODO
	int index = findId(id);
	if (index >= 0) {

		ciupThreadData *d = ciupThreadDataList[index];

		// TODO: send stop command to server


		d->run = false;

		// join thread
		if (WaitForSingleObject(d->h, 2000) == WAIT_TIMEOUT) {
			//std::cerr << "ch1Thread timeout" << std::endl;
			TerminateThread(d->h, 0);
		}

		ciupThreadDataList.erase(ciupThreadDataList.begin()+index);
		delete d;
	}

	return 0;
}

__declspec(dllexport) void __stdcall ciupcStopAllReceivers() {

	// TODO: stop all

}

