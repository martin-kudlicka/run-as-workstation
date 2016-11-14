#include "stdafx.h"
#include "inject.h"

#include <stdlib.h>
#include <string>

std::wstring injectEventName(DWORD id);

HANDLE createInjectEvent(DWORD id)
{
  return CreateEvent(nullptr, TRUE, FALSE, injectEventName(id).c_str());
}

DWORD inject(HMODULE module, HANDLE process)
{
  WCHAR filePath[MAX_PATH];
  auto chars = GetModuleFileName(module, filePath, _countof(filePath));
  if (chars == 0)
  {
    return GetLastError();
  }

  return inject(filePath, process);
}

DWORD inject(LPCWSTR filePath, HANDLE process)
{
  auto memory = VirtualAllocEx(process, nullptr, (wcslen(filePath) + 1) * sizeof(WCHAR), MEM_COMMIT, PAGE_READWRITE);
  if (!memory)
  {
    return GetLastError();
  }

  auto error = ERROR_SUCCESS;

  do
  {
    auto ok = WriteProcessMemory(process, memory, filePath, (wcslen(filePath) + 1) * sizeof(WCHAR), nullptr);
    if (!ok)
    {
      error = GetLastError();
      break;
    }

    auto kernel32dll = GetModuleHandle(L"Kernel32.dll");
    if (!kernel32dll)
    {
      error = GetLastError();
      break;
    }

    auto loadLibraryFnc = GetProcAddress(kernel32dll, "LoadLibraryW");
    if (!loadLibraryFnc)
    {
      error = GetLastError();
      break;
    }

    auto remoteThread = CreateRemoteThread(process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryFnc), memory, 0, nullptr);
    if (!remoteThread)
    {
      error = GetLastError();
      break;
    }
  } while (false);

  if (error != ERROR_SUCCESS)
  {
    VirtualFreeEx(process, memory, 0, MEM_RELEASE);
  }

  return error;
}

std::wstring injectEventName(DWORD id)
{
  WCHAR buffer[32] = { 0 };
  wsprintf(buffer, L"RunAsWsHook_%d", id);

  return buffer;
}

HANDLE openInjectEvent(DWORD id)
{
  return OpenEvent(EVENT_MODIFY_STATE, FALSE, injectEventName(id).c_str());
}