/* (C)2017 Matteo Lucarelli
*
*	Header file for UDP socket functions on win32
*	Date: 23/05/2017
*	Author: Matteo Lucarelli
*
*******************************************************************************/

#pragma once

#include <Winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

// return values
#define UDP_SOCK_RET_SUCCESS  0
#define UDP_SOCK_RET_FAIL    -1
#define UDP_SOCK_RET_TIMEOUT -2

// startup must be called before any other operation
void w32_wsa_startup();

// cleanup must be called after any other operation
void w32_wsa_cleanup();

// create a UDP socket
// return socket handle, <=0 on error
// must be closed after use with closesocket(SOCKET);
SOCKET w32_udp_socket_create(
	unsigned short port);        // port to use (0 for auto)

// read data from UDP socket
// return the amount of bytes read
// return readed bytes in buff
int w32_udp_socket_read(
	SOCKET sock                 // socket to use
	, void* buff                // buffer to return data
	, size_t buff_size          // max amount of data to write in buff
	, char* fromAddr            // if !=NULL returns sending host
	, unsigned short* fromPort  // if !=NULL returns sending port
	, long msTimeout);          // timeout in mS (0 = blocking connection)

// write data on socket
// return the amount of bytes wrote or errors
// WARN: buff_size must be greater than any possible incoming datagram
int w32_udp_socket_write(
	SOCKET sock              // socket to use
	, const void* buff       // buffer to write
	, size_t buff_size       // amount of bytes to write
	, const char* addr       // destination host
	, unsigned short port);  // destination port
