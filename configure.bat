@echo off
REM Setting code page to UTF-8
chcp 65001 >nul

REM Define project root directory
set ROOT_DIR=%CD%

REM Clean CMake cache files
if exist "%ROOT_DIR%\CMakeCache.txt" (
    echo Deleting CMakeCache.txt...
    del /F /Q "%ROOT_DIR%\CMakeCache.txt"
)

if exist "%ROOT_DIR%\CMakeFiles" (
    echo Deleting CMakeFiles directory...
    rmdir /S /Q "%ROOT_DIR%\CMakeFiles"
)

if exist "%ROOT_DIR%\Makefile" (
    echo Deleting Makefile...
    del /F /Q "%ROOT_DIR%\Makefile"
)

if exist "%ROOT_DIR%\cmake_install.cmake" (
    echo Deleting cmake_install.cmake...
    del /F /Q "%ROOT_DIR%\cmake_install.cmake"
)

REM Generate Visual Studio project files
cmake -G "Visual Studio 17 2022" -A x64 .
echo Visual Studio project files generated, please open the CSC8503.sln file in the project folder.

pause