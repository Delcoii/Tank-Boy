@echo off
echo Building Tank-Boy project...

REM Set Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

msbuild Tankboy.sln /p:Configuration=Debug /p:Platform=x64

REM Build entire solution (not just the project)
echo Building entire solution Debug x64...
msbuild Tankboy.sln /p:Configuration=Debug /p:Platform=x64 /verbosity:normal

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Executable location: x64/Debug/TankBoy.exe
) else (
    echo Build failed!
    pause
)