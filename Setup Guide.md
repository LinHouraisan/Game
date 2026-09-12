# NCL CSC8508 Project By Team 6

&nbsp;
# 🎯 Requirements
- **Visual Studio 2022**
- **CMake: Version >= 3.16.0**
- **Git**

&nbsp;
# 💻 Project Setup
### 1. Prepare the Environment
- Ensure **Visual Studio 2022**, **CMake**, and **Git** are installed and properly configured in your environment variables.
### 2. Clone the Project Repository
- Use Git or a GUI client to clone the project repository:
```shellscript
git clone https://github.com/deep-river/NCL8508
cd NCL8508
```
### 3. Generate the Project Solution (`.sln` file)
- #### Method 1: Using the configure.bat script
    - Run the configure.bat script from the project root directory.
    - After execution, the `.sln` solution file will be generated in the project root directory.
- #### Method 2: Using CMake GUI
    - Open CMake GUI.
    - Set "Source path" to the project root directory (`NCL8508`).
    - Set "Build path" to the project root directory.
    - Click "Configure" and select "Visual Studio 17 2022" as the generator.
    - Click "Generate" to create the `.sln` file.
### 4. Install vcpkg and Third-party Libraries (`bullet3`, `GLFW3`, etc.)
- #### If you haven't installed vcpkg before:
    - Copy install_vcpkg.bat to another directory on your computer (e.g., Dev).
    - Run install_vcpkg.bat to install vcpkg and bullet3 in the current directory.
    - The script will automatically complete the installation and configuration of vcpkg and `bullet3`.
- #### If you have already installed vcpkg:
    Use the following commands to install the required libraries:
```shellscript
vcpkg install bullet3
vcpkg install glfw3
vcpkg install assimp
vcpkg install glm
```
### 5. Compile and Run the Project
- Open the `.sln` file in the Build directory and compile/run it in Visual Studio 2022.

&nbsp;
# 🛠️ Troubleshooting
## 1. Error `LNK2038: _ITERATOR_DEBUG_LEVEL` Mismatch  
### **Symptom:**  
- The following error appears during compilation:  
```plaintext
Error LNK2038 mismatch detected for "_ITERATOR_DEBUG_LEVEL": value "0" doesn't match value "2" (in cmake_pch.obj)
```
### **Cause:**  
- The project build mode (Debug/Release) doesn't match the compilation mode of the referenced Bullet3 library:  
- **Debug mode** requires linking to the Debug version of libraries.  
- **Release mode** requires linking to the Release version of libraries.  
### **Solution:**  
- **Open Project Properties**  
   - In Solution Explorer, right-click on the **CSC8503** project → select **Properties**.  
- **Configure Mode Consistency**  
   - In the properties page upper-left corner, select the current build mode (e.g., `Debug` or `Release`).  
   - Navigate to **Configuration Properties → vcpkg**.  
   - Ensure the **Vcpkg Configuration** value on the right matches the current configuration mode:  
     - `Debug` configuration → select `Debug`  
     - `Release` configuration → select `Release`  
- **Save and Recompile**  
   - Click **OK** to save the configuration.  
   - Clean the solution (**Build → Clean Solution**), then recompile.