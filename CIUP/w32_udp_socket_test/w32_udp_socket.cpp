#include "stdafx.h"
#include "w32_udp_socket.h"

void w32_wsa_startup() {
	WSADATA socketInfo;
	WSAStartup(MAKEWORD(2, 0), &socketInfo);
}

void w32_wsa_cleanup() {
	WSACleanup();
}

SOCKET w32_udp_socket_create(unsigned short port)
{
	SOCKET sock = 0;

	// create socket
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sock>0) {

		struct sockaddr_in bindAddr = {};

		// initialize address struct
		memset(&bindAddr, 0, sizeof(bindAddr));   // clear address struct
		bindAddr.sin_family = PF_INET;            // address type is INET
		bindAddr.sin_addr.s_addr = INADDR_ANY;    // address from (anywhere)
		bindAddr.sin_port = htons(port);          // local port (0 is auto)

		// associate socket with port
		if ((bind(sock, (const struct sockaddr*)&bindAddr, sizeof(bindAddr))) < 0) {
			closesocket(sock);
			sock = 0;
		}
	}

	return sock;
}

int w32_udp_socket_read(SOCKET sock, void* buff, size_t buff_size, char* fromAddr, unsigned short* fromPort, long msTimeout)
{
	int ret = UDP_SOCK_RET_FAIL;

	// test read select
	if (msTimeout > 0) {

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		timeval timeOut;
		timeOut.tv_sec = (int)(msTimeout / 1000);
		timeOut.tv_usec = (msTimeout % 1000) * 1000;
		if (select(0, &readfds, NULL, NULL, &timeOut) <= 0) {
			return UDP_SOCK_RET_TIMEOUT;
		}
	}

	struct sockaddr_in recvAddr = {};
	int aDim = sizeof(recvAddr);
	ret = recvfrom(sock, (char*)buff, buff_size, 0, (struct sockaddr*)&recvAddr, &aDim);

	if ((ret > 0) && (fromAddr != NULL)) {
		inet_ntop(AF_INET, &(recvAddr.sin_addr), fromAddr, INET_ADDRSTRLEN);
		//strncpy_s(fromAddr, INET_ADDRSTRLEN, inet_ntoa(recvAddr.sin_addr), INET_ADDRSTRLEN);  // deprecated after XP
	}
	if ((ret>0) && (fromPort != NULL)) *fromPort = ntohs(recvAddr.sin_port);

	return ret;
}

int w32_udp_socket_write(SOCKET sock, const void* buff, size_t buff_size, const char* addr, unsigned short port)
{
	int ret = UDP_SOCK_RET_SUCCESS;
	struct sockaddr_in saddr = {};

	//saddr.sin_addr.s_addr = inet_addr(addr); // deprecated after XP
	inet_pton(AF_INET, addr, &(saddr.sin_addr));
	saddr.sin_family = PF_INET;
	saddr.sin_port = htons(port);
	ret = sendto(sock, (char*)buff, buff_size, 0, (const sockaddr*)&saddr, sizeof(saddr));

	return ret;
}


