#pragma once

HANDLE createInjectEvent(DWORD id);
DWORD  inject           (HMODULE module, HANDLE process);
DWORD  inject           (LPCWSTR filePath, HANDLE process);
HANDLE openInjectEvent  (DWORD id);