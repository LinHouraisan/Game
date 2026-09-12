@echo off
echo Building Effekseer in Release mode...

cd EffekseerForCpp170e
if not exist build_msvc2022_x64_release mkdir build_msvc2022_x64_release
cd build_msvc2022_x64_release

echo Generating Effekseer project files...
cmake -G "Visual Studio 17 2022" -A x64 -D CMAKE_INSTALL_PREFIX=../install_msvc2022_x64_release -DCMAKE_BUILD_TYPE=Release -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON ..

echo Building Effekseer (Release)...
cmake --build . --config Release --target install

cd ../..
echo Effekseer Release build complete.