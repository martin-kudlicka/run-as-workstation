#include "stdafx.h"
#include "runasws.h"

#include <string>
#include <vector>
#include "../1stparty/inject/inject.h"

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
  wcscat_s(filePath, _countof(filePath), L"RunAsWsHook.dll");

  return inject(filePath, process);
}

DWORD run()
{
  auto processInformation = executeSuspended();
  if (!processInformation.hProcess)
  {
    return GetLastError();
  }

  auto injectEvent = createInjectEvent(processInformation.dwProcessId);
  auto error       = injectLibrary(processInformation.hProcess);

  if (injectEvent)
  {
    WaitForSingleObject(injectEvent, INFINITE);
    CloseHandle(injectEvent);
  }

  ResumeThread(processInformation.hThread);

  CloseHandle(processInformation.hThread);
  CloseHandle(processInformation.hProcess);

  return error;
}

void showHelp()
{
  wprintf(L"RunAsWs.exe \"application\" [arguments]\n\n");
  wprintf(L"application\tfile path to application to execute\n");
  wprintf(L"arguments\toptional arguments to pass to application\n");
}