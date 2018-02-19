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

/*!
*  @brief     TCP socket helper functions for win32
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


