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