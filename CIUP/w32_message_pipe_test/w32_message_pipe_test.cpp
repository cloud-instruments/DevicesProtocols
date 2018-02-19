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

// w32_message_pipe_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <time.h>

#include "w32_message_pipe.h"

using namespace std;

#define BUFSIZE 256

CRITICAL_SECTION outCriticalSection;

// SERVER //////////////////////////////////////////////////////////////////////

DWORD tCount = 0;

DWORD WINAPI pipeServerThread(LPVOID lpvParam)
{
	// not safe, for test only
	DWORD myCount = tCount;
	tCount++;

	BYTE buff[BUFSIZE];
	DWORD readCount;
	DWORD wroteCount;

	BOOL fSuccess = FALSE;

	// Print verbose messages. In production code, this should be for debugging only.
	cout << "Started T" << myCount << endl;

	// The thread's parameter is a handle to a pipe object instance. 
	HANDLE hPipe = (HANDLE)lpvParam;

	int clientId = -1;

	DWORD msgcount = 0;
	DWORD tPrev = GetTickCount();
	DWORD cPrev = 0;
	DWORD tNow;

	DWORD ret;

	// Loop until done reading
	while (1)
	{
		// read client message
		ret = w32_message_pipe_read(hPipe, buff, BUFSIZE, &readCount);

		if (ret == ERROR_BROKEN_PIPE) {
			cerr << "Client disconnected, GLE=" << GetLastError() << endl;
			break;
		}
		else if (ret != 0) {
			cerr << "WriteFile failed, GLE=" << ret << endl;
			break;
		}
		else if (readCount != BUFSIZE) {
			cerr << "Uncomplete read" << endl;
			break;
		}

		// control message content
		// sequence
		if (*((DWORD*)buff) != msgcount) cerr << "T" << myCount << " Sequence error" << endl;
		// client id
		if (clientId == -1) {
			clientId = *((int*)(buff + sizeof(DWORD)));
			cerr << "T" << myCount << " client ID:" << clientId << endl;
		}
		else {

			if (*((int*)(buff + sizeof(DWORD))) != clientId) cerr << "T" << myCount << " Id error" << endl;
		}

		// Write the reply to the pipe (same as message) 
		ret = w32_message_pipe_write(hPipe, buff, BUFSIZE, &wroteCount);

		if (ret == ERROR_BROKEN_PIPE) {
			cerr << "Client disconnected, GLE=" << GetLastError() << endl;
			break;
		}
		else if (ret != 0) {
			cerr << "WriteFile failed, GLE=" << ret << endl;
			break;
		}
		else if (wroteCount != BUFSIZE) {
			cerr << "Uncomplete write" << endl;
			break;
		}

		// performance control
		tNow = GetTickCount();
		if (tNow - tPrev > 1000) {
			double mS = (msgcount - cPrev) / ((tNow - tPrev) / 1000.0);
			EnterCriticalSection(&outCriticalSection);
			cout << "T" << myCount << " " << (int)mS << " msg/s " << (int)(mS*BUFSIZE) << " B/s" << " (" << tNow - tPrev << ")" << endl;
			LeaveCriticalSection(&outCriticalSection);
			tPrev = GetTickCount();
			cPrev = msgcount;
		}

		msgcount++;
	}

	w32_message_pipe_close(&hPipe);

	cout << "Thread exiting" << endl;
	return 1;
}

void wait_for_message_pipe_client()
{
	// create server pipe
	HANDLE hPipe = w32_message_pipe_create("ciuppipe");
	if (hPipe == INVALID_HANDLE_VALUE) {

		cerr << "w32_message_pipe_create failed, GLE=" << GetLastError() << endl;
		return;
	}

	cout << "Pipe Server awaiting client connection" << endl;

	// Wait for the client to connect
	BOOL fConnected = w32_message_pipe_wait(hPipe);

	if (fConnected) {

		cout << "Client connected: creating a processing thread" << endl;

		// Create a thread for this client. 
		HANDLE hThread = CreateThread(NULL, 0, pipeServerThread, (LPVOID)hPipe, 0, 0); // thread parameter 

		if (hThread == NULL) {

			cerr << "CreateThread failed, GLE=" << GetLastError() << endl;
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

// CLIENT //////////////////////////////////////////////////////////////////////

bool write_read_pipe_client()
{
	// first time connection
	static HANDLE hPipe = NULL;
	if (hPipe == NULL) {

		// connect to the pipe server
		cout << "Connecting to pipe..." << endl;
		hPipe = w32_named_pipe_connect("ciuppipe");
		if (!hPipe) {
			cerr << "Failed to connect to pipe. Server not running?" << endl;
			return false;
		}
	}

	// generate random ID
	static int myId = - 1;
	if (myId == -1) {
		myId = rand() % 1000 + 1;
		srand((unsigned int)time(NULL));
		cout << "My id:" << myId << endl;
	}

	static DWORD msgcount = 0;
	static DWORD tPrev = GetTickCount();
	static DWORD cPrev = 0;

	DWORD tNow;
	DWORD ret;
	BYTE inBuff[BUFSIZE];
	DWORD readCount;
	BYTE outBuff[BUFSIZE];
	DWORD wroteCount;

	// output message
	*((DWORD*)outBuff) = msgcount;
	*((int*)(outBuff + sizeof(DWORD))) = myId;

	ret = w32_message_pipe_write(hPipe, outBuff, BUFSIZE, &wroteCount);

	if (ret == ERROR_BROKEN_PIPE) {
		cerr << "Server disconnected, GLE=" << GetLastError() << endl;
		return false;
	}
	else if (ret != 0) {
		cerr << "WriteFile failed, GLE=" << ret << endl;
		return false;
	}
	else if (wroteCount != BUFSIZE) {
		cerr << "Uncomplete write" << endl;
		return false;
	}

	ret = w32_message_pipe_read(hPipe, inBuff, BUFSIZE, &readCount);

	if (ret == ERROR_BROKEN_PIPE) {
		cerr << "Server disconnected, GLE=" << GetLastError() << endl;
		return false;
	}
	else if (ret != 0) {
		cerr << "WriteFile failed, GLE=" << ret << endl;
		return false;
	}
	else if (readCount != BUFSIZE) {
		cerr << "Uncomplete read" << endl;
		return false;
	}

	// control answer (same as message)
	if (*((DWORD*)inBuff) != msgcount) cerr << "Sequence error" << endl;
	if (*((int*)(outBuff + sizeof(DWORD))) != myId) cerr << "Id error" << endl;
	if (memcmp(inBuff, outBuff, BUFSIZE) != 0) cerr << "Answer error" << endl;

	// ouput performance
	tNow = GetTickCount();
	if (tNow - tPrev > 1000) {

		double mS = (msgcount - cPrev) / ((tNow - tPrev) / 1000.0);
		cout << (int)mS << " msg/s " << (int)(mS*BUFSIZE) << " B/s" << " (" << tNow - tPrev << ")" << endl;
		tPrev = GetTickCount();
		cPrev = msgcount;
	}

	msgcount++;
	return true;
}

void print_usage(const char *exe) {

	cerr << "usage: " << exe << " [-c]" << endl;
	cerr << "-c : connect as client" << endl;
}

int main(int argc, char **argv)
{
	bool clientmode = false;

	for (int i = 1; i < argc; i++) {

		if (!strcmp(argv[i], "-h")) {

			print_usage(argv[0]);
			return 0;
		}

		if (!strcmp(argv[i], "-c")) {

			clientmode = true;
		}
	}
	
	InitializeCriticalSectionAndSpinCount(&outCriticalSection, 0x00000400);
	while (true) {

		if (clientmode) {
			if (!write_read_pipe_client()) break;
		}
		else {
			wait_for_message_pipe_client();
		}
		Sleep(0);
	}
	
	return 0;
}

