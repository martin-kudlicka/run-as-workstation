#include "stdafx.h"

#include <process.h>
#include "runaswshook.h"

HMODULE g_module = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			g_module = hModule;
			hookNow();
			_beginthreadex(nullptr, 0, &hookThread, nullptr, 0, nullptr);
			break;
		case DLL_PROCESS_DETACH:
			uninstallHooks();
			g_module = nullptr;
			break;
	}

	return TRUE;
}