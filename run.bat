@echo off
echo Running Tank-Boy game...

REM Check if executable exists
if not exist "TankBoy\x64\Debug\TankBoy.exe" (
    echo Error: TankBoy.exe not found!
    echo Please build the project first using build.bat
    pause
    exit /b 1
)

REM Run the game
echo Starting Tank-Boy...
TankBoy\x64\Debug\TankBoy.exe

REM Game finished, stay in current directory
echo Game finished.
pause
