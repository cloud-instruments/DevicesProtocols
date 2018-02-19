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

		// TODO unit test dll
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