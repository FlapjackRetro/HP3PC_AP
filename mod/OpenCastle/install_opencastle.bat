@echo off
rem Run as administrator. Copies HP3OpenCastle.asi into the game's plugins folder and HP3AP.utx into textures.
cd /d "%~dp0"
set "R=C:\Program Files (x86)\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)"
set "G=%R%\system\plugins"
if not exist "%G%" mkdir "%G%"
copy /y "HP3OpenCastle.asi" "%G%\HP3OpenCastle.asi"
if errorlevel 1 goto fail
if exist "HP3AP.utx" copy /y "HP3AP.utx" "%R%\textures\HP3AP.utx"
if errorlevel 1 goto fail
echo Installed HP3OpenCastle.asi and HP3AP.utx (run uninstall_opencastle.bat for the vanilla game)
goto end
:fail
echo FAILED - right-click the .bat and choose Run as administrator
:end
pause
