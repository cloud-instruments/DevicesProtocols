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

// TODO: for DLL: call WSA startup only once not from DLLmain function

#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#include <string>

/// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

#define W32_TCP_SOCKET_BLOCK (-1)

typedef struct w32_socket_t{
	std::string lasterr="";
	SOCKET sock=INVALID_SOCKET;
}w32_socket;

// TODO: add WSA starup / WSA cleanup

/// @brief   create a server socket on port
/// @param   port port to use
/// @return  socket, control lasterr for errors
/// @warning socket must be closed after use with w32_tcp_socket_close
w32_socket *w32_tcp_socket_server_create(unsigned short port);

/// @brief   create a client socket 
/// @param   addr IPv4 address of server to connect
/// @param   port port to connect
/// @return  socket, control lasterr for errors
/// @warning socket must be closed after use with w32_tcp_socket_close
w32_socket *w32_tcp_socket_client_create(const char* addr, unsigned short port);

/// @brief   enable keepalive for socket s
/// @param   timeout_s time 0 with no activity until the first keep-alive packet is sent (0 to disable)
/// @param   interval_s interval between when successive keep-alive packets are sent if no ack is received (0 to disable)
int w32_tcp_socket_keepalive(w32_socket *s, float timeout_s, float interval_s);

/// @brief   wait for a connection on s, and accept if available
/// @return  the new connected socket handle, NULL on error
/// @warning connection socket must be close after use with w32_tcp_socket_close
/// @warning blocking call
w32_socket *w32_tcp_socket_server_wait(w32_socket *s, int max_queue = SOMAXCONN);

/// @brief   read data from socket
/// @return  the amount of bytes read
/// @return  0 for timeout
/// @return  -1 for client diconnected
/// @return  -2 for read error (WSA error in lasterr)
int w32_tcp_socket_read(w32_socket *s, void* buff, int buff_size, int timeout_ms = W32_TCP_SOCKET_BLOCK);

/// @brief   write data on socket
/// @return  the amount of bytes wrote
int w32_tcp_socket_write(w32_socket *s, const void* buff, int buff_size, int timeout_ms = W32_TCP_SOCKET_BLOCK);

// close the socket
// return 0 or SOCKET_ERROR 
// specific error code can be retrieved by calling WSAGetLastError.
int w32_tcp_socket_close(w32_socket **s);

