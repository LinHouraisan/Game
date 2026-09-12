@echo off
echo Checking Effekseer libraries...

REM Use the full path to dumpbin.exe
set DUMPBIN="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\bin\Hostx64\x64\dumpbin.exe"

echo Using dumpbin at: %DUMPBIN%

REM Check if Effekseer release directory exists
if not exist "EffekseerForCpp170e\install_msvc2022_x64_release\lib" (
    echo Release directory not found: EffekseerForCpp170e\install_msvc2022_x64_release\lib
    goto :check_debug
)

echo.
echo ======= CHECKING RELEASE VERSION OF LIBRARIES =======

cd EffekseerForCpp170e\install_msvc2022_x64_release\lib

REM Check Effekseer.lib
echo Checking Release Effekseer.lib...
%DUMPBIN% /DIRECTIVES Effekseer.lib | findstr /i "runtime debug release"

REM Check EffekseerRendererGL.lib if it exists
if exist "EffekseerRendererGL.lib" (
    echo.
    echo Checking Release EffekseerRendererGL.lib...
    %DUMPBIN% /DIRECTIVES EffekseerRendererGL.lib | findstr /i "runtime debug release"
)

cd ..\..\..

:check_debug
REM Check if Effekseer debug directory exists
if not exist "EffekseerForCpp170e\install_msvc2022_x64\lib" (
    echo Debug directory not found: EffekseerForCpp170e\install_msvc2022_x64\lib
    goto :end
)

echo.
echo ======= CHECKING DEBUG VERSION OF LIBRARIES =======

cd EffekseerForCpp170e\install_msvc2022_x64\lib

REM Check Effekseer.lib
echo Checking Debug Effekseer.lib...
%DUMPBIN% /DIRECTIVES Effekseer.lib | findstr /i "runtime debug release"

REM Check EffekseerRendererGL.lib if it exists
if exist "EffekseerRendererGL.lib" (
    echo.
    echo Checking Debug EffekseerRendererGL.lib...
    %DUMPBIN% /DIRECTIVES EffekseerRendererGL.lib | findstr /i "runtime debug release"
)

cd ..\..\..

:end
echo.
echo Library check complete.

REM Compare file sizes between debug and release libraries
echo.
echo Comparing file sizes (debug vs release):
echo ---------------------------------------
if exist "EffekseerForCpp170e\install_msvc2022_x64\lib\Effekseer.lib" (
    for %%F in (EffekseerForCpp170e\install_msvc2022_x64\lib\Effekseer.lib) do set DEBUG_SIZE=%%~zF
    echo Debug Effekseer.lib: %DEBUG_SIZE% bytes
)

if exist "EffekseerForCpp170e\install_msvc2022_x64_release\lib\Effekseer.lib" (
    for %%F in (EffekseerForCpp170e\install_msvc2022_x64_release\lib\Effekseer.lib) do set RELEASE_SIZE=%%~zF
    echo Release Effekseer.lib: %RELEASE_SIZE% bytes
)