// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

ofstream outfile;

#ifdef _WIN32

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
            outfile.open(L"C:\\temp\\J2534.txt", ios::out);
            outfile << "attach\n";
            break;
		case DLL_PROCESS_DETACH:
            outfile << "detatch\n";
            outfile.close();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

#else
/*
__attribute__((constructor)) void DllLoad() {
	outfile.open("/tmp/J2534.txt", ios::out);
	outfile << "attach\n";
}

__attribute__((destructor)) void DllUnload() {
	outfile << "detatch\n";
	outfile.close();
}
*/
#endif