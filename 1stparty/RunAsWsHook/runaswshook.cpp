#include "stdafx.h"
#include "runaswshook.h"

#include "hooks.h"
#include <mhook-lib/mhook.h>

HMODULE g_advapi32dll = nullptr;

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

void installHooks()
{
  g_advapi32dll = LoadLibrary(L"Advapi32.dll");
  if (!g_advapi32dll)
  {
    return;
  }

  g_regQueryValueExA = reinterpret_cast<RegQueryValueExAType>(GetProcAddress(g_advapi32dll, "RegQueryValueExA"));
  g_regQueryValueExW = reinterpret_cast<RegQueryValueExWType>(GetProcAddress(g_advapi32dll, "RegQueryValueExW"));

  hook(reinterpret_cast<PVOID *>(&g_regQueryValueExA), hook_RegQueryValueExA);
  hook(reinterpret_cast<PVOID *>(&g_regQueryValueExW), hook_RegQueryValueExW);
}

unsigned __stdcall hookThread(LPVOID arguments)
{
  installHooks();

  return ERROR_SUCCESS;
}

void unhook(PVOID *hookedFunction)
{
  if (!*hookedFunction)
  {
    return;
  }

  Mhook_Unhook(hookedFunction);
}

void uninstallHooks()
{
  if (!g_advapi32dll)
  {
    return;
  }

  unhook(reinterpret_cast<PVOID *>(&g_regQueryValueExA));
  unhook(reinterpret_cast<PVOID *>(&g_regQueryValueExW));

  FreeLibrary(g_advapi32dll);
}