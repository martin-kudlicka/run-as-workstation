# Run As Workstation

Tries to run applications designed for Windows Workstation only also on Windows Server.

## Description
Run As Workstation catches wProductType of GetVersionEx WinAPI call and changes it to VER_NT_WORKSTATION value. It also tries to do this in every child process.

## Usage
`RunAsWs.exe "C:\application.exe" arguments`

## Requirements
* Windows Server 2008
* [Visual C++ Redistributable for Visual Studio 2019](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)

## Components
* [mhook](https://github.com/martona/mhook)
