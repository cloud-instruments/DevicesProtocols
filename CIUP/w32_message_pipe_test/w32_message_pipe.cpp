#include "stdafx.h"
#include "w32_message_pipe.h"
#include <string>

HANDLE w32_message_pipe_create(const char *name)
{
	// build name
	std::string s = "\\\\.\\pipe\\";
	s.append(name);
	std::wstring sw = std::wstring(s.begin(), s.end());

	HANDLE hPipe = CreateNamedPipe(
		sw.c_str(),               // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		0,                        // output buffer size 
		0,                        // input buffer size 
		0,                        // client time-out 
		NULL);                    // default security attribute 

	return hPipe;
}

BOOL w32_message_pipe_wait(HANDLE hPipe)
{
	return ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
}

HANDLE w32_named_pipe_connect(const char *name)
{
	// build name
	std::string s = "\\\\.\\pipe\\";
	s.append(name);
	std::wstring sw = std::wstring(s.begin(), s.end());

	// Open the named pipe
	// Most of these parameters aren't very relevant for pipes.
	HANDLE hPipe = CreateFile(
		sw.c_str(),
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED, //FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE) return NULL;

	// set the pipe to message mode
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	BOOL fSuccess = SetNamedPipeHandleState(
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time

	if (!fSuccess){
		w32_message_pipe_close(&hPipe);
		return NULL;
	}

	return hPipe;
}

DWORD w32_message_pipe_read(HANDLE hPipe, LPVOID buff, DWORD buffSize, LPDWORD bytesRead)
{
	// Read client requests from the pipe. This simplistic code only allows messages
	// up to BUFSIZE characters in length.
	BOOL fSuccess = ReadFile(
		hPipe,       // handle to pipe 
		buff,        // buffer to receive data 
		buffSize,    // size of buffer 
		bytesRead,   // number of bytes read 
		NULL);       // not overlapped I/O 

	if (!fSuccess) return GetLastError();
	return 0;
}

DWORD w32_message_pipe_write(HANDLE hPipe, LPVOID buff, DWORD bytesToWrite, LPDWORD bytesWrote)
{
	// Write the reply to the pipe (same as message) 
	BOOL fSuccess = WriteFile(
		hPipe,         // handle to pipe 
		buff,          // buffer to write from 
		bytesToWrite,  // number of bytes to write 
		bytesWrote,   // number of bytes written 
		NULL);         // not overlapped I/O 

	if (!fSuccess) return GetLastError();
	return 0;
}



void w32_message_pipe_close(HANDLE *hPipe)
{
	if (!*hPipe) return;
	FlushFileBuffers(*hPipe);
	DisconnectNamedPipe(*hPipe);
	CloseHandle(*hPipe);
	*hPipe = NULL;
}


