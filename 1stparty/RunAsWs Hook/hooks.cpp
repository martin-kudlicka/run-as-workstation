#include "stdafx.h"
#include "hooks.h"

#include <MkInject/MInject>

CreateProcessAType g_createProcessA     = nullptr;
CreateProcessWType g_createProcessW     = nullptr;
GetVersionExAType g_getVersionExA       = nullptr;
GetVersionExWType g_getVersionExW       = nullptr;
RegQueryValueExAType g_regQueryValueExA = nullptr;
RegQueryValueExWType g_regQueryValueExW = nullptr;

extern HMODULE g_module;

void injectAndWait(const LPPROCESS_INFORMATION processInformation);

BOOL WINAPI hook_CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
  auto suspended = dwCreationFlags & CREATE_SUSPENDED;
  if (!suspended)
  {
    dwCreationFlags |= CREATE_SUSPENDED;
  }

  auto ok = g_createProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
  if (ok)
  {
    injectAndWait(lpProcessInformation);

    if (!suspended)
    {
      ResumeThread(lpProcessInformation->hThread);
    }
  }

  return ok;
}

BOOL WINAPI hook_CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
  auto suspended = dwCreationFlags & CREATE_SUSPENDED;
  if (!suspended)
  {
    dwCreationFlags |= CREATE_SUSPENDED;
  }

  auto ok = g_createProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
  if (ok)
  {
    injectAndWait(lpProcessInformation);

    if (!suspended)
    {
      ResumeThread(lpProcessInformation->hThread);
    }
  }

  return ok;
}

BOOL WINAPI hook_GetVersionExA(LPOSVERSIONINFO lpVersionInfo)
{
  auto ok = g_getVersionExA(lpVersionInfo);
  if (ok)
  {
    if (lpVersionInfo->dwOSVersionInfoSize == sizeof(OSVERSIONINFOEX))
    {
      auto osVersionInfoEx          = reinterpret_cast<LPOSVERSIONINFOEX>(lpVersionInfo);
      osVersionInfoEx->wProductType = VER_NT_WORKSTATION;
    }
  }

  return ok;
}

BOOL WINAPI hook_GetVersionExW(LPOSVERSIONINFO lpVersionInfo)
{
  auto ok = g_getVersionExW(lpVersionInfo);
  if (ok)
  {
    if (lpVersionInfo->dwOSVersionInfoSize == sizeof(OSVERSIONINFOEX))
    {
      auto osVersionInfoEx          = reinterpret_cast<LPOSVERSIONINFOEX>(lpVersionInfo);
      osVersionInfoEx->wProductType = VER_NT_WORKSTATION;
    }
  }

  return ok;
}

LONG WINAPI hook_RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
  auto error = g_regQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
  if (error == ERROR_SUCCESS && lpValueName && lpData)
  {
    if (strcmp(lpValueName, "ProductType") == 0 && strcmp(reinterpret_cast<char *>(lpData), "ServerNT") == 0)
    {
      strcpy_s(reinterpret_cast<char *>(lpData), *lpcbData, "WinNT");
      *lpcbData = static_cast<DWORD>(strlen("WinNT") + 1);
    }
  }

  return error;
}

LONG WINAPI hook_RegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
  auto error = g_regQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
  if (error == ERROR_SUCCESS && lpValueName && lpData)
  {
    if (wcscmp(lpValueName, L"ProductType") == 0 && wcscmp(reinterpret_cast<wchar_t *>(lpData), L"ServerNT") == 0)
    {
      wcscpy_s(reinterpret_cast<wchar_t *>(lpData), *lpcbData, L"WinNT");
      *lpcbData = static_cast<DWORD>((wcslen(L"WinNT") + 1) * sizeof(WCHAR));
    }
  }

  return error;
}

void injectAndWait(const LPPROCESS_INFORMATION processInformation)
{
  auto injectEvent = MInject::createInjectEvent(processInformation->dwProcessId);
  MInject::inject(g_module, processInformation->hProcess);

  if (injectEvent)
  {
    WaitForSingleObject(injectEvent, INFINITE);
    CloseHandle(injectEvent);
  }
}