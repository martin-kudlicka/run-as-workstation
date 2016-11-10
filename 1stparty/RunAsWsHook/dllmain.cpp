#include "stdafx.h"

#include <process.h>
#include "runaswshook.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	  case DLL_PROCESS_ATTACH:
      _beginthreadex(nullptr, 0, &hookThread, nullptr, 0, nullptr);
      break;
	  case DLL_PROCESS_DETACH:
      uninstallHooks();
		  break;
	}

	return TRUE;
}