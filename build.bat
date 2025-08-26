@echo off
echo Building Tank-Boy project...

REM Set Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Build Debug configuration
echo Building Debug x64...
msbuild TankBoy/TankBoy.vcxproj /p:Configuration=Debug /p:Platform=x64

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Executable location: TankBoy/x64/Debug/TankBoy.exe
) else (
    echo Build failed!
    pause
)