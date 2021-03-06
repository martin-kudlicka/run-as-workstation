#include "stdafx.h"
#include "runaswshook.h"

#include "hooks.h"
#include <mhook-lib/mhook.h>
#include <MkInject/MInject>

HMODULE g_advapi32dll = nullptr;
HMODULE g_kernel32dll = nullptr;

void hook(PVOID *systemFunction, PVOID hookFunction)
{
  if (!*systemFunction)
  {
    return;
  }

  auto ok = Mhook_SetHook(systemFunction, hookFunction);
  if (!ok)
  {
    *systemFunction = nullptr;
  }
}

void hookNow()
{
  g_kernel32dll = GetModuleHandle(L"kernel32.dll");

  g_createProcessA = reinterpret_cast<CreateProcessAType>(GetProcAddress(g_kernel32dll, "CreateProcessA"));
  g_createProcessW = reinterpret_cast<CreateProcessWType>(GetProcAddress(g_kernel32dll, "CreateProcessW"));
  g_getVersionExA  = reinterpret_cast<GetVersionExAType> (GetProcAddress(g_kernel32dll, "GetVersionExA"));
  g_getVersionExW  = reinterpret_cast<GetVersionExWType> (GetProcAddress(g_kernel32dll, "GetVersionExW"));

  hook(reinterpret_cast<PVOID *>(&g_createProcessA), hook_CreateProcessA);
  hook(reinterpret_cast<PVOID *>(&g_createProcessW), hook_CreateProcessW);
  hook(reinterpret_cast<PVOID *>(&g_getVersionExA),  hook_GetVersionExA);
  hook(reinterpret_cast<PVOID *>(&g_getVersionExW),  hook_GetVersionExW);
}

unsigned __stdcall hookThread(LPVOID arguments)
{
  UNREFERENCED_PARAMETER(arguments);

  auto injectEvent = MInject::openInjectEvent(GetCurrentProcessId());

  DWORD error = ERROR_SUCCESS;

  do
  {
    g_advapi32dll = LoadLibrary(L"Advapi32.dll");
    if (!g_advapi32dll)
    {
      error = GetLastError();
      break;
    }

    g_regQueryValueExA = reinterpret_cast<RegQueryValueExAType>(GetProcAddress(g_advapi32dll, "RegQueryValueExA"));
    g_regQueryValueExW = reinterpret_cast<RegQueryValueExWType>(GetProcAddress(g_advapi32dll, "RegQueryValueExW"));

    hook(reinterpret_cast<PVOID *>(&g_regQueryValueExA), hook_RegQueryValueExA);
    hook(reinterpret_cast<PVOID *>(&g_regQueryValueExW), hook_RegQueryValueExW);
  }
  while (false);

  if (injectEvent)
  {
    SetEvent(injectEvent);
  }

  return error;
}

void unhook(PVOID *hookedFunction)
{
  if (!*hookedFunction)
  {
    return;
  }

  Mhook_Unhook(hookedFunction);
  *hookedFunction = nullptr;
}

void uninstallHooks()
{
  unhook(reinterpret_cast<PVOID *>(&g_regQueryValueExW));
  unhook(reinterpret_cast<PVOID *>(&g_regQueryValueExA));
  unhook(reinterpret_cast<PVOID *>(&g_getVersionExW));
  unhook(reinterpret_cast<PVOID *>(&g_getVersionExA));
  unhook(reinterpret_cast<PVOID *>(&g_createProcessW));
  unhook(reinterpret_cast<PVOID *>(&g_createProcessA));

  g_kernel32dll = nullptr;
  if (g_advapi32dll)
  {
    FreeLibrary(g_advapi32dll);
    g_advapi32dll = nullptr;
  }
}