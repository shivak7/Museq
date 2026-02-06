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

find_package(glfw3 REQUIRED)

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

set(MUSEQ_ENGINE_SOURCES
    ../src/AudioRenderer.cpp
    ../src/AudioPlayer.cpp
    ../src/Instrument.cpp
    ../src/Note.cpp
    ../src/NoteParser.cpp
    ../src/ScriptParser.cpp
    ../src/Sequence.cpp
    ../src/WavWriter.cpp
    ../src/AdsrEnvelope.cpp
    ../src/JsonSerializer.cpp
    ../src/Voice.cpp
    ../src/AudioUtils.cpp
    ../src/Mp3Writer.cpp
    ../src/OggWriter.cpp
    ../src/Sampler.cpp
)

# --- Executable ---
add_executable(muqomposer 
    main.cpp
    ${IMGUI_SOURCES}
    ${MUSEQ_ENGINE_SOURCES}
)

target_link_libraries(muqomposer 
    glfw
    GL
    ${CMAKE_DL_LIBS} # for Linux
)

if(UNIX)
    target_link_libraries(muqomposer pthread dl m)
endif()

# --- Dependency Discovery (Synchronized with root CMakeLists.txt) ---

find_package(PkgConfig REQUIRED)

# 1. sndfile
pkg_check_modules(SNDFILE REQUIRED sndfile)
target_include_directories(muqomposer PRIVATE ${SNDFILE_INCLUDE_DIRS})
target_link_libraries(muqomposer ${SNDFILE_LIBRARIES})

# 2. Vorbis/Ogg
pkg_check_modules(VORBIS REQUIRED vorbis vorbisenc ogg)
target_include_directories(muqomposer PRIVATE ${VORBIS_INCLUDE_DIRS})
target_link_libraries(muqomposer ${VORBIS_LIBRARIES})

# 3. LAME
pkg_check_modules(LAME REQUIRED lame)
target_include_directories(muqomposer PRIVATE ${LAME_INCLUDE_DIRS})
target_link_libraries(muqomposer ${LAME_LIBRARIES})
