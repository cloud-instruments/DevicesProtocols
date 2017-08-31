#include "stdafx.h"
#include "ciupServerMaccor.h"

#include <queue>

static bool gRun = false;

// LOGS MANAGEMENT /////////////////////////////////////////////////////////////

static std::queue<ciupLog> gMaccorLogQueue;
static bool csMaccorLogInitialized = false;
static CRITICAL_SECTION csMaccorLog;
static streamlog::level logFilter = streamlog::debug;

void maccorSetLogFilter(streamlog::level filter)
{
	logFilter = filter;
}

template< typename ... Args > void maccorQueueLog(streamlog::level level, Args const& ... args)
{
	if (level < logFilter) return;

	std::ostringstream stream;
	using List = int[];
	(void)List {
		0, ((void)(stream << args), 0) ...
	};

	ciupLog log;

	log.level = level;
	log.descr = stream.str();

	if (csMaccorLogInitialized) EnterCriticalSection(&csMaccorLog);
	gMaccorLogQueue.push(log);
	if (gMaccorLogQueue.size() > CIUP_LOG_MAX_STORE) gMaccorLogQueue.pop();
	if (csMaccorLogInitialized) LeaveCriticalSection(&csMaccorLog);
}

int maccorGetLog(ciupLog *log)
{
	if (gMaccorLogQueue.empty()) return -1;

	if (csMaccorLogInitialized) EnterCriticalSection(&csMaccorLog);
	*log = gMaccorLogQueue.front();
	gMaccorLogQueue.pop();
	if (csMaccorLogInitialized) LeaveCriticalSection(&csMaccorLog);

	return 0;
}

// THREADS /////////////////////////////////////////////////////////////////////

DWORD WINAPI serverMaccorPipeThread(LPVOID lpParam) {

	HANDLE hPipe = (HANDLE)lpParam;

	DWORD ret;
	DWORD readCount;
	ciupDataPoint point;
	bool run = true;

	while (run) {

		// read client message
		ret = w32_message_pipe_read(hPipe, &point, sizeof(ciupDataPoint), &readCount);

		if (ret == ERROR_BROKEN_PIPE) {
			maccorQueueLog(streamlog::error, "P", hPipe, " Client disconnected, GLE=", GetLastError());
			run = false;
		}
		else if (ret != 0) {
			maccorQueueLog(streamlog::error, "P", hPipe, " WriteFile failed, GLE=", ret);
			run = false;
		}
		else if (readCount != sizeof(ciupDataPoint)) {
			maccorQueueLog(streamlog::error, "P", hPipe, " Uncomplete read");
			run = false;
		}
		else {
			ciupServerEnqueueDatapoint(point);
		}
	}
	w32_message_pipe_close(&hPipe);
	return 0;
}

DWORD WINAPI serverMaccorWaitThread(LPVOID lpParam) 
{
	HANDLE hPipe;

	while (gRun) {

		// create server pipe
		hPipe = w32_message_pipe_create(CIUP_MACCOR_PIPENAME);
		if (hPipe == INVALID_HANDLE_VALUE) {

			maccorQueueLog(streamlog::error, "w32_message_pipe_create failed, GLE=", GetLastError());
			gRun = false;
		}

		maccorQueueLog(streamlog::trace, "pipe Server awaiting client connection");

		// Wait for the client to connect
		BOOL fConnected = w32_message_pipe_wait(hPipe);

		if (fConnected) {

			maccorQueueLog(streamlog::trace, "Client connected: creating a processing thread");

			// Create a thread for this client. 
			HANDLE hThread = CreateThread(NULL, 0, serverMaccorPipeThread, (LPVOID)hPipe, 0, 0);

			if (hThread == NULL) {

				maccorQueueLog(streamlog::error, "CreateThread failed, GLE=", GetLastError());
				w32_message_pipe_close(&hPipe);
			}
			else {
				CloseHandle(hThread);
			}
		}
		else {
			w32_message_pipe_close(&hPipe);
		}
	}

	return 0;
}

int serverMaccorStart()
{
	if (gRun) return 0;
	gRun = true;

	if (!csMaccorLogInitialized) {
		InitializeCriticalSection(&csMaccorLog);
		csMaccorLogInitialized = true;
	}

	HANDLE hThread = CreateThread(0, 0, serverMaccorWaitThread, NULL, 0, NULL);
	if (hThread == NULL) {
		maccorQueueLog(streamlog::error, "CreateThread failed, GLE=", GetLastError());
	}
	else {
		CloseHandle(hThread);
	}

	return 0;
}

int serverMaccorStop()
{
	if (!gRun) return 0;
	gRun = false;

	if (csMaccorLogInitialized) {
		csMaccorLogInitialized = false;
		DeleteCriticalSection(&csMaccorLog);
	}
	return 0;
}