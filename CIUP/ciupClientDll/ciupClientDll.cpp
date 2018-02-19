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

// ciupClientDll.cpp : dll for ciup client functions

#include "stdafx.h"

#include "ciupClientDll.h"
#include <string>
#include <sstream>
#include <queue>
#include "../w32_tcp_socket_test/win32_tcp_socket.h"
#include <process.h>

// json serialization functions ////////////////////////////////////////////////

// convert ciupServerInfo to json string
void ciupJsonSerialize(ciupServerInfo d, std::string &ret) {

	std::ostringstream oss;

	oss << "{";
	oss << "\"id\":\"" << d.id << "\",";
	oss << "\"status\":\"" << CIUP_STATUS_DESCR(d.status) << "\",";
	oss << "\"mode\":\"" << (int)(d.mode) << "\"";
	oss << "}";

	ret = oss.str();
}

// convert ciupDataPoint to json string
void ciupJsonSerialize(ciupDataPoint d, std::string &ret) {

	std::ostringstream oss;

	oss << "{";
	oss << "\"counter\":\"" << d.counter << "\",";
	oss << "\"channel\":\"" << d.channel << "\",";
	oss << "\"Cycle\":\"" << d.Cycle << "\",";
	oss << "\"TestTime\":\"" << d.TestTime << "\",";
	oss << "\"StepTime\":\"" << d.StepTime << "\",";
	oss << "\"Step\":\"" << d.Step << "\",";
	//oss << "\"RF1\":\"" << d.RF1 << "\",";
	//oss << "\"RF2\":\"" << d.RF2 << "\",";
	oss << "\"Current\":\"" << d.Current << "\",";
	oss << "\"Voltage\":\"" << d.Voltage << "\",";
	oss << "\"Capacity\":\"" << d.Capacity << "\",";
	oss << "\"LHCCapacity\":\"" << d.LHCCapacity << "\",";
	oss << "\"HCCapacity\":\"" << d.HCCapacity << "\",";
	oss << "\"Energy\":\"" << d.Energy << "\",";
	oss << "\"HCEnergy\":\"" << d.HCEnergy << "\",";
	oss << "\"LHCEnergy\":\"" << d.LHCEnergy << "\"";
	oss << "}";

	ret = oss.str();
}

// convert payload of msg to json string
int ciupJsonSerialize(const BYTE* msg, std::string &res) {

	int ret = CIUP_NO_ERR;

	switch (*(msg + CIUP_TYPE_POS)) {

	case CIUP_MSG_SERVERINFO:
	{
		ciupServerInfo d;

		// check payload size
		if (CIUP_PAYLOAD_SIZE(msg) != sizeof(d)) {
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
			ret = CIUP_ERR_SIZE_MISMATCH;
		}

		memcpy_s(&d, sizeof(d), msg + CIUP_PAYLOAD_POS, sizeof(d));
		ciupJsonSerialize(d, res);
	}
	break;

	default:
		ret = CIUP_ERR_UNKNOWN_TYPE;
		break;
	}

	return ret;
}

// connections threads /////////////////////////////////////////////////////////

typedef struct ciupConnectionData_t {

	ciupDataCb dataCb = 0;
	ciupErrorCb errorCb = 0;
	w32_socket *sock = NULL;
	char addr[INET_ADDRSTRLEN] = {};
	unsigned short port = 0;
	bool run = false;
	HANDLE hThread = 0;
	std::queue<BYTE> commands;

	void setup(ciupDataCb c, ciupErrorCb e, w32_socket *s, const char *a, unsigned short p)
	{
		dataCb = c;
		errorCb = e;
		sock = s;
		strncpy_s(addr, a, INET_ADDRSTRLEN);
		port = p;
		run = true;
	}

	void empty()
	{
		dataCb = NULL;
		errorCb = NULL;
		sock = NULL;
		*addr = 0;
		port = 0;
		run = false;
		hThread = 0;
		std::queue<BYTE>().swap(commands);
	}

	bool isEmpty() {
		if (dataCb != NULL) return false;
		if (errorCb != NULL) return false;
		if (sock != NULL) return false;
		if (*addr != 0) return false;
		if (port != 0) return false;
		if (run != false) return false;
		if (hThread != 0) return false;
		if (!commands.empty()) return false;
		return true;
	}

}ciupConnectionData;

static bool csConnectionsListInitialized = false;
static CRITICAL_SECTION csConnectionsList;
static ciupConnectionData ciupConnectionsList[CIUP_MAX_CONNECTIONS];

// returns the first place not currently used in ciupReceiverList
int getFreeId() {

	for (int i = 0; i < CIUP_MAX_CONNECTIONS; i++) {
		if (ciupConnectionsList[i].isEmpty()) return i;
	}
	return CIUP_ERR_MAX_CONNECTIONS;
}

// send log to callback using varags
template< typename ... Args > void ciupError(int id, int code, Args const& ... args)
{
	std::ostringstream stream;
	using List = int[];
	(void)List {
		0, ((void)(stream << args), 0) ...
	};

	ciupConnectionsList[id].errorCb(code, stream.str().c_str(), id);
}

void resetChCount(int *counters)
{
	for (int i = 0; i < CIUP_CH_MAX_COUNT; i++) counters[i] = -1;
}

DWORD WINAPI ciupConnectionThread(LPVOID lpParam) {

	int id = (int)lpParam;

	BYTE buff[CIUP_MAX_MSG_SIZE];
	int ret;
	std::string json;

	int rErr = 0;
	int wErr = 0;

	int chCount[CIUP_CH_MAX_COUNT];
	resetChCount(chCount);

	while (ciupConnectionsList[id].run) {

		// send command if required
		if (!ciupConnectionsList[id].commands.empty()) {

			BYTE c = ciupConnectionsList[id].commands.front();
			void *cmd = ciupBuildMessage(c);

			int wsize = w32_tcp_socket_write(ciupConnectionsList[id].sock, cmd, CIUP_MSG_SIZE(0), 1);
			if (wsize != CIUP_MSG_SIZE(0)) {

				// write error
				ciupError(id, CIUP_ERR_SOCKET, "w32_tcp_socket_write size:", wsize);
				wErr++;
			}
			else {

				// write ok
				ciupConnectionsList[id].commands.pop();
				wErr = 0;
			}
		}

		// read socket data
		ret = w32_tcp_socket_read(ciupConnectionsList[id].sock, buff, CIUP_MAX_MSG_SIZE, 100);

		if (ret == 0) {
			// TIMEOUT: no data
			Sleep(10);
		}
		else if (ret < 0) {

			// socket error
			ciupError(id, CIUP_ERR_SOCKET, "w32_tcp_socket_read error:", ret);
			rErr++;
		}
		else if (ciupCheckMessageSyntax(buff, ret) != CIUP_NO_ERR) {

			// syntax error
			ciupError(id, CIUP_ERR_SOCKET, "syntax error in incoming message");
			rErr++;
		}
		else if (ciupJsonSerialize(buff, json) != CIUP_NO_ERR) {

			// serialization error
			ciupError(id, CIUP_ERR_SYNTAX, "serialize error in incoming message");
			rErr++;
		}
		else {

			// control sequence for datapoints
			bool seqError = false;
			if (*(buff + CIUP_TYPE_POS) == CIUP_MSG_DATAPOINT) {

				USHORT counter = *((USHORT*)(buff + CIUP_PAYLOAD_POS));
				USHORT channel = *((USHORT*)(buff + CIUP_PAYLOAD_POS + 2));

				if (chCount[channel] != -1) {
					int expected = (chCount[channel] + 1) % USHRT_MAX;
					if (counter != expected) {
						ciupError(id, CIUP_ERR_SYNTAX, "sequence, expected:", expected, " received:", counter);
					}

				}
				chCount[channel] = counter;
			}

			// message is ok
			ciupConnectionsList[id].dataCb(*(buff + CIUP_TYPE_POS), json.c_str(), id);
		}

		// check error count
		if ((wErr >= CIUP_SOCKET_ERROR_LIMIT) || (rErr >= CIUP_SOCKET_ERROR_LIMIT)) {

			ciupError(id, CIUP_ERR_SOCKET, "Socket error limit reached, closing connection");
			w32_tcp_socket_close(&(ciupConnectionsList[id].sock));
			wErr = 0;
			rErr = 0;
			resetChCount(chCount);
		}

		// if required reconnect
		while (ciupConnectionsList[id].run && (ciupConnectionsList[id].sock == NULL)) {

			ciupConnectionsList[id].sock = w32_tcp_socket_client_create(ciupConnectionsList[id].addr, ciupConnectionsList[id].port);
			if (!ciupConnectionsList[id].sock->lasterr.empty()) {
				w32_tcp_socket_close(&(ciupConnectionsList[id].sock));
				Sleep(100);
			}
			else {
				ciupcStart(id);
			}
		}
		Sleep(0);
	}

	return 0;
}

// exported functions //////////////////////////////////////////////////////////

extern "C" {

	__declspec(dllexport) int __stdcall ciupcConnect(const char *addr, unsigned short port, ciupDataCb dataCb, ciupErrorCb errorCb) {

		if (!csConnectionsListInitialized) {
			InitializeCriticalSection(&csConnectionsList);
			csConnectionsListInitialized = true;
		}

		EnterCriticalSection(&csConnectionsList);
		int id = getFreeId();
		if (id < 0) return CIUP_ERR_MAX_CONNECTIONS;

		w32_socket *s = w32_tcp_socket_client_create(addr, port);
		if (!s->lasterr.empty()) {
			w32_tcp_socket_close(&s);
			return CIUP_ERR_SOCKET;
		}

		ciupConnectionsList[id].setup(dataCb, errorCb, s, addr, port);
		LeaveCriticalSection(&csConnectionsList);
		ciupConnectionsList[id].hThread = CreateThread(0, 0, ciupConnectionThread, (LPVOID)id, 0, NULL);

		ciupcStart(id);
		return id;
	}

	__declspec(dllexport) void __stdcall ciupcInfo(int id) {

		if (id >= CIUP_MAX_CONNECTIONS) return;
		BYTE cmd = CIUP_MSG_SERVERINFO;
		ciupConnectionsList[id].commands.push(cmd);
	}

	__declspec(dllexport) void __stdcall ciupcStart(int id) {

		if (id >= CIUP_MAX_CONNECTIONS) return;
		BYTE cmd = CIUP_MSG_START;
		ciupConnectionsList[id].commands.push(cmd);
	}

	__declspec(dllexport) void __stdcall ciupcStop(int id) {

		if (id >= CIUP_MAX_CONNECTIONS) return;
		BYTE cmd = CIUP_MSG_STOP;
		ciupConnectionsList[id].commands.push(cmd);
	}

	__declspec(dllexport) void __stdcall ciupcDisconnect(int id) {

		if (id >= CIUP_MAX_CONNECTIONS) return;

		// join thread
		ciupConnectionsList[id].run = false;
		WaitForSingleObject(ciupConnectionsList[id].hThread, INFINITE);
		CloseHandle(ciupConnectionsList[id].hThread);
		ciupConnectionsList[id].hThread = NULL;

		w32_tcp_socket_close(&(ciupConnectionsList[id].sock));
		ciupConnectionsList[id].empty();
	}
}