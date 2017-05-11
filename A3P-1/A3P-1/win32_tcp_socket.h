// Win32 TCP socket helper functions
// (c)2017 Matteo Lucarelli

// TODO: for DLL: call WSA startuo only once not from DLLmain function

#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

#define W32_TCP_SOCKET_S2U(i) ((i)*1000000)

// timeout!=0 means non blocking
#define W32_TCP_SOCKET_TIMEOUT_DEFAULT (0)

// create a server socket on port
// return socket handle, INVALID_SOCKET on error
// must close after use: w32_tcp_socket_close
SOCKET w32_tcp_socket_server_create(unsigned short port);

// create a client socket for addr:port
// return socket handle, INVALID_SOCKET on error
// must be close after use with w32_tcp_socket_close
SOCKET w32_tcp_socket_client_create(const char* addr, unsigned short port, float timeout_sec = W32_TCP_SOCKET_TIMEOUT_DEFAULT);

// enable keepalive for socket
// timeout_s: time 0 ith no activity until the first keep-alive packet is sent
// interval_s interval between when successive keep-alive packets are sent if no ack is received
// Use (0,0) to disabe keepalive (default)
int w32_tcp_socket_keepalive(SOCKET s, float timeout_s, float interval_s);

// wait for a connection, and accept if available - WARN: blocking call
// return the new connected socket handle, INVALID_SOCKET on error
// connection socket must be close after use with w32_tcp_socket_close
SOCKET w32_tcp_socket_server_wait(SOCKET s, int max_queue = SOMAXCONN);

// read data from socket
// return the amount of bytes read
int w32_tcp_socket_read(SOCKET s, char* buff, int buff_size, float timeout_sec = W32_TCP_SOCKET_TIMEOUT_DEFAULT);

// write data on socket
// return the amount of bytes written or errors (TCP_SOCKET_ERR_* or -1:broken pipe)
int w32_tcp_socket_write(SOCKET s, const char* buff, int buff_size, float timeout_sec = W32_TCP_SOCKET_TIMEOUT_DEFAULT);

// close the socket
// return 0 or SOCKET_ERROR 
// specific error code can be retrieved by calling WSAGetLastError.
int w32_tcp_socket_close(SOCKET s);

