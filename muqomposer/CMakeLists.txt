# GUI Prototype CMake Configuration
# This shows how to integrate Dear ImGui and GLFW with the Museq engine.

cmake_minimum_required(VERSION 3.10)
project(muqomposer)

set(CMAKE_CXX_STANDARD 17)

# --- Dependencies ---
# You would install GLFW via:
# Linux: sudo apt install libglfw3-dev
# macOS: brew install glfw
# Windows: vcpkg install glfw3:x64-windows

# --- ImGui Files ---
# ImGui is usually included as source. You would download it from
# https://github.com/ocornut/imgui and place the core files in third_party/imgui
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

# --- Museq Engine ---
# Include the existing source files from the main project
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../third_party")
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)

# --- Executable ---
add_executable(muqomposer main.cpp ${IMGUI_SOURCES})

find_package(OpenGL REQUIRED)

target_link_libraries(muqomposer 
    PRIVATE
    museq_engine
    glfw
    OpenGL::GL
    ${CMAKE_DL_LIBS} # for Linux
)

if(WIN32)
    target_link_libraries(muqomposer PRIVATE ws2_32 winmm shlwapi)
endif()

if(UNIX)
    target_link_libraries(muqomposer PRIVATE pthread dl m)
endif()

# --- Dependency Discovery ---

# 1. GLFW (Always needed for GUI)
find_package(glfw3 CONFIG QUIET)
if(TARGET glfw)
    target_link_libraries(muqomposer PRIVATE glfw)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED IMPORTED_TARGET glfw3)
    target_link_libraries(muqomposer PRIVATE PkgConfig::GLFW)
endif()

# 2. Audio Dependencies (Inherit from parent if possible)
# On Linux, they are often in SNDFILE_LIBRARIES, VORBIS_LIBRARIES, LAME_LIBRARIES
# On Windows, they are often in targets like SndFile::sndfile

# SndFile
if(TARGET SndFile::sndfile)
    target_link_libraries(muqomposer PRIVATE SndFile::sndfile)
elseif(SNDFILE_LIBRARIES)
    target_link_directories(muqomposer PRIVATE ${SNDFILE_LIBRARY_DIRS})
    target_link_libraries(muqomposer PRIVATE ${SNDFILE_LIBRARIES})
else()
    pkg_check_modules(SNDFILE REQUIRED IMPORTED_TARGET sndfile)
    target_link_libraries(muqomposer PRIVATE PkgConfig::SNDFILE)
endif()

# Vorbis/Ogg
if(TARGET Vorbis::vorbis AND TARGET Ogg::ogg)
    if(TARGET Vorbis::vorbisenc)
        target_link_libraries(muqomposer PRIVATE Vorbis::vorbis Vorbis::vorbisenc Ogg::ogg)
    else()
        target_link_libraries(muqomposer PRIVATE Vorbis::vorbis Ogg::ogg)
        find_library(VORBISENC_LIB NAMES vorbisenc libvorbisenc)
        if(VORBISENC_LIB)
            target_link_libraries(muqomposer PRIVATE ${VORBISENC_LIB})
        endif()
    endif()
elseif(VORBIS_LIBRARIES)
    target_link_directories(muqomposer PRIVATE ${VORBIS_LIBRARY_DIRS})
    target_link_libraries(muqomposer PRIVATE ${VORBIS_LIBRARIES})
else()
    pkg_check_modules(VORBIS REQUIRED IMPORTED_TARGET vorbis vorbisenc ogg)
    target_link_libraries(muqomposer PRIVATE PkgConfig::VORBIS)
endif()

# LAME
if(TARGET mp3lame::mp3lame)
    target_link_libraries(muqomposer PRIVATE mp3lame::mp3lame)
elseif(LAME_LIBRARIES)
    target_link_directories(muqomposer PRIVATE ${LAME_LIBRARY_DIRS})
    target_link_libraries(muqomposer PRIVATE ${LAME_LIBRARIES})
else()
    pkg_check_modules(LAME REQUIRED IMPORTED_TARGET lame)
    target_link_libraries(muqomposer PRIVATE PkgConfig::LAME)
endif()
