@echo off
REM Setting code page to UTF-8
chcp 65001 >nul
setlocal EnableDelayedExpansion

:: Set vcpkg directory
set VCPKG_ROOT=%CD%\vcpkg

:: Download and install vcpkg
echo Downloading vcpkg...
git clone https://github.com/Microsoft/vcpkg.git
if not exist "%VCPKG_ROOT%" (
    echo Failed to download vcpkg.
    exit /b 1
)

cd vcpkg
echo Compiling vcpkg...
call bootstrap-vcpkg.bat
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo vcpkg compilation failed.
    exit /b 1
)

echo Configuring vcpkg...
call vcpkg integrate install

:: Install Bullet 3
call vcpkg install bullet3
if %errorlevel% neq 0 (
    echo Bullet3 installation failed.
    exit /b 1
)
echo Bullet3 installation successful!

:: Install GLFW3
call vcpkg install glfw3
if %errorlevel% neq 0 (
    echo GLFW3 installation failed.
    exit /b 1
)
echo GLFW3 installation successful!

:: Install Assimp
call vcpkg install assimp
if %errorlevel% neq 0 (
    echo Assimp installation failed.
    exit /b 1
)
echo Assimp installation successful!

:: Install GLM
call vcpkg install glm
if %errorlevel% neq 0 (
    echo GLM installation failed.
    exit /b 1
)
echo GLM installation successful!

cd ..
pause