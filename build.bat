@echo off
setlocal
rem Finds Visual Studio and builds a Release exe into bin\ with its bundled CMake.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS=%%i"
if not defined VS (
    echo Visual Studio with the C++ tools was not found.
    exit /b 1
)
set "CMAKE=%VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
cd /d "%~dp0"
"%CMAKE%" -S . -B build -A x64 || exit /b 1
"%CMAKE%" --build build --config Release || exit /b 1
echo.
echo Built: %~dp0bin\Veyreon Injector.exe
