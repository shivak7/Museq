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
    pkg_check_modules(GLFW IMPORTED_TARGET glfw3)
    if(GLFW_FOUND)
        target_link_libraries(muqomposer PRIVATE PkgConfig::GLFW)
    else()
        find_path(GLFW_INC GLFW/glfw3.h HINTS /opt/homebrew/include /usr/local/include)
        find_library(GLFW_LIB NAMES glfw glfw3 HINTS /opt/homebrew/lib /usr/local/lib)
        if(GLFW_INC AND GLFW_LIB)
            target_include_directories(muqomposer PRIVATE ${GLFW_INC})
            target_link_libraries(muqomposer PRIVATE ${GLFW_LIB})
        endif()
    endif()
endif()

# 2. Audio Dependencies (Inherit from parent if possible)
# On Linux, they are often in SNDFILE_LIBRARIES, VORBIS_LIBRARIES, LAME_LIBRARIES
# On Windows, they are often in targets like SndFile::sndfile

# SndFile
if(TARGET SndFile::sndfile)
    target_link_libraries(muqomposer PRIVATE SndFile::sndfile)
elseif(SNDFILE_LIBRARIES)
    target_link_libraries(muqomposer PRIVATE ${SNDFILE_LIBRARIES})
else()
    pkg_check_modules(SNDFILE IMPORTED_TARGET sndfile)
    if(SNDFILE_FOUND)
        target_link_libraries(muqomposer PRIVATE PkgConfig::SNDFILE)
    else()
        find_path(SNDFILE_INC sndfile.h HINTS /opt/homebrew/include /usr/local/include)
        find_library(SNDFILE_LIB NAMES sndfile libsndfile HINTS /opt/homebrew/lib /usr/local/lib)
        if(SNDFILE_INC AND SNDFILE_LIB)
            target_include_directories(muqomposer PRIVATE ${SNDFILE_INC})
            target_link_libraries(muqomposer PRIVATE ${SNDFILE_LIB})
        endif()
    endif()
endif()

# Vorbis/Ogg
if(TARGET Vorbis::vorbis AND TARGET Ogg::ogg)
    if(TARGET Vorbis::vorbisenc)
        target_link_libraries(muqomposer PRIVATE Vorbis::vorbis Vorbis::vorbisenc Ogg::ogg)
    else()
        target_link_libraries(muqomposer PRIVATE Vorbis::vorbis Ogg::ogg)
        find_library(VORBISENC_LIB NAMES vorbisenc libvorbisenc HINTS /opt/homebrew/lib /usr/local/lib)
        if(VORBISENC_LIB)
            target_link_libraries(muqomposer PRIVATE ${VORBISENC_LIB})
        endif()
    endif()
elseif(VORBIS_LIBRARIES)
    target_link_libraries(muqomposer PRIVATE ${VORBIS_LIBRARIES})
else()
    pkg_check_modules(VORBIS IMPORTED_TARGET vorbis vorbisenc ogg)
    if(VORBIS_FOUND)
        target_link_libraries(muqomposer PRIVATE PkgConfig::VORBIS)
    else()
        find_path(VORBIS_INC vorbis/codec.h HINTS /opt/homebrew/include /usr/local/include)
        find_path(OGG_INC ogg/ogg.h HINTS /opt/homebrew/include /usr/local/include)
        find_library(VORBIS_LIB NAMES vorbis libvorbis HINTS /opt/homebrew/lib /usr/local/lib)
        find_library(VORBISENC_LIB NAMES vorbisenc libvorbisenc HINTS /opt/homebrew/lib /usr/local/lib)
        find_library(OGG_LIB NAMES ogg libogg HINTS /opt/homebrew/lib /usr/local/lib)
        if(VORBIS_LIB AND OGG_LIB)
            target_include_directories(muqomposer PRIVATE ${VORBIS_INC} ${OGG_INC})
            target_link_libraries(muqomposer PRIVATE ${VORBIS_LIB} ${VORBISENC_LIB} ${OGG_LIB})
        endif()
    endif()
endif()

# LAME
if(TARGET mp3lame::mp3lame)
    target_link_libraries(muqomposer PRIVATE mp3lame::mp3lame)
elseif(LAME_LIBRARIES)
    target_link_libraries(muqomposer PRIVATE ${LAME_LIBRARIES})
else()
    pkg_check_modules(LAME IMPORTED_TARGET lame)
    if(LAME_FOUND)
        target_link_libraries(muqomposer PRIVATE PkgConfig::LAME)
    else()
        find_path(LAME_INC NAMES lame/lame.h lame.h HINTS /opt/homebrew/include /usr/local/include)
        find_library(LAME_LIB NAMES mp3lame libmp3lame lame HINTS /opt/homebrew/lib /usr/local/lib)
        if(LAME_INC AND LAME_LIB)
            target_include_directories(muqomposer PRIVATE ${LAME_INC})
            target_link_libraries(muqomposer PRIVATE ${LAME_LIB})
        endif()
    endif()
endif()
