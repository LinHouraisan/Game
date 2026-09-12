pipeline {
    agent any
    
    triggers {
        pollSCM('H/20 * * * *')
    }
    
    stages {
        stage('Environment Info') {
            steps {
                bat '@echo off && echo Building with Jenkins && echo Current directory: %CD% && dir'
            }
        }
        
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        
        stage('Check Effekseer Files') {
            steps {
                bat '''
                    @echo off
                    echo Checking Effekseer files...
                    
                    echo Release includes:
                    if exist "EffekseerForCpp170e\\install_msvc2022_x64_release\\include" (
                        dir "EffekseerForCpp170e\\install_msvc2022_x64_release\\include"
                        if exist "EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\Effekseer" (
                            echo Effekseer subdirectory found
                            dir "EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\Effekseer"
                        )
                        if exist "EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\Effekseer.h" (
                            echo Effekseer.h found in root
                        ) else (
                            echo Effekseer.h NOT found in root
                        )
                    ) else (
                        echo Release include directory not found
                        mkdir -p "EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\Effekseer"
                    )
                    
                    echo Debug includes:
                    if exist "EffekseerForCpp170e\\install_msvc2022_x64\\include" (
                        dir "EffekseerForCpp170e\\install_msvc2022_x64\\include"
                    )
                '''
            }
        }
        
        stage('Fix Header Files') {
            steps {
                bat '''
                    @echo off
                    echo Fixing header file includes...
                    
                    REM 检查是否需要创建符号链接
                    if not exist "EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\Effekseer.h" (
                        if exist "EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\Effekseer\\Effekseer.h" (
                            echo Copying Effekseer.h to include root
                            copy "EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\Effekseer\\Effekseer.h" "EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\"
                        )
                    )
                    
                    REM 为可能使用相对路径的头文件创建符号链接
                    if exist "EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\EffekseerRendererGL\\EffekseerRendererGL.h" (
                        echo Patching EffekseerRendererGL.h...
                        powershell -Command "(Get-Content 'EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\EffekseerRendererGL\\EffekseerRendererGL.h') -replace '#include <Effekseer.h>', '#include <Effekseer/Effekseer.h>' | Set-Content 'EffekseerForCpp170e\\install_msvc2022_x64_release\\include\\EffekseerRendererGL\\EffekseerRendererGL.h'"
                    )
                '''
            }
        }
        
        stage('Install Dependencies') {
            steps {
                bat '''
                    @echo off
                    where vcpkg >nul 2>&1
                    if %errorlevel% neq 0 (
                        echo Using local vcpkg if available
                        if exist "vcpkg\\vcpkg.exe" (
                            set VCPKG_PATH=.\\vcpkg\\vcpkg.exe
                        ) else (
                            echo Error: vcpkg not found
                            exit 1
                        )
                    ) else (
                        set VCPKG_PATH=vcpkg
                    )
                    
                    %VCPKG_PATH% install bullet3:x64-windows
                    %VCPKG_PATH% install glfw3:x64-windows
                    %VCPKG_PATH% install assimp:x64-windows
                    %VCPKG_PATH% install glm:x64-windows
                '''
            }
        }
        
        stage('Build') {
            steps {
                bat '''
                    @echo off
                    if not exist "build" mkdir build
                    cd build
                    
                    REM 查找vcpkg工具链文件
                    set VCPKG_TOOLCHAIN_PATH=
                    if exist "C:\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake" (
                        set VCPKG_TOOLCHAIN_PATH=C:\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake
                    ) else if exist "%VCPKG_ROOT%\\scripts\\buildsystems\\vcpkg.cmake" (
                        set VCPKG_TOOLCHAIN_PATH=%VCPKG_ROOT%\\scripts\\buildsystems\\vcpkg.cmake
                    ) else if exist "..\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake" (
                        set VCPKG_TOOLCHAIN_PATH=..\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake
                    )
                    
                    if defined VCPKG_TOOLCHAIN_PATH (
                        cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_TOOLCHAIN_PATH%" -DCMAKE_BUILD_TYPE=Release ..
                    ) else (
                        cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..
                    )
                    
                    cmake --build . --config Release
                '''
            }
        }
        
        stage('Copy Artifacts') {
            steps {
                bat '''
                    @echo off
                    if not exist "artifacts" mkdir artifacts
                    
                    REM 复制主可执行文件
                    copy /Y "build\\CSC8503\\Release\\CSC8503.exe" "artifacts\\"
                    
                    REM 复制项目DLL
                    copy /Y "build\\NCLCoreClasses\\Release\\NCLCoreClasses.dll" "artifacts\\"
                    copy /Y "build\\CSC8503CoreClasses\\Release\\CSC8503CoreClasses.dll" "artifacts\\"
                    copy /Y "build\\OpenGLRendering\\Release\\OpenGLRendering.dll" "artifacts\\"
                    
                    REM 复制vcpkg DLL
                    if exist "C:\\vcpkg\\installed\\x64-windows\\bin" (
                        copy /Y "C:\\vcpkg\\installed\\x64-windows\\bin\\*.dll" "artifacts\\"
                    ) else if exist "%VCPKG_ROOT%\\installed\\x64-windows\\bin" (
                        copy /Y "%VCPKG_ROOT%\\installed\\x64-windows\\bin\\*.dll" "artifacts\\"
                    ) else if exist "..\\vcpkg\\installed\\x64-windows\\bin" (
                        copy /Y "..\\vcpkg\\installed\\x64-windows\\bin\\*.dll" "artifacts\\"
                    )
                    
                    REM 复制Effekseer DLL (直接从repo中复制)
                    copy /Y "EffekseerForCpp170e\\install_msvc2022_x64_release\\bin\\*.dll" "artifacts\\"
                    
                    REM 复制FMOD和Noesis DLL
                    copy /Y "CSC8503\\FMod\\lib\\*.dll" "artifacts\\"
                    copy /Y "CSC8503\\NoesisGUI\\Bin\\windows_x86_64\\*.dll" "artifacts\\"
                    
                    REM 复制Assets文件夹
                    if exist "Assets" (
                        xcopy /E /I /Y "Assets" "artifacts\\Assets"
                    )
                '''
            }
        }
        
        stage('Archive') {
            steps {
                archiveArtifacts artifacts: 'artifacts/**', fingerprint: true, allowEmptyArchive: true
            }
        }
    }
}