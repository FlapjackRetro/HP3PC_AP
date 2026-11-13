@echo off
rem Runs setup.ps1 as administrator (the game folder is under Program Files). Extra arguments go to setup.ps1, e.g.
rem   setup.bat -DebugBuild -MapTracker
rem Run setup.ps1 -? for every option. For a -GameDir with spaces, run setup.ps1 from an elevated PowerShell instead.
cd /d "%~dp0"
net session >nul 2>&1
if %errorlevel% neq 0 (
    powershell -NoProfile -Command "$a = '%*'; if ($a) { Start-Process -Verb RunAs -FilePath '%~f0' -ArgumentList $a } else { Start-Process -Verb RunAs -FilePath '%~f0' }"
    exit /b
)
rem always the 64-bit PowerShell: a 32-bit one sees "Program Files (x86)" as %ProgramFiles% and the 32-bit registry
set "PS=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
if exist "%SystemRoot%\Sysnative\WindowsPowerShell\v1.0\powershell.exe" set "PS=%SystemRoot%\Sysnative\WindowsPowerShell\v1.0\powershell.exe"
"%PS%" -NoProfile -ExecutionPolicy Bypass -File "%~dp0setup.ps1" %*
pause
