#pragma once

DWORD inject(HMODULE module, HANDLE process);
DWORD inject(LPCWSTR filePath, HANDLE process);