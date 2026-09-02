@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" x64 >nul
if errorlevel 1 exit /b 1
set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
cmake -S "D:\opencode\autoflow-cpp" -B "D:\opencode\autoflow-cpp\build-release" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Qt\6.8.3\msvc2022_64"
if errorlevel 1 exit /b 1
cmake --build "D:\opencode\autoflow-cpp\build-release"
if errorlevel 1 exit /b 1
cmake --build "D:\opencode\autoflow-cpp\build-release" --target deploy
exit /b %errorlevel%
