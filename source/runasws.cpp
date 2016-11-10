#include "runasws.h"

#include <string>
#include <vector>

PROCESS_INFORMATION executeSuspended()
{
  std::wstring commandLine;
  for (auto index = 1; index < __argc; index++)
  {
    if (!commandLine.empty())
    {
      commandLine.push_back(L' ');
    }

    auto containSpace = wcschr(__wargv[index], L' ');

    if (containSpace)
    {
      commandLine.push_back(L'\"');
    }
    commandLine.append(__wargv[index]);
    if (containSpace)
    {
      commandLine.push_back(L'\"');
    }
  }

  std::vector<WCHAR> cmdBuffer(commandLine.length() + 1);
  wcscpy_s(cmdBuffer.data(), cmdBuffer.size(), commandLine.c_str());
  STARTUPINFO startupInfo                = { 0 };
  startupInfo.cb                         = sizeof(startupInfo);
  PROCESS_INFORMATION processInformation = { 0 };
  auto ok                                = CreateProcess(nullptr, cmdBuffer.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &startupInfo, &processInformation);
  if (!ok)
  {
    wprintf(L"failed to execute %s\n", __wargv[1]);
    wprintf(L"error code: %d", GetLastError());
  }

  return processInformation;
}

DWORD injectLibrary(HANDLE process)
{
  WCHAR filePath[MAX_PATH];
  auto chars = GetModuleFileName(nullptr, filePath, _countof(filePath));
  if (chars == 0)
  {
    return GetLastError();
  }
  auto lastSlash = wcsrchr(filePath, L'\\');
  if (!lastSlash)
  {
    return ERROR_PATH_NOT_FOUND;
  }
  *(lastSlash + 1) = 0;
  wcscat_s(filePath, _countof(filePath),  L"RunAsWsHook.dll");

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
  }
  while (false);

  if (error != ERROR_SUCCESS)
  {
    VirtualFreeEx(process, memory, 0, MEM_RELEASE);
  }

  return error;
}

DWORD run()
{
  auto processInformation = executeSuspended();
  if (!processInformation.hProcess)
  {
    return GetLastError();
  }

  auto error = injectLibrary(processInformation.hProcess);

  ResumeThread(processInformation.hThread);

  CloseHandle(processInformation.hThread);
  CloseHandle(processInformation.hProcess);

  return error;
}

void showHelp()
{
  // TODO
}