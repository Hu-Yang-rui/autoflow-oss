# AutoFlow 一键构建脚本（CMake + Ninja + MSVC）
# 用法:  powershell -ExecutionPolicy Bypass -File build.ps1
param(
    [switch]$Release,
    [switch]$Deploy        # 构建后运行 windeployqt，生成绿色免安装目录
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$QtDir = "C:\Qt\6.8.3\msvc2022_64"
$VcVars = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
$VsCmake = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
$VsNinja = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"

if (-not (Test-Path $QtDir))   { throw "未找到 Qt: $QtDir" }
if (-not (Test-Path $VcVars))  { throw "未找到 MSVC: $VcVars" }

$BuildDir = Join-Path $Root "build"
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

$Cfg = if ($Release) { "Release" } else { "Debug" }
$DeployArg = if ($Deploy) { "&& cmake --build . --target deploy" } else { "" }

$Cmd = "call `"$VcVars`" x64 >nul && " +
       "set PATH=$VsNinja;$VsCmake;%PATH% && " +
       "set Qt6_DIR=$QtDir\lib\cmake\Qt6 && " +
       "cmake -S `"$Root`" -B `"$BuildDir`" -G Ninja " +
       "-DCMAKE_BUILD_TYPE=$Cfg " +
       "-DCMAKE_PREFIX_PATH=`"$QtDir`" " +
       "&& cmake --build `"$BuildDir`"" +
       "$DeployArg"

cmd /c $Cmd
if ($LASTEXITCODE -ne 0) { throw "构建失败，退出码 $LASTEXITCODE" }

$Exe = Join-Path $BuildDir "bin\AutoFlow.exe"
Write-Host ""
Write-Host "=== 构建成功 ===" -ForegroundColor Green
Write-Host "产物: $Exe"
