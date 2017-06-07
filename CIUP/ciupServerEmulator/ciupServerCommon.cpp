#include "stdafx.h"
#include "ciupServerCommon.h"
#include <queue>

// TODO: datapoints buffer

// LOGS MANAGEMENT /////////////////////////////////////////////////////////////

// TODO: move to ciupCommon

std::queue<ciupLog> gLogQueue;

// enqueue new log
template< typename ... Args > void ciupQueueLog(streamlog::streamlog_level level, Args const& ... args)
{
	std::ostringstream stream;
	using List = int[];
	(void)List {
		0, ((void)(stream << args), 0) ...
	};

	ciupLog log;

	log.level = level;
	log.descr = stream.str();

	gLogQueue.push(log);
}

int ciupGetLog(ciupLog *log)
{
	if (gLogQueue.empty()) return -1;

	*log = gLogQueue.front();
	gLogQueue.pop();
	return 0;
}

// SENDER THREADS //////////////////////////////////////////////////////////////

// Sender thread data struct
typedef struct ciupSenderData_t {

	char addr[INET_ADDRSTRLEN];
	unsigned short port;
	SOCKET sock;
	bool run;
	HANDLE handle;

	ciupSenderData_t(const char *a, unsigned short p, SOCKET s) {
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
		if (!strcmp(addr, (*it)->addr) && (port == (*it)->port)) {
			return std::distance(ciupSenderList.begin(), it);
		}
	}
	return -1;
}

DWORD WINAPI senderThread(LPVOID lpParam) {

	ciupSenderData *data = (ciupSenderData*)lpParam;

	ciupDataPoint point;
	void *msg;
	int wcount;
	BYTE ack[CIUP_MSG_SIZE(0)];
	int errcount = 0;
	int size;
	USHORT counter = 0;
	DWORD sTime = GetTickCount();

	while (data->run) {

		// TODO: get point from queue
		point.counter = counter;
		point.channel = 0;
		point.Stime = (float)(GetTickCount() - sTime) / 1000;
		point.Acurr = (float)rand() / RAND_MAX;
		point.AHcap = (float)rand() / RAND_MAX;
		point.Ktemp = (float)rand() / RAND_MAX;
		point.Vdiff = (float)rand() / RAND_MAX;

		msg = ciupBuildMessage(CIUP_MSG_DATAPOINT, &point, sizeof(point));
		wcount = w32_udp_socket_write(data->sock, msg, CIUP_MSG_SIZE(sizeof(point)), data->addr, data->port);
		delete[] msg;

		ciupQueueLog(streamlog::debug, "T", data->handle, " Wrote ", wcount, " bytes to ", data->addr, ":", data->port, " counter:", counter, " time:", point.Stime);

		counter = ++counter%USHRT_MAX;

		// read ack as watchdog TODO: read command (stop simulation)
		size = w32_udp_socket_read(data->sock, ack, CIUP_MSG_SIZE(0), NULL, NULL, CIUP_ANS_TIMEOUT_MS);

		if (size < 0) {

			ciupQueueLog(streamlog::error, "T", data->handle, " Socket read ack err:", UDP_SOCK_RET_DESCR(size));
			errcount++;
		}
		else if (size != CIUP_MSG_SIZE(0)) {

			ciupQueueLog(streamlog::error, "T", data->handle, " Wrong ack size, expected:", CIUP_MSG_SIZE(0), " received:", size);
			errcount++;
		}
		else if (ciupCheckMessageSyntax(ack, size) != CIUP_NO_ERR) {

			ciupQueueLog(streamlog::error, "T", data->handle, " Wrong ack syntax");
			errcount++;
		}
		else {
			errcount = 0;
		}

		if (errcount >= CIUP_SERVER_ERROR_LIMIT) {
			ciupQueueLog(streamlog::error, "T", data->handle, " Error limit reached, stopping sender");
			data->run = false;
		}

		Sleep(10); // TODO: define
	}

	return 0;
}

int startSender(SOCKET s, const char* addr, unsigned short port) {

	ciupQueueLog(streamlog::trace, "Starting sender for ", addr, ":", port);

	void *ack = ciupBuildMessage(CIUP_MSG_START);
	w32_udp_socket_write(s, ack, CIUP_MSG_SIZE(0), addr, port);
	delete[] ack;

	// verify if sender exists
	if (findSender(addr, port) < 0) {

		// TODO Error control
		ciupSenderData *d = new ciupSenderData(addr, port, w32_udp_socket_create(0));
		ciupSenderList.push_back(d);
		d->handle = CreateThread(0, 0, senderThread, d, 0, NULL);

		ciupQueueLog(streamlog::trace, "Done");
	}
	else {
		ciupQueueLog(streamlog::error, "Server was yet on");
	}

	return CIUP_NO_ERR;
}

int stopSender(SOCKET s, const char* addr, unsigned short port) {

	ciupQueueLog(streamlog::trace, "Stopping sender for ", addr, ":", port);

	void *ack = ciupBuildMessage(CIUP_MSG_STOP);
	w32_udp_socket_write(s, ack, CIUP_MSG_SIZE(0), addr, port);
	delete[] ack;

	// on stop stop senddata thread on addr:port
	int index = findSender(addr, port);

	if (index >= 0) {

		ciupSenderData *d = ciupSenderList[index];
		ciupSenderList.erase(ciupSenderList.begin() + index);

		d->run = false;

		// join thread
		if (WaitForSingleObject(d->handle, CIUP_STOP_THREAD_TIMEOUT_MS) == WAIT_TIMEOUT) {
			TerminateThread(d->handle, 0);
		}

		closesocket(d->sock);
		delete d;

		ciupQueueLog(streamlog::trace, "Done");
	}
	else {
		ciupQueueLog(streamlog::error, "No server to stop");
	}
	return CIUP_NO_ERR;
}

void stopAllSenders()
{

	// TODO

}

// SERVER INFOS ////////////////////////////////////////////////////////////////

// TODO: set server infos

int sendServerInfo(SOCKET s, const char* addr, unsigned short port) {

	ciupServerInfo d;
	d.status = CIUP_ST_WORKING;
	strncpy_s(d.id, "ciupServerTest", CIUP_MAX_STRING_SIZE);

	void *ans = ciupBuildMessage(CIUP_MSG_SERVERINFO, &d, sizeof(d));
	w32_udp_socket_write(s, ans, CIUP_MSG_SIZE(sizeof(d)), addr, port);
	delete[] ans;

	return CIUP_NO_ERR;
}

// SERVER THREAD ///////////////////////////////////////////////////////////////

HANDLE gServerThread = NULL;
bool gServerRun = false;
SOCKET gServerSock = 0;

DWORD WINAPI serverThread(LPVOID lpParam) {

	BYTE cmd[CIUP_MAX_MSG_SIZE];
	char inAddr[INET_ADDRSTRLEN];
	unsigned short inPort;

	while (gServerRun) {

		// read socket 
		int size = w32_udp_socket_read(gServerSock, cmd, CIUP_MAX_MSG_SIZE, inAddr, &inPort, 1000);

		if (size < 0) {

			if (size != UDP_SOCK_RET_TIMEOUT) {
				ciupQueueLog(streamlog::error, "Socket err:", UDP_SOCK_RET_DESCR(size));
			}
		}
		else if (size != CIUP_MSG_SIZE(0)) {

			ciupQueueLog(streamlog::error, "Wrong size for incoming cmd, expected:", CIUP_MSG_SIZE(0), " received:", size);
		}
		else if (ciupCheckMessageSyntax(cmd, size) != CIUP_NO_ERR) {

			ciupQueueLog(streamlog::error, "Wrong syntax for incoming cmd");
		}
		else {

			// parse command

			switch (*(cmd + CIUP_TYPE_POS)) {

			case CIUP_MSG_SERVERINFO:
				ciupQueueLog(streamlog::debug, "Received SERVERINFO command from ", inAddr, ":", inPort);
				sendServerInfo(gServerSock, inAddr, inPort);
				break;

			case CIUP_MSG_START:
				ciupQueueLog(streamlog::debug, "Received START command from ", inAddr, ":", inPort);
				startSender(gServerSock, inAddr, inPort);
				break;

			case CIUP_MSG_STOP:
				ciupQueueLog(streamlog::debug, "Received STOP command from ", inAddr, ":", inPort);
				stopSender(gServerSock, inAddr, inPort);
				break;

			default:
				ciupQueueLog(streamlog::error, "Unknown message type:", *(cmd + CIUP_TYPE_POS));
				break;
			}
		}
	}

	return 0;
}


int ciupServerStart(unsigned short port) {

	w32_wsa_startup();

	// open socket
	gServerSock = w32_udp_socket_create(port);
	if (gServerSock <= 0) {
		ciupQueueLog(streamlog::error, "Cannot open UDP port ", port);
		return -1;
	}

	// start server thread
	gServerRun = true;
	gServerThread = CreateThread(0, 0, serverThread, 0, 0, NULL);

	// TODO: control thread run error

	ciupQueueLog(streamlog::trace, "ciup server listening un UDP port ", port);
	return 0;
}

int ciupServerStop()
{
	stopAllSenders();

	// stop server thread
	gServerRun = false;
	if (WaitForSingleObject(gServerThread, CIUP_STOP_THREAD_TIMEOUT_MS) == WAIT_TIMEOUT) {
		TerminateThread(gServerThread, 0);
	}
	gServerThread = NULL;

	closesocket(gServerSock);
	gServerSock = 0;

	w32_wsa_cleanup();
	return 0;
}
