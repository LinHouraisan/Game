# NCL CSC8508 Project By Team 6

&nbsp;
# 🎯 环境要求
- **Visual Studio 2022**
- **CMake：版本 >= 3.16.0**
- **Git**

&nbsp;
# 💻 项目准备
### 1. 准备环境
- 确保已安装 **Visual Studio 2022**、**CMake** 和 **Git**，并正确配置环境变量。
### 2. 拉取项目代码
- 使用 Git 或 fork 图形界面克隆项目代码仓库：
```shellscript
git clone https://github.com/deep-river/NCL8508
cd NCL8508
```
### 3. 生成项目方案 (`.sln` 文件)
- #### 方法1：使用 `configure.bat` 脚本
    - 在项目根目录执行 `configure.bat` 脚本。
    - 运行后，项目文件夹根目录中将生成 `.sln` 解决方案文件。
- #### 方法2：使用 CMake GUI
    - 打开 CMake GUI。
    - 设置"源代码路径"为项目根目录（`CL8508`）。
    - 设置"构建路径"为项目根目录。
    - 点击"Configure"，选择生成器为"Visual Studio 17 2022"。
    - 点击"Generate"生成 `.sln` 文件。
### 4. 安装 `vcpkg` 和 `bullet3`, `GLFW3` 等第三方库
- #### 如果没有安装过 vcpkg：
    - 将 `install_vcpkg.bat` 复制到计算机的其他目录（如 `C:\Dev`）。
    - 执行 `install_vcpkg.bat` 以在当前目录下安装 vcpkg 和 bullet3。
    - 该脚本会自动完成 `vcpkg` 和 `bullet3` 的安装与配置。
- #### 如果已经安装了 vcpkg：
    则直接使用下面的命令安装 bullet3, GLFW3, Assimp, glm 库：
```shellscript
vcpkg install bullet3
vcpkg install glfw3
vcpkg install assimp
vcpkg install glm
```
### 5. 编译和运行项目
- 打开 `Build` 目录中的 `.sln` 文件，并在 Visual Studio 2022 中进行编译和运行。

&nbsp;
# 🛠️ 故障排除
## 1. 错误 `LNK2038: _ITERATOR_DEBUG_LEVEL` 不匹配  
### **错误现象:**  
- 编译时出现以下错误：  
```plaintext
错误 LNK2038 检测到“_ITERATOR_DEBUG_LEVEL”的不匹配项: 值“0”不匹配值“2”(cmake_pch.obj 中)
```
### **出现原因:**  
- 项目生成模式（Debug/Release）与引用的 Bullet3 库编译模式不一致：  
- **Debug 模式**需链接 Debug 版本的库。  
- **Release 模式**需链接 Release 版本的库。  
### **解决方案:**  
- **打开项目属性**  
   - 在解决方案资源管理器中，右键项目 **CSC8503** → 选择 **属性**。  
- **配置模式一致性**  
   - 在属性页左上角，选择当前生成模式（如 `Debug` 或 `Release`）。  
   - 导航到 **配置属性 → vcpkg**。  
   - 确保右侧的 **Vcpkg Configuration** 值与当前配置模式一致：  
     - `Debug` 配置 → 选择 `Debug`  
     - `Release` 配置 → 选择 `Release`  
- **保存并重新编译**  
   - 点击 **确定** 保存配置。  
   - 清理解决方案（**生成 → 清理解决方案**），然后重新编译。  

## 2. .gitignore文件不生效，编译文件仍被提交到代码仓库
### **错误现象:**
- Github提交代码时Local Changes中包含.cmake或.vcxproj等项目编译文件
### **出现原因:**
- .gitignore文件更新后未生效，导致修改后应被忽略的文件仍被追踪修改记录
### **解决方案:**  
- **在命令行中打开项目文件夹**
   - 在Windows资源管理器中打开项目文件夹，右键点击空白处 → 选择 **显示更多选项** → 选择 **在终端中打开**。 
- **执行以下代码更新本地git记录缓存**
```cmd
git rm -r --cached .
git add .
git commit -m 'Update .gitignore'
git push
```
---
