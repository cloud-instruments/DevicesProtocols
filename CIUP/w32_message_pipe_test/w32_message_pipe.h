/*!
*  @brief     TCP socket helper functions for win32
*  @author    Matteo Lucarelli
*  @date      2017
*/

#pragma once

#include <Windows.h>

// create the pipe (server side) 
// return INVALID_HANDLE_VALUE on failure (GetLastError for details)
HANDLE w32_message_pipe_create(const char *name);

// wait for client connection
// return true when client is connected
BOOL w32_message_pipe_wait(HANDLE hPipe);

// connect the pipe (client side)
// return NULL on failure
HANDLE w32_named_pipe_connect(const char *name);

// read at least buffSize bytes in buff
// return GetLastError on error (ERROR_BROKEN_PIPE if peer has disconnected)
DWORD w32_message_pipe_read(HANDLE hPipe, LPVOID buff, DWORD buffSize, LPDWORD bytesRead);

// write bytesToWrite bytes from buff
// return GetLastError on error (ERROR_BROKEN_PIPE if peer has disconnected)
DWORD w32_message_pipe_write(HANDLE hPipe, LPVOID buff, DWORD bytesToWrite, LPDWORD bytesWrote);

// cloese the pipe after use
void w32_message_pipe_close(HANDLE *hPipe);


