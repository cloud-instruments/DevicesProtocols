// Win32 TCP socket helper functions
// (c)2017 Matteo Lucarelli

#include "stdafx.h"
#include "win32_tcp_socket.h"

#include <sstream>
#include <iostream>

w32_socket *w32_tcp_socket_server_create(unsigned short port)
{
	w32_socket *s = new w32_socket;

	// init WSA
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		s->lasterr = "WSAStartup failed: " + res;
		return s;
	}

	// First call to socket() function
	s->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (s->sock != INVALID_SOCKET) {

		// Initialize socket structure
		struct sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(port);

		// Now bind the host address using bind() call
		res = bind(s->sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
		if (res != 0) {

			s->lasterr = "bind with port " + port;
			s->lasterr += " failed: " + res;
			return s;
		}
	}

	return s;
}

w32_socket *w32_tcp_socket_client_create(const char* addr, unsigned short port) {

	w32_socket *s = new w32_socket;

	// init WSA
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		s->lasterr = "WSAStartup failed: " + res;
		return s;
	}

	// Create the client socket
	s->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s->sock == INVALID_SOCKET) {
		s->lasterr = "socket failed: " + WSAGetLastError();
		return s;
	}


	/*int flag = 1;
	setsockopt(s->sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
	int sendbuff = 0;
	setsockopt(s->sock, SOL_SOCKET, SO_SNDBUF, (char*)&sendbuff, sizeof(int));*/

	// Connect to server
	struct sockaddr_in server;
	//server.sin_addr.s_addr = inet_addr(addr);  // XP only, now deprecated
	inet_pton(AF_INET, addr, &(server.sin_addr));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// if WSAGetLastError == WSAEWOULDBLOCK should retry)
	if (connect(s->sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		std::stringstream ss;
		ss << "connect to " << addr << ":" << port << " failed: " << WSAGetLastError();
		s->lasterr = ss.str();
		//s->lasterr = "connect to " + addr + ":" + port + " failed: " + WSAGetLastError();
		// TODO WSAGetLastError==WSAECONNREFUSED 10061
		return s;
	}

	return s;
}

int w32_tcp_socket_keepalive(w32_socket *s, float timeout_s, float interval_s)
{
	s->lasterr.clear();

	DWORD v = 1;
	if (timeout_s == 0 && interval_s == 0) v = 0;

	if (setsockopt(s->sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&v, sizeof(v)) != 0) {
		s->lasterr = "setsockopt failed: " + WSAGetLastError();
		return -1;
	}

	if (v != 0) {

		struct tcp_keepalive k;
		k.onoff = 1;
		k.keepalivetime = (u_long)(timeout_s * 1000);
		k.keepaliveinterval = (u_long)(interval_s * 1000);

		DWORD outsize = 0;
		if  (WSAIoctl(s->sock, SIO_KEEPALIVE_VALS, &k, sizeof(k), NULL, 0, &outsize, NULL, NULL) != 0 ) {
			s->lasterr = "WSAIoctl failed: " + WSAGetLastError();
			return -2;
		}
	}
	return 0;
}

// TODO: return connection address
w32_socket *w32_tcp_socket_server_wait(w32_socket *s, int max_queue){

	s->lasterr.clear();

	/*int flag = 1;
	setsockopt(s->sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
	int sendbuff = 0;
	setsockopt(s->sock, SOL_SOCKET, SO_SNDBUF, (char*)&sendbuff, sizeof(int));*/


	// mark socket as passive
	//if (max_queue != SOMAXCONN) max_queue = SOMAXCONN_HINT(max_queue);
	if (max_queue > SOMAXCONN) max_queue = SOMAXCONN;
	if (listen(s->sock, max_queue) != 0) return NULL;

	// actual connection from the client
	sockaddr_in client;
	w32_socket *ns = new w32_socket;

	ns->sock = accept(s->sock, (struct sockaddr *)&client, NULL);

	if (ns->sock == INVALID_SOCKET) {
		s->lasterr = "accept failed: " + WSAGetLastError();
		delete ns;
		return NULL;
	}

	// init WSA
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		s->lasterr = "WSAStartup failed: " + res;
		delete ns;
		return NULL;
	}

	// 10093: WSANOTINITIALISED

	return ns;
}

int w32_tcp_socket_read(w32_socket *s, void* buff, int buff_size, int timeout_ms) {

	s->lasterr.clear();

	// test read select
	if (timeout_ms >= 0) {

		fd_set Read;
		FD_ZERO(&Read);
		FD_SET(s->sock, &Read);
		TIMEVAL Timeout;
		Timeout.tv_sec = (int)(timeout_ms/1000);
		Timeout.tv_usec = (int)((timeout_ms%1000)*1000);
		if (select(0, &Read, NULL, NULL, &Timeout) <= 0) {
			return 0;
		}
	}

	int ret = recv(s->sock, (char*)buff, buff_size, 0);
	if (ret < 0) {

		if (WSAGetLastError() == 10054) {
			s->lasterr = "peer disconnected";
			return -1;
		}

		s->lasterr = "recv failed: " + WSAGetLastError();
		return -2;
	}
	return ret;
}

int w32_tcp_socket_write(w32_socket *s, const void* buff, int buff_size, int timeout_ms) {

	s->lasterr.clear();

	// test write select
	if (timeout_ms >= 0){

		fd_set Write;
		FD_ZERO(&Write);
		FD_SET(s->sock, &Write);
		TIMEVAL Timeout;
		Timeout.tv_sec = (int)(timeout_ms / 1000);
		Timeout.tv_usec = (int)((timeout_ms % 1000) * 1000);		
		if (select(0, NULL, &Write, NULL, &Timeout) <= 0){
			return 0;
		}
	}

	int ret = send(s->sock, (const char*)buff, buff_size, 0);
	if (ret < 0) {

		if (WSAGetLastError() == 10054) {
			s->lasterr = "peer disconnected";
			return -1;
		}

		s->lasterr = "send failed: " + WSAGetLastError();
		return -2;
	}
	else if (ret != buff_size) {

		s->lasterr = "uncomplete write";
		return -3;
	}
	
	/*if (ret == SOCKET_ERROR) {
		s->lasterr = "send failed: " + WSAGetLastError();
	}*/

	return ret;
}

int w32_tcp_socket_close(w32_socket **s) {

	if (!(*s)) return 0;

	(*s)->lasterr.clear();

	if ((*s)->sock != INVALID_SOCKET) {

		// to gracefully close
		if (shutdown((*s)->sock, SD_BOTH) == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err != WSAENOTCONN) {
				(*s)->lasterr = "shutdown failed: " + err;
				return -1;
			}
		}

		// TODO intercept error
		closesocket((*s)->sock);
	}

	WSACleanup();

	delete (*s);
	*s = NULL;

	return 0;
}

