// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

HANDLE gCommMutex = INVALID_HANDLE_VALUE;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		/*if (gCommMutex == INVALID_HANDLE_VALUE)
			gCommMutex = CreateMutex(NULL, TRUE, "gCommMutex");*/
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		/*if(gCommMutex != INVALID_HANDLE_VALUE)
		{
			CloseHandle(gCommMutex);
			gCommMutex = INVALID_HANDLE_VALUE;
		}*/
		break;
	}
	return TRUE;
}

