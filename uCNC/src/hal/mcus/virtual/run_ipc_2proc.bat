@echo off
rem Runs the uCNC split as two independent processes that talk over a pair of
rem Windows named pipes (see the virtual_windows.c).
rem
rem The two are separate boards, not two cores of one chip: each is a one sided
rem build with its own hardware, so the RT side produces the step trace and
rem serves the IO dashboard, and the NonRT side owns the console, the streams,
rem the settings and the network. That is the ESP8266/AVR pairing, emulated on
rem one host instead of two boards.
rem
rem Build both first:
rem   pio run -e EMULATOR_WINDOWS_IPC_NONRT -e EMULATOR_WINDOWS_IPC_RT
rem
rem Usage: run_ipc_2proc.bat [build directory]
rem
rem The RT process is the pipe server, so the two may be started in any order.
rem It prints only when a step trace or dashboard is involved, so it is given its
rem own window to keep its output readable while debugging.
setlocal

set "BUILD=%~1"
if "%BUILD%"=="" set "BUILD=%~dp0..\..\..\..\..\.pio\build"

set "RT=%BUILD%\EMULATOR_WINDOWS_IPC_RT\program.exe"
set "NONRT=%BUILD%\EMULATOR_WINDOWS_IPC_NONRT\program.exe"

if not exist "%RT%" (
  echo Cannot find "%RT%".
  echo Build both with: pio run -e EMULATOR_WINDOWS_IPC_NONRT -e EMULATOR_WINDOWS_IPC_RT
  exit /b 1
)
if not exist "%NONRT%" (
  echo Cannot find "%NONRT%".
  echo Build both with: pio run -e EMULATOR_WINDOWS_IPC_NONRT -e EMULATOR_WINDOWS_IPC_RT
  exit /b 1
)

start "uCNC RT" "%RT%"

rem The NonRT side owns the console, so this is the one to type gcode at.
"%NONRT%"

rem The NonRT side exited. Stop the RT side too, matching only our own window so
rem that an unrelated program.exe is not touched.
taskkill /FI "WINDOWTITLE eq uCNC RT*" /F >nul 2>&1
endlocal
