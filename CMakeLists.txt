cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(CSC8503 CXX)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
#set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 23)

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


if(USE_VULKAN)
    find_package(Vulkan)
endif() 
set(ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets/" CACHE STRING "" FORCE)
add_compile_definitions(ASSETROOTLOCATION="${ASSET_ROOT}") 

set(USE_VULKAN CACHE BOOL FORCE)
if(USE_VULKAN)
    add_compile_definitions("USEVULKAN") 
    add_compile_definitions("VK_USE_PLATFORM_WIN32_KHR") 
endif() 

if(MSVC)
    add_compile_definitions("NOMINMAX")
    add_compile_definitions("WIN32_LEAN_AND_MEAN")  
endif()

################################################################################
# Effekseer Configuration
################################################################################
# 设置Effekseer库路径
set(EFFEKSEER_ROOT "${CMAKE_SOURCE_DIR}/EffekseerForCpp170e")
set(EFFEKSEER_DEBUG_DIR "${EFFEKSEER_ROOT}/install_msvc2022_x64")
set(EFFEKSEER_RELEASE_DIR "${EFFEKSEER_ROOT}/install_msvc2022_x64_release")

# 设置库文件列表
set(EFFEKSEER_DEBUG_LIBS
    "${EFFEKSEER_DEBUG_DIR}/lib/Effekseer.lib"
    "${EFFEKSEER_DEBUG_DIR}/lib/EffekseerRendererGL.lib"
    "${EFFEKSEER_DEBUG_DIR}/lib/EffekseerSoundDSound.lib"
)

set(EFFEKSEER_RELEASE_LIBS
    "${EFFEKSEER_RELEASE_DIR}/lib/Effekseer.lib"
    "${EFFEKSEER_RELEASE_DIR}/lib/EffekseerRendererGL.lib"
    "${EFFEKSEER_RELEASE_DIR}/lib/EffekseerSoundDSound.lib"
)

################################################################################
# Sub-projects (Modules) - 只添加一次子目录
################################################################################
add_subdirectory(NCLCoreClasses)
add_subdirectory(CSC8503CoreClasses)
add_subdirectory(OpenGLRendering)
add_subdirectory(CSC8503)

# Vulkan support
if(USE_VULKAN)
    add_subdirectory(VulkanRendering)
endif()

# 添加NewOpenGL包含路径
target_include_directories(CSC8503 PUBLIC ${CMAKE_SOURCE_DIR}/NewOpenGL)
target_include_directories(CSC8503CoreClasses PUBLIC ${CMAKE_SOURCE_DIR}/NewOpenGL)
target_include_directories(NCLCoreClasses PUBLIC ${CMAKE_SOURCE_DIR}/NewOpenGL)

# 添加Effekseer包含路径
target_include_directories(CSC8503 PUBLIC 
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_DIR}/include>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_DIR}/include>
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_DIR}/include/Effekseer>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_DIR}/include/Effekseer>
)
target_include_directories(CSC8503CoreClasses PUBLIC 
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_DIR}/include>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_DIR}/include>
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_DIR}/include/Effekseer>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_DIR}/include/Effekseer>
)
target_include_directories(NCLCoreClasses PUBLIC 
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_DIR}/include>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_DIR}/include>
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_DIR}/include/Effekseer>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_DIR}/include/Effekseer>
)
target_include_directories(OpenGLRendering PUBLIC 
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_DIR}/include>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_DIR}/include>
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_DIR}/include/Effekseer>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_DIR}/include/Effekseer>
)

# 设置库链接
target_link_libraries(CSC8503 PUBLIC 
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_LIBS}>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_LIBS}>
)
target_link_libraries(CSC8503CoreClasses PUBLIC 
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_LIBS}>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_LIBS}>
)
target_link_libraries(NCLCoreClasses PUBLIC 
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_LIBS}>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_LIBS}>
)
target_link_libraries(OpenGLRendering PUBLIC 
    $<$<CONFIG:Debug>:${EFFEKSEER_DEBUG_LIBS}>
    $<$<NOT:$<CONFIG:Debug>>:${EFFEKSEER_RELEASE_LIBS}>
)

# NewOpenGL头文件
file(GLOB_RECURSE NewOpenGL_Header_Files "${CMAKE_SOURCE_DIR}/NewOpenGL/*.h")
source_group("Header Files\\NewOpenGL" FILES ${NewOpenGL_Header_Files})
set(ALL_FILES ${ALL_FILES} ${NewOpenGL_Header_Files})

################################################################################
# Set startup project
################################################################################
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CSC8503)