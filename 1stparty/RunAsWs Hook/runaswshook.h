#pragma once

void               hookNow       ();
unsigned __stdcall hookThread    (LPVOID arguments);
void               uninstallHooks();