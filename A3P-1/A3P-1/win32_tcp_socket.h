/*!
*  @brief     TCP socket helper functions for win32
*  @author    Matteo Lucarelli
*  @date      2017
*/

// TODO: for DLL: call WSA startuo only once not from DLLmain function

#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#include <string>

/// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

/// Seconds to microseconds
#define W32_TCP_SOCKET_S2U(i) ((i)*1000000)

/// default timeout for operation
/// 0 means non blocking
/// @warning leave 0 unless you know what you're doing
#define W32_TCP_SOCKET_TIMEOUT_DEFAULT (0)

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
int w32_tcp_socket_read(w32_socket *s, char* buff, int buff_size, float timeout_sec = W32_TCP_SOCKET_TIMEOUT_DEFAULT);

/// @brief   write data on socket
/// @return  the amount of bytes wrote
int w32_tcp_socket_write(w32_socket *s, const char* buff, int buff_size, float timeout_sec = W32_TCP_SOCKET_TIMEOUT_DEFAULT);

// close the socket
// return 0 or SOCKET_ERROR 
// specific error code can be retrieved by calling WSAGetLastError.
int w32_tcp_socket_close(w32_socket **s);

