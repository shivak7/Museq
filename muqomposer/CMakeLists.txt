# Muqomposer GUI Build Configuration
cmake_minimum_required(VERSION 3.10)
project(muqomposer)

set(CMAKE_CXX_STANDARD 17)

# --- ImGui Files ---
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# --- Include Directories ---
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../third_party")
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)

# --- Executable ---
add_executable(muqomposer main.cpp AssetManager.cpp ${IMGUI_SOURCES})

# --- Graphics Dependencies ---
find_package(OpenGL REQUIRED)

# 1. GLFW
find_package(glfw3 CONFIG QUIET)
if(NOT TARGET glfw)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW IMPORTED_TARGET glfw3)
    if(GLFW_FOUND)
        set(GLFW_TARGET PkgConfig::GLFW)
    else()
        find_path(GLFW_INC GLFW/glfw3.h HINTS /opt/homebrew/include /usr/local/include)
        find_library(GLFW_LIB NAMES glfw glfw3 HINTS /opt/homebrew/lib /usr/local/lib)
        if(GLFW_INC AND GLFW_LIB)
            target_include_directories(muqomposer PRIVATE ${GLFW_INC})
            set(GLFW_TARGET ${GLFW_LIB})
        endif()
    endif()
else()
    set(GLFW_TARGET glfw)
endif()

# --- Linking ---
# muqomposer inherits SndFile, Vorbis, and LAME from museq_engine PUBLIC links
target_link_libraries(muqomposer 
    PRIVATE
    museq_engine
    ${GLFW_TARGET}
    OpenGL::GL
)

if(WIN32)
    target_link_libraries(muqomposer PRIVATE ws2_32 winmm shlwapi)
endif()

if(UNIX)
    target_link_libraries(muqomposer PRIVATE pthread dl m)
endif()