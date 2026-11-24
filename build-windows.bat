@echo off
setlocal
cd /d "%~dp0"
where g++ >nul 2>nul
if errorlevel 1 (
 echo Install MinGW-w64 and add its bin directory to PATH.
 exit /b 1
)
windres native/app.rc -O coff -o native/resources.o
if errorlevel 1 exit /b 1
g++ native/main.cpp native/resources.o -o LuckyBreak.exe -std=c++17 -O2 -s -static -static-libgcc -static-libstdc++ -municode -mwindows -lgdiplus -lshlwapi -lole32 -lshell32 -lwinmm -lgdi32 -luser32
if errorlevel 1 exit /b 1
echo Built LuckyBreak.exe
