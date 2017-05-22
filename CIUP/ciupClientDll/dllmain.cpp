// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "CIUPclientDLL.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

__declspec(dllexport) ciupServerInfo __stdcall ciupcGetServerInfo(const char *addr, unsigned short port) {

	ciupServerInfo info;

	// TODO

	return info;
}

