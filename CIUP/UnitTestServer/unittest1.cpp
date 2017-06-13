#include "stdafx.h"
#include "CppUnitTest.h"

#include "..\ciupServerEmulator\ciupServerCommon.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestServer
{		
	TEST_CLASS(UnitTest1)
	{
	public:

		TEST_METHOD(ServerLogs)
		{
			ciupServerInit();

			// logBuffer starts empty
			ciupLog log;
			Assert::AreEqual(ciupGetLog(&log), -1, L"logBuffer starts empty", LINE_INFO());
		}

		TEST_METHOD(ServerPointBuffer)
		{
			ciupServerInit();

			// gPointBuffer starts empty
			Assert::AreEqual(ciupDatapointIndex(), 0, L"gPointBuffer starts empty", LINE_INFO());

			// gpointBuffer write
			ciupDataPoint p;
			ciupEnqueueDatapoint(p);
			ciupEnqueueDatapoint(p);
			Assert::AreEqual(ciupDatapointIndex(), 2, L"gpointBuffer write", LINE_INFO());

			// gPointBuffer is circular
			for (int i = 0; i<CIUP_POINT_MAX_STORE; i++) ciupEnqueueDatapoint(p);
			Assert::AreEqual(ciupDatapointIndex(), 2, L"gPointBuffer is circular", LINE_INFO());
		}

		TEST_METHOD(ServerServer)
		{
			// WARN: a free port must be sected here
			unsigned short port = 10001;

			ciupServerInit();

			// server start
			Assert::AreEqual(ciupServerStart(port), 0, L"server start", LINE_INFO());

			// server restart do nothing
			Assert::AreEqual(ciupServerStart(port), 0, L"server restart do nothing", LINE_INFO());

			// port is busy
			w32_socket *s = w32_tcp_socket_server_create(port);
			Assert::AreNotEqual(s->lasterr.c_str(),"", L"port is busy", LINE_INFO());
			w32_tcp_socket_close(&s);

			// client can connect
			w32_socket *sc = w32_tcp_socket_client_create("127.0.0.1", port);
			Assert::AreEqual(sc->lasterr.c_str(), "", L"client can connect", LINE_INFO());
			w32_tcp_socket_close(&sc);
			
			ciupServerStop();

			// port is now free
			s = w32_tcp_socket_server_create(port);
			Assert::AreEqual(s->lasterr.c_str(), "", L"port is now free", LINE_INFO());
			w32_tcp_socket_close(&s);
		}

	};
}