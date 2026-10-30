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
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0setup.ps1" %*
pause
