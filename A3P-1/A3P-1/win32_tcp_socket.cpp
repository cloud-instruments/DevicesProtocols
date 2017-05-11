// Win32 TCP socket helper functions
// (c)2017 Matteo Lucarelli

#include "stdafx.h"
#include "win32_tcp_socket.h"

// TODO: remove output, use queue or last_error
#include <iostream>

SOCKET w32_tcp_socket_server_create(unsigned short port)
{
	// init WSA
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		std::cerr << "WSAStartup failed: " << res << std::endl;
		return INVALID_SOCKET;
	}
	
	SOCKET s = INVALID_SOCKET;

	// First call to socket() function
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s != INVALID_SOCKET) {

		// Initialize socket structure
		struct sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(port);

		// Now bind the host address using bind() call
		res = bind(s, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
		if (res != 0) {

			std::cerr << "bind failed: " << res << std::endl;
			return INVALID_SOCKET;
		}
	}

	return s;
}

SOCKET w32_tcp_socket_client_create(const char* addr, unsigned short port, float timeout_sec) {

	// init WSA
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		std::cerr << "WSAStartup failed: " << res << std::endl;
		return INVALID_SOCKET;
	}

	SOCKET s = INVALID_SOCKET;

	// Create the client socket
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
		return INVALID_SOCKET;
	}

	if (timeout_sec > 0) {

		// set the socket in non-blocking
		unsigned long mode = 1;
		if (ioctlsocket(s, FIONBIO, &mode) != NO_ERROR) {
			std::cerr << "ioctlsocket failed: " << WSAGetLastError() << std::endl;
			return INVALID_SOCKET;
		}
	}

	// Connect to server
	struct sockaddr_in server;
	//server.sin_addr.s_addr = inet_addr(addr);  // XP only, now deprecated
	inet_pton(AF_INET, addr, &(server.sin_addr));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// if WSAGetLastError == WSAEWOULDBLOCK should retry)
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		std::cerr << "connect to " << addr << ":" << port << " failed: " << WSAGetLastError() << std::endl;

		// TODO WSAGetLastError==WSAECONNREFUSED 10061
		return INVALID_SOCKET;
	}

	if (timeout_sec > 0) {

		// reset to blocking mode
		unsigned long mode = 0;
		if (ioctlsocket(s, FIONBIO, &mode) != NO_ERROR) {
			std::cerr << "ioctlsocket failed: " << WSAGetLastError() << std::endl;
			return INVALID_SOCKET;
		}

		// test if socket is ready to write
		fd_set Write, Err;
		FD_ZERO(&Write);
		FD_ZERO(&Err);
		FD_SET(s, &Write);
		FD_SET(s, &Err);
		TIMEVAL Timeout;
		Timeout.tv_sec = (int)(timeout_sec);
		Timeout.tv_usec = (int)(W32_TCP_SOCKET_S2U(timeout_sec - (int)timeout_sec));
		select(0, NULL, &Write, &Err, &Timeout);
		if (FD_ISSET(s, &Write) == 0) {
			return INVALID_SOCKET;
		}
	}

	return s;
}

int w32_tcp_socket_keepalive(SOCKET s, float timeout_s, float interval_s)
{
	DWORD v = 1;
	if (timeout_s == 0 && interval_s == 0) v = 0;

	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&v, sizeof(v)) != 0) {

		std::cerr << "setsockopt failed: " << WSAGetLastError() << std::endl;
		return SOCKET_ERROR;
	}

	if (v != 0) {

		struct tcp_keepalive k;
		k.onoff = 1;
		k.keepalivetime = (u_long)(timeout_s * 1000);
		k.keepaliveinterval = (u_long)(interval_s * 1000);

		DWORD outsize = 0;
		if  (WSAIoctl(s, SIO_KEEPALIVE_VALS, &k, sizeof(k), NULL, 0, &outsize, NULL, NULL) != 0 ) {
			std::cerr << "WSAIoctl failed: " << WSAGetLastError() << std::endl;
			return SOCKET_ERROR;
		}
	}
	return 0;
}


SOCKET w32_tcp_socket_server_wait(SOCKET s, int max_queue)
{
	// init WSA
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		std::cerr << "WSAStartup failed: " << res << std::endl;
		return INVALID_SOCKET;
	}

	SOCKET ns = INVALID_SOCKET;

	// mark socket as passive
	if (max_queue != SOMAXCONN) max_queue = SOMAXCONN_HINT(max_queue);
	if (listen(s, max_queue) != 0) return INVALID_SOCKET;

	// actual connection from the client
	sockaddr_in client;
	ns = accept(s, (struct sockaddr *)&client, NULL);

	if (ns == INVALID_SOCKET) {

		std::cerr << "accept failed: " << WSAGetLastError() << std::endl;

	}else {

		char addr[16];
		inet_ntop(AF_INET, &(client.sin_addr), addr, 16);
		std::cout << "accepted " << addr << ":" << ntohs(client.sin_port) << std::endl;

	}

	// 10093: WSANOTINITIALISED

	return ns;
}

int w32_tcp_socket_read(SOCKET s, char* buff, int buff_size, float timeout_sec) {

	// test read select
	if (timeout_sec>0) {

		fd_set Read, Err;
		FD_ZERO(&Read);
		FD_ZERO(&Err);
		FD_SET(s, &Read);
		FD_SET(s, &Err);
		TIMEVAL Timeout;
		Timeout.tv_sec = (int)(timeout_sec);
		Timeout.tv_usec = (int)(W32_TCP_SOCKET_S2U(timeout_sec - (int)timeout_sec));
		select(0, &Read, NULL, &Err, &Timeout);
		if (FD_ISSET(s, &Read) == 0) {
			return 0;
		}
	}

	int ret = recv(s, buff, buff_size, 0);
	if (ret < 0) {
		std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
		// WSAECONNRESET 10054 connection hard closed by peer
	}
	return ret;
}

int w32_tcp_socket_write(SOCKET s, const char* buff, int buff_size, float timeout_sec) {

	// test write select
	if (timeout_sec > 0){

		fd_set Write, Err;
		FD_ZERO(&Write);
		FD_ZERO(&Err);
		FD_SET(s, &Write);
		FD_SET(s, &Err);
		TIMEVAL Timeout;
		Timeout.tv_sec = (int)(timeout_sec);
		Timeout.tv_usec = (int)(W32_TCP_SOCKET_S2U(timeout_sec - (int)timeout_sec));
		select(0, NULL, &Write, &Err, &Timeout);
		if (FD_ISSET(s, &Write) == 0) {
			return 0;
		}
	}

	int ret = send(s, buff, buff_size, 0);
	if (ret == SOCKET_ERROR) {
		std::cerr << "send failed: " << WSAGetLastError() << std::endl;
	}

	return ret;
}

int w32_tcp_socket_close(SOCKET s) {

	int ret = 0;

	if (s != INVALID_SOCKET) {

		// to gracefully close
		if (shutdown(s, SD_BOTH) == SOCKET_ERROR) {
			std::cerr << "shutdown failed with error: " << WSAGetLastError();
			ret = SOCKET_ERROR;
		}

		// TODO intercept error
		closesocket(s);
	}

	WSACleanup();
	return ret;
}

