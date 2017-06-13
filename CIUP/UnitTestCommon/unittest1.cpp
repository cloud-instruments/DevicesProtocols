#include "stdafx.h"
#include "CppUnitTest.h"

#include "..\ciupClientDll\ciupCommon.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestCommon
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(MessageSyntax)
		{
			void *msg = ciupBuildMessage(CIUP_MSG_SERVERINFO);

			// check syntax
			Assert::AreEqual(ciupCheckMessageSyntax(msg, CIUP_MSG_SIZE(0)), CIUP_NO_ERR, L"check syntax", LINE_INFO());

			// check message type
			Assert::AreEqual(*((BYTE*)msg + CIUP_TYPE_POS), CIUP_MSG_SERVERINFO, L"check message type", LINE_INFO());

			// check payload size
			Assert::AreEqual((int)CIUP_PAYLOAD_SIZE(msg), 0, L"check payload size", LINE_INFO());
			delete[] msg;
		}

		// TODO dll
		/*TEST_METHOD(ciupClient)
		{
		ciupServerInfo d;
		strncpy_s(d.id, "test", CIUP_MAX_STRING_SIZE);
		d.status = CIUP_ST_UNKNOWN;

		std::string ret;
		ciupJsonSerialize(d, ret);
		Assert::AreEqual(ret.c_str(),"{\"id\":\"test\",\"status\":\"UKN\"}",L"ciupJsonSerialize ciupServerInfo", LINE_INFO());
		}*/

	};
}