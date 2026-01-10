@echo off
setlocal
REM Simple wrapper to run the PowerShell build+run script.
REM Usage:
REM   build_run.cmd
REM   build_run.cmd Release

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\build_run.ps1" -Config %CONFIG%
set ERR=%ERRORLEVEL%
if not "%ERR%"=="0" exit /b %ERR%
endlocal
