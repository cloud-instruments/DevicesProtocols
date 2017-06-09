#include "stdafx.h"
#include "ciupServerCommon.h"
#include <queue>

// datapoints buffer ///////////////////////////////////////////////////////////

struct {
	ciupDataPoint p[CIUP_POINT_MAX_STORE];
	unsigned int i = 0;
}gPointBuffer;

void ciupEnqueueDatapoint(ciupDataPoint &p)
{
	gPointBuffer.p[gPointBuffer.i] = p;
	gPointBuffer.i = ++(gPointBuffer.i) % CIUP_POINT_MAX_STORE;
}

// LOGS MANAGEMENT /////////////////////////////////////////////////////////////

std::queue<ciupLog> gLogQueue;

// enqueue new log
template< typename ... Args > void ciupQueueLog(streamlog::level level, Args const& ... args)
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
	if (gLogQueue.size() > CIUP_LOG_MAX_STORE) gLogQueue.pop();
}

int ciupGetLog(ciupLog *log)
{
	if (gLogQueue.empty()) return -1;

	*log = gLogQueue.front();
	gLogQueue.pop();
	return 0;
}

// CONNECTION THREAD ///////////////////////////////////////////////////////////////

// Sender thread data struct
typedef struct ciupConnectionData_t {

	w32_socket *sock;
	bool run;
	HANDLE hThread;

	ciupConnectionData_t(w32_socket *s) {
		sock = s;
		run = true;
	}

}ciupConnectionData;

// list of running threads
std::vector<ciupConnectionData*> ciupConnectionList;

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
	int qIndex = 0;

	while (data->run) {

		// read socket commands
		ret = w32_tcp_socket_read(data->sock, cmd, CIUP_MAX_MSG_SIZE, 0);

		if (ret == 0) {
			// TIMEOUT: nothing to do
		}
		else if (ret < 0) {

			ciupQueueLog(streamlog::error, "T", data->hThread, " w32_tcp_socket_read:", data->sock->lasterr.c_str());
			rErr++;
		}
		else if (ret != CIUP_MSG_SIZE(0)) {

			ciupQueueLog(streamlog::error, "T", data->hThread, " Wrong size for incoming cmd, expected:", CIUP_MSG_SIZE(0), " received:", ret);
			rErr++;
		}
		else if (ciupCheckMessageSyntax(cmd, ret) != CIUP_NO_ERR) {

			ciupQueueLog(streamlog::error, "T", data->hThread, " Wrong syntax for incoming cmd");
			rErr++;
		}
		else {

			// parse command

			switch (*(cmd + CIUP_TYPE_POS)) {

				case CIUP_MSG_SERVERINFO:
					ciupQueueLog(streamlog::debug, "T", data->hThread, " Received SERVERINFO command");
					if (sendServerInfo(data->sock) != CIUP_NO_ERR) {
						ciupQueueLog(streamlog::error, "T", data->hThread, " Error sending SERVERINFO");
						wErr++;
					}
					else {
						wErr = 0;
					}
					break;

				case CIUP_MSG_START:
					ciupQueueLog(streamlog::debug, "T", data->hThread, " Received START command");
					doSend = true;
					break;

				case CIUP_MSG_STOP:
					ciupQueueLog(streamlog::debug, "T", data->hThread, " Received STOP command");
					doSend = false;
					break;

				case CIUP_MSG_DISCONNECT:
					ciupQueueLog(streamlog::debug, "T", data->hThread, " Received DISCONNECT command");
					doSend = false;
					data->run = false;
					break;

				default:
					ciupQueueLog(streamlog::error, "T", data->hThread, " Unknown message type:", *(cmd + CIUP_TYPE_POS));
					break;
			}

			rErr = 0;
		}

		if (rErr >= CIUP_SOCKET_ERROR_LIMIT) {
			ciupQueueLog(streamlog::error, "T", data->hThread, " Read error limit reached, closing connection");
			doSend = false;
			data->run = false;
		}

		// send data
		if ((doSend) && (qIndex != gPointBuffer.i) ) {

			msg = ciupBuildMessage(CIUP_MSG_DATAPOINT, &(gPointBuffer.p[qIndex]), sizeof(ciupDataPoint));
			wcount = w32_tcp_socket_write(data->sock, msg, CIUP_MSG_SIZE(sizeof(ciupDataPoint)), 0);
			delete[] msg;
			
			if (wcount == 0) {

				// TIMEOUT
				ciupQueueLog(streamlog::error, "T", data->hThread, " w32_tcp_socket_write timeout");
				wErr++;
			}
			else if (wcount < 0) {

				// WRITE ERROR
				ciupQueueLog(streamlog::error, "T", data->hThread, " w32_tcp_socket_write:", data->sock->lasterr.c_str());
				wErr++;
			}
			else {

				// TODO: control point sequence

				// send success
				ciupQueueLog(streamlog::debug, "T", data->hThread, " wrote point counter:", gPointBuffer.p[qIndex].counter );
				qIndex = ++qIndex % CIUP_POINT_MAX_STORE;
				wErr = 0;
			}

			if (wErr >= CIUP_SOCKET_ERROR_LIMIT) {
				ciupQueueLog(streamlog::error, "T", data->hThread, " Write error limit reached, stopping send");
				doSend = false;
			}
		}
		Sleep(0);
	}

	w32_tcp_socket_close(&(data->sock));
	return 0;
}

void closeAllConnections()
{

	// TODO

}

// SERVER THREAD ///////////////////////////////////////////////////////////////

HANDLE gServerThread = NULL;
bool gServerRun = false;
w32_socket *gServerSock = NULL;

// listen to new connection on server port
// run connectionThread on each new connection
DWORD WINAPI serverThread(LPVOID lpParam) {

	w32_socket *s = NULL;

	while (gServerRun) {

		// listen for connections
		s = w32_tcp_socket_server_wait(gServerSock);

		if (s != NULL) {

			// start new sender thread on connection, TODO: get address
			ciupConnectionData *d = new ciupConnectionData(s);
			ciupConnectionList.push_back(d);
			d->hThread = CreateThread(0, 0, connectionThread, d, 0, NULL); // TODO: error control
			ciupQueueLog(streamlog::trace, "Started new connection T", d->hThread);
		}
		else {
			ciupQueueLog(streamlog::error, "w32_tcp_socket_server_wait: ", gServerSock->lasterr.c_str());
		}

		Sleep(100);
	}

	return 0;
}

int ciupServerStart(unsigned short port) {

	// open socket
	gServerSock = w32_tcp_socket_server_create(port);
	if (gServerSock->sock == INVALID_SOCKET) {
		ciupQueueLog(streamlog::error, gServerSock->lasterr.c_str(), port);
		w32_tcp_socket_close(&gServerSock);
		gServerSock = NULL;
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
	closeAllConnections();

	// stop server thread
	gServerRun = false;
	if (WaitForSingleObject(gServerThread, CIUP_STOP_THREAD_TIMEOUT_MS) == WAIT_TIMEOUT) {
		TerminateThread(gServerThread, 0);
	}
	gServerThread = NULL;

	w32_tcp_socket_close(&gServerSock);
	return 0;
}
