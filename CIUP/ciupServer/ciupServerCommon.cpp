#include "stdafx.h"
#include "ciupServerCommon.h"
#include <queue>

// datapoints buffer ///////////////////////////////////////////////////////////

static CRITICAL_SECTION csServerData;

struct {
	ciupDataPoint p[CIUP_POINT_MAX_STORE];
	unsigned int i = 0;
}gPointBuffer;

void ciupServerEnqueueDatapoint(ciupDataPoint &p)
{
	EnterCriticalSection(&csServerData);
	gPointBuffer.p[gPointBuffer.i] = p;
	gPointBuffer.i = ++(gPointBuffer.i) % CIUP_POINT_MAX_STORE;
	LeaveCriticalSection(&csServerData);
}

int ciupServerDatapointIndex()
{
	return gPointBuffer.i;
}

// LOGS MANAGEMENT /////////////////////////////////////////////////////////////

static std::queue<ciupLog> gServerLogQueue;
static CRITICAL_SECTION csServerLog;

// TODO: set loglevel to avoid queueing filtered logs

// enqueue new log
template< typename ... Args > void ciupServerQueueLog(streamlog::level level, Args const& ... args)
{
	std::ostringstream stream;
	using List = int[];
	(void)List {
		0, ((void)(stream << args), 0) ...
	};

	ciupLog log;

	log.level = level;
	log.descr = stream.str();

	EnterCriticalSection(&csServerLog);
	gServerLogQueue.push(log);
	if (gServerLogQueue.size() > CIUP_LOG_MAX_STORE) gServerLogQueue.pop();
	LeaveCriticalSection(&csServerLog);
}

int ciupServerGetLog(ciupLog *log)
{
	if (gServerLogQueue.empty()) return -1;

	EnterCriticalSection(&csServerLog);
	*log = gServerLogQueue.front();
	gServerLogQueue.pop();
	LeaveCriticalSection(&csServerLog);

	return 0;
}

// CONNECTION THREAD ///////////////////////////////////////////////////////////////

// Sender thread data struct
typedef struct ciupConnectionData_t {

	w32_socket *sock;
	bool run;
	HANDLE hThread;
	int qIndex;

	ciupConnectionData_t(w32_socket *s) {
		sock = s;
		run = true;
		qIndex = 0;
	}

}ciupConnectionData;

// list of running threads
static std::vector<ciupConnectionData*> ciupConnectionList;

size_t ciupServerConnectionCount()
{
	return ciupConnectionList.size();
}

int ciupServerQueueIndex(size_t connection)
{
	if (connection >= ciupConnectionList.size()) return -1;
	return ciupConnectionList[connection]->qIndex;
}

int sendServerInfo(w32_socket *s) {

	// TODO: set server info function

	int ret = CIUP_ERR_SOCKET;

	ciupServerInfo d;
	d.status = CIUP_ST_WORKING;
	strncpy_s(d.id, "ciupServerTest", CIUP_MAX_STRING_SIZE);

	void *ans = ciupBuildMessage(CIUP_MSG_SERVERINFO, &d, sizeof(d));
	if (w32_tcp_socket_write(s, ans, CIUP_MSG_SIZE(sizeof(d)), 0) == CIUP_MSG_SIZE(sizeof(d))) ret = CIUP_NO_ERR;
	delete[] ans;

	return ret;
}

DWORD WINAPI connectionThread(LPVOID lpParam) {

	ciupConnectionData *data = (ciupConnectionData*)lpParam;

	ciupDataPoint point;

	void *msg;
	int wcount;
	int wErr = 0;
	int rErr = 0;
	int ret;

	BYTE cmd[CIUP_MAX_MSG_SIZE];
	bool doSend = true;

	while (data->run) {

		// read socket commands
		ret = w32_tcp_socket_read(data->sock, cmd, CIUP_MAX_MSG_SIZE, 1);

		if (ret == 0) {
			// TIMEOUT: nothing to do
		}
		else if (ret < 0) {

			ciupServerQueueLog(streamlog::error, "T", data->hThread, " w32_tcp_socket_read:", data->sock->lasterr.c_str());
			rErr++;
		}
		else if (ret != CIUP_MSG_SIZE(0)) {

			ciupServerQueueLog(streamlog::error, "T", data->hThread, " Wrong size for incoming cmd, expected:", CIUP_MSG_SIZE(0), " received:", ret);
			rErr++;
		}
		else if (ciupCheckMessageSyntax(cmd, ret) != CIUP_NO_ERR) {

			ciupServerQueueLog(streamlog::error, "T", data->hThread, " Wrong syntax for incoming cmd");
			rErr++;
		}
		else {

			// parse command

			switch (*(cmd + CIUP_TYPE_POS)) {

				case CIUP_MSG_SERVERINFO:
					ciupServerQueueLog(streamlog::debug, "T", data->hThread, " Received SERVERINFO command");
					if (sendServerInfo(data->sock) != CIUP_NO_ERR) {
						ciupServerQueueLog(streamlog::error, "T", data->hThread, " Error sending SERVERINFO");
						wErr++;
					}
					else {
						wErr = 0;
					}
					break;

				case CIUP_MSG_START:
					ciupServerQueueLog(streamlog::debug, "T", data->hThread, " Received START command");
					doSend = true;
					break;

				case CIUP_MSG_STOP:
					ciupServerQueueLog(streamlog::debug, "T", data->hThread, " Received STOP command");
					doSend = false;
					break;

				case CIUP_MSG_DISCONNECT:
					ciupServerQueueLog(streamlog::debug, "T", data->hThread, " Received DISCONNECT command");
					doSend = false;
					data->run = false;
					break;

				default:
					ciupServerQueueLog(streamlog::error, "T", data->hThread, " Unknown message type:", *(cmd + CIUP_TYPE_POS));
					break;
			}
			rErr = 0;
		}

		if (rErr >= CIUP_SOCKET_ERROR_LIMIT) {
			ciupServerQueueLog(streamlog::error, "T", data->hThread, " Read error limit reached, closing connection");
			doSend = false;
			data->run = false;
		}

		// send data
		if (doSend) {

			while ((data->qIndex != gPointBuffer.i)&&doSend) {

				msg = ciupBuildMessage(CIUP_MSG_DATAPOINT, &(gPointBuffer.p[data->qIndex]), sizeof(ciupDataPoint));
				wcount = w32_tcp_socket_write(data->sock, msg, CIUP_MSG_SIZE(sizeof(ciupDataPoint)), 100);
				delete[] msg;

				if (wcount == 0) {

					// TIMEOUT
					ciupServerQueueLog(streamlog::error, "T", data->hThread, " w32_tcp_socket_write timeout");
					wErr++;
				}
				else if (wcount < 0) {

					// WRITE ERROR
					ciupServerQueueLog(streamlog::error, "T", data->hThread, " w32_tcp_socket_write:", data->sock->lasterr.c_str());
					wErr++; 
				}
				else {
					// send success
					data->qIndex = ++(data->qIndex) % CIUP_POINT_MAX_STORE;
					wErr = 0;
				}

				if (wErr >= CIUP_SOCKET_ERROR_LIMIT) {
					ciupServerQueueLog(streamlog::error, "T", data->hThread, " Write error limit reached, closing connection");
					doSend = false;
					data->run = false;
				}
				Sleep(0);
			}
		}
		else {
			Sleep(100);
		}
		Sleep(0);
	}
	return 0;
}

void closeConnection(int i)
{
	ciupConnectionList[i]->run = false;
	WaitForSingleObject(ciupConnectionList[i]->hThread, INFINITE);
	CloseHandle(ciupConnectionList[i]->hThread);
	ciupConnectionList[i]->hThread = NULL;
	w32_tcp_socket_close(&(ciupConnectionList[i]->sock));
	delete[] ciupConnectionList[i];
	ciupConnectionList.erase(ciupConnectionList.begin() + i);
}

void closeInactiveConnections()
{
	for (unsigned int i = 0; i < ciupConnectionList.size(); i++) {
		if (!ciupConnectionList[i]->run) {
			closeConnection(i);
		}
	}
}

void closeAllConnections()
{
	while (!ciupConnectionList.empty()) closeConnection(0);
}

// SERVER THREAD ///////////////////////////////////////////////////////////////

static HANDLE gServerThread = NULL;
static bool gServerRun = false;
static w32_socket *gServerSock = NULL;

// listen to new connection on server port
// run connectionThread on each new connection
DWORD WINAPI serverThread(LPVOID lpParam) {

	w32_socket *s = NULL;

	while (gServerRun) {

		// listen for connections
		s = w32_tcp_socket_server_wait(gServerSock);

		if (s != NULL) {

			// start new sender thread on connection
			ciupConnectionData *d = new ciupConnectionData(s);
			ciupConnectionList.push_back(d);
			d->hThread = CreateThread(0, 0, connectionThread, d, 0, NULL);
			if (d->hThread == NULL) {
				ciupServerQueueLog(streamlog::error, "CreateThread: ", GetLastError());
				ciupConnectionList.pop_back();
				delete[] d;
			}
			/*if (SetThreadPriority(d->hThread, THREAD_PRIORITY_HIGHEST) == 0) {
				ciupServerQueueLog(streamlog::error, "SetThreadPriority: ", GetLastError());
			}*/

			ciupServerQueueLog(streamlog::trace, "Started new connection T", d->hThread);
		}
		else {
			ciupServerQueueLog(streamlog::error, "w32_tcp_socket_server_wait: ", gServerSock->lasterr.c_str());
		}
		closeInactiveConnections();
		Sleep(100);
	}

	return 0;
}

int ciupServerStart(unsigned short port) {

	// if is ON return
	if (gServerThread != NULL) return 0;

	InitializeCriticalSection(&csServerLog);
	InitializeCriticalSection(&csServerData);

	// open socket
	gServerSock = w32_tcp_socket_server_create(port);
	if (gServerSock->sock == INVALID_SOCKET || !gServerSock->lasterr.empty()) {
		ciupServerQueueLog(streamlog::error, "w32_tcp_socket_server_create: ", gServerSock->lasterr.c_str());
		w32_tcp_socket_close(&gServerSock);
		return -1;
	}

	// start server thread
	gServerRun = true;
	gServerThread = CreateThread(0, 0, serverThread, 0, 0, NULL);
	if (gServerThread == NULL) {
		ciupServerQueueLog(streamlog::error, "CreateThread: ", GetLastError());
		w32_tcp_socket_close(&gServerSock);
		return -2;
	}

	ciupServerQueueLog(streamlog::trace, "ciup server listening un UDP port ", port);
	return 0;
}

int ciupServerStop()
{
	// if is OFF return
	if (gServerThread == NULL) return 0;

	closeAllConnections();

	// stop server thread
	gServerRun = false;
	w32_tcp_socket_close(&gServerSock);
	WaitForSingleObject(gServerThread, INFINITE);
	CloseHandle(gServerThread);
	gServerThread = NULL;

	DeleteCriticalSection(&csServerLog);
	DeleteCriticalSection(&csServerData);

	return 0;
}

