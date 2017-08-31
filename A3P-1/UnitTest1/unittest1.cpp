#include "stdafx.h"
#include "CppUnitTest.h"
#include "..\w32_tcp_socket_test\win32_tcp_socket.h"

#include <iostream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(w32_socket_init)
		{
			w32_socket sock;
			Assert::IsTrue(sock.sock==  INVALID_SOCKET, L"w32_socket.sock not init as INVALID_SOCKET", LINE_INFO());
			Assert::IsTrue(sock.lasterr == "", L"w32_socket.sock not init as empty string", LINE_INFO());
		}

		/*TEST_METHOD(w32_socket_create)
		{
			unsigned short port = 1024;
			w32_socket *sock;
			while ((sock == NULL) && (port < 2048)) {
				sock = w32_tcp_socket_server_create(port);
				if (!sock->lasterr.empty()) {
					w32_tcp_socket_close(&sock);
					port++;
				}
			}

			if (port < 2048) {

				w32_socket *csock = w32_tcp_socket_client_create("127.0.0.1", port);

				Assert::IsTrue(csock->lasterr.empty(), L"Cannot connect server socket:", LINE_INFO());

				w32_tcp_socket_close(&sock);
				w32_tcp_socket_close(&csock);
			}
		}*/
	};
}