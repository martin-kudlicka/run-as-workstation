#include "stdafx.h"
#include "hooks.h"

RegQueryValueExAType g_regQueryValueExA = nullptr;
RegQueryValueExWType g_regQueryValueExW = nullptr;

LONG WINAPI hook_RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
  auto error = g_regQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
  if (error == ERROR_SUCCESS && lpValueName && lpData)
  {
    if (strcmp(lpValueName, "ProductType") == 0 && strcmp(reinterpret_cast<char *>(lpData), "ServerNT") == 0)
    {
      strcpy_s(reinterpret_cast<char *>(lpData), *lpcbData, "WinNT");
      *lpcbData = strlen("WinNT") + 1;
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
      *lpcbData = (wcslen(L"WinNT") + 1) * sizeof(WCHAR);
    }
  }

  return error;
}