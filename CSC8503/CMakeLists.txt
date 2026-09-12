set(PROJECT_NAME CSC8503)

# FMOD Configuration
set(FMOD_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/FMod")
set(FMOD_INCLUDE_DIR "${FMOD_ROOT_DIR}/inc")
set(FMOD_LIBRARY_DIR "${FMOD_ROOT_DIR}/lib")

# Noesis Configuration
set(NOESIS_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/NoesisGUI")
message(STATUS "NOESIS_ROOT_DIR: ${NOESIS_ROOT_DIR}")
set(NOESIS_INCLUDE_DIR "${NOESIS_ROOT_DIR}/Include")
message(STATUS "NOESIS_INCLUDE_DIR: ${NOESIS_INCLUDE_DIR}")
set(NOESIS_LIBRARY_DIR "${NOESIS_ROOT_DIR}/Lib/windows_x86_64")
message(STATUS "NOESIS_LIBRARY_DIR: ${NOESIS_LIBRARY_DIR}")
set(NOESIS_BIN_DIR "${NOESIS_ROOT_DIR}/Bin/windows_x86_64")
message(STATUS "NOESIS_BIN_DIR: ${NOESIS_BIN_DIR}")


# 检查库文件是否存在
if(NOT EXISTS "${NOESIS_LIBRARY_DIR}/Noesis.lib")
    message(FATAL_ERROR "Noesis.lib not found at ${NOESIS_LIBRARY_DIR}/Noesis.lib")
endif()

# ImGui Configuration
set(IMGUI_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
if(NOT EXISTS "${IMGUI_ROOT_DIR}")
    message(FATAL_ERROR "ImGui directory not found at ${IMGUI_ROOT_DIR}")
endif()

# 添加 ImGui 包含目录
include_directories(${IMGUI_ROOT_DIR})
include_directories("${IMGUI_ROOT_DIR}/backends")

# Add FMOD include directory
include_directories(${FMOD_INCLUDE_DIR})
include_directories(${NOESIS_INCLUDE_DIR})

# Effekseer - 使用硬编码相对路径
set(EFFEKSEER_ROOT_DIR "${CMAKE_SOURCE_DIR}/EffekseerForCpp170e")
set(EFFEKSEER_RELEASE_DIR "${EFFEKSEER_ROOT_DIR}/install_msvc2022_x64_release")
set(EFFEKSEER_DEBUG_DIR "${EFFEKSEER_ROOT_DIR}/install_msvc2022_x64")

# 设置库文件路径
set(EFFEKSEER_RELEASE_LIBS
    "${EFFEKSEER_RELEASE_DIR}/lib/Effekseer.lib"
    "${EFFEKSEER_RELEASE_DIR}/lib/EffekseerRendererGL.lib"
    "${EFFEKSEER_RELEASE_DIR}/lib/EffekseerSoundDSound.lib"
)

set(EFFEKSEER_DEBUG_LIBS
    "${EFFEKSEER_DEBUG_DIR}/lib/Effekseer.lib"
    "${EFFEKSEER_DEBUG_DIR}/lib/EffekseerRendererGL.lib"
    "${EFFEKSEER_DEBUG_DIR}/lib/EffekseerSoundDSound.lib"
)

################################################################################
# Source groups
################################################################################
#set(Header_Files
#    "GameTechRenderer.h"
#    "NetworkedGame.h"
#    "NetworkPlayer.h"
#    "StateGameObject.h"
#    "TutorialGame.h"
#    "AudioSystem.h"
#    “GameMaterialLoad.h”
#    “ResourceManager.h”
#    “RenderPrepare.h”
#	 "UIManager.h"
#    "Effekseer.h"
#)
file(GLOB Header_Files *.h)
source_group("Header Files" FILES ${Header_Files})



#set(Source_Files
#    "GameTechRenderer.cpp"
#    "Main.cpp"
#    "NetworkedGame.cpp"
#    "NetworkPlayer.cpp"
#    "StateGameObject.cpp"
#    "TutorialGame.cpp"
#    "AudioSystem.cpp"
#    “GameMaterialLoad.cpp”
#    “ResourceManager.cpp”
#    “RenderPrepare.cpp”
#    "UIManager.cpp"
#    "Effekseer.cpp"
#    "FlowField.cpp"
#    "InputProcess.cpp"
#)
file(GLOB Source_Files *.cpp)



# 添加 Shaders 目录下的所有着色器文件
file(GLOB Shader_Files ${CMAKE_SOURCE_DIR}/NewOpenGL/Shaders/*.*)

# 让着色器文件成为 CMake 目标的一部分（不会编译，但能在 IDE 中可见）
source_group("Shaders" FILES ${Shader_Files})

# 添加着色器文件到 CSC8503 的文件列表中
set(ALL_FILES ${ALL_FILES} ${Shader_Files})


file(GLOB OpenGL_Files ${CMAKE_SOURCE_DIR}/NewOpenGL/*.*)

source_group("OpenGL" FILES ${OpenGL_Files})


file(GLOB IMGUI_FILES 
    "${IMGUI_ROOT_DIR}/*.cpp"
    "${IMGUI_ROOT_DIR}/*.h"
    "${IMGUI_ROOT_DIR}/backends/imgui_impl_opengl3.cpp"
    "${IMGUI_ROOT_DIR}/backends/imgui_impl_opengl3.h"
    "${IMGUI_ROOT_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_ROOT_DIR}/backends/imgui_impl_glfw.h"
)

# 创建源文件组
source_group("ImGui" FILES ${IMGUI_FILES})


if(USE_VULKAN)
	file(GLOB SHADER_FILES ${ASSET_ROOT}/Shaders/VK/*.*)
else()
	list(REMOVE_ITEM Header_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechVulkanRenderer.h)
	list(REMOVE_ITEM Source_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechVulkanRenderer.cpp)
endif()

source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    ${Shader_Files}
    ${OpenGL_Files}
    ${IMGUI_FILES}
)

if(USE_VULKAN)
    set(ALL_FILES 
        ${Header_Files}
        ${Source_Files}
        ${SHADER_FILES}
    )

foreach (file ${SHADER_FILES})
    get_filename_component(file_name ${file} NAME)
	get_filename_component(file_ext ${file} EXT)
	
	if(file_ext STREQUAL ".h" OR file_ext STREQUAL ".cpp")
		continue()
	endif()
		
	if( file_ext STREQUAL  ".vert" OR
		file_ext STREQUAL  ".frag" OR
		file_ext STREQUAL  ".comp" OR
		file_ext STREQUAL  ".geom" OR
		file_ext STREQUAL  ".tesc" OR
		file_ext STREQUAL  ".tese" OR
		file_ext STREQUAL  ".rgen" OR
		file_ext STREQUAL  ".rint" OR
		file_ext STREQUAL  ".rahit" OR
		file_ext STREQUAL  ".rchit" OR
		file_ext STREQUAL  ".rmiss" OR
		file_ext STREQUAL  ".rcall" OR
		file_ext STREQUAL  ".task" OR
		file_ext STREQUAL  ".mesh"
	)
		message("Adding custom command to ${file}")
		get_filename_component(file_dir ${file} ABSOLUTE)
        set(SPIRV_OUTPUT ${file_name}.spv)
		set(SPIRV_ABS_INPUT ${file_dir})
		set(SPIRV_ABS_OUTPUT ${file_dir}.spv)
		
		add_custom_command(
			OUTPUT ${SPIRV_ABS_OUTPUT}
			
			COMMENT "Compiling GLSL shader:"
			COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V  ${SPIRV_ABS_INPUT} -o ${SPIRV_ABS_OUTPUT}
            DEPENDS ${file}
			VERBATIM
		)
        list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUTPUT})   
	endif()
endforeach()

endif()

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME}  ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE CSC8503)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

################################################################################
# Compile definitions
################################################################################
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "UNICODE;"
        "_UNICODE" 
        "WIN32_LEAN_AND_MEAN"
        "_WINSOCKAPI_"   
        "_WINSOCK2API_"
        "_WINSOCK_DEPRECATED_NO_WARNINGS"
    )
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vector>
    <map>
    <stack>
    <list>   
	<set>   
	<string>
    <thread>
    <atomic>
    <functional>
    <iostream>
	<chrono>
	<sstream>
	
	"../NCLCoreClasses/Vector.h"
    "../NCLCoreClasses/Quaternion.h"
    "../NCLCoreClasses/Plane.h"
    "../NCLCoreClasses/Matrix.h"
    "../NCLCoreClasses/GameTimer.h"
)
 
if(USE_VULKAN)
target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vulkan/vulkan.hpp>
    <SmartTypes.h>
    <VulkanRenderer.h>
    <VulkanShader.h>
    <VulkanShaderBuilder.h>
    <VulkanTexture.h>
    <VulkanMesh.h>
    <VulkanPipelineBuilder.h>
    <VulkanDynamicRenderBuilder.h>

    <VulkanTextureBuilder.h>

    <VulkanDescriptorSetLayoutBuilder.h>
    <VulkanRenderPassBuilder.h>
    <VulkanCompute.h>
    <VulkanComputePipelineBuilder.h>
	<VulkanBufferBuilder.h>
)
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        /permissive-;
        /std:c++latest;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
    )
endif()

################################################################################
# Dependencies
################################################################################
if(MSVC)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "Winmm.lib")
endif()

include_directories("../OpenGLRendering/")
include_directories("../NCLCoreClasses/")
include_directories("../CSC8503CoreClasses/")

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503CoreClasses)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC OpenGLRendering)

if(USE_VULKAN)
    include_directories("../VulkanRendering/")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC VulkanRendering)

    add_compile_definitions(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
    add_compile_definitions(VK_NO_PROTOTYPES)

    add_custom_target(
        SHADER_FILES
        DEPENDS ON ${SPIRV_BINARY_FILES}
    )

    add_dependencies(${PROJECT_NAME} SHADER_FILES)
endif()

# FMOD Libraries
if(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${FMOD_LIBRARY_DIR}/fmod_vc.lib"
        "${FMOD_LIBRARY_DIR}/fmodstudio_vc.lib"
    )

    # Copy FMOD DLLs
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FMOD_LIBRARY_DIR}/fmod.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FMOD_LIBRARY_DIR}/fmodstudio.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

     # 先找到库文件
    find_library(NOESIS_LIB Noesis PATHS ${NOESIS_LIBRARY_DIR} NO_DEFAULT_PATH)
    find_library(NOESIS_APP_LIB NoesisApp PATHS ${NOESIS_LIBRARY_DIR} NO_DEFAULT_PATH)

    # 输出调试信息
    message(STATUS "NOESIS_LIB: ${NOESIS_LIB}")
    message(STATUS "NOESIS_APP_LIB: ${NOESIS_APP_LIB}")

     # 检查是否找到
    if(NOT NOESIS_LIB OR NOT NOESIS_APP_LIB)
        message(FATAL_ERROR "Noesis libraries not found!")
    endif()

    # Noesis Libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${NOESIS_LIBRARY_DIR}/Noesis.lib"
        "${NOESIS_LIBRARY_DIR}/NoesisApp.lib"
    )

    # Copy Noesis DLLs
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${NOESIS_BIN_DIR}/Noesis.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${NOESIS_BIN_DIR}/NoesisApp.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # Add Noesis compile definitions
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        NS_STATIC_LIBRARY
        NS_APP_STATIC_LIBRARY
        $<$<CONFIG:Debug>:NS_DEBUG>
    )
endif()

