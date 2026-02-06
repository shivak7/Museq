# GUI Prototype CMake Configuration
# This shows how to integrate Dear ImGui and GLFW with the Museq engine.

cmake_minimum_required(VERSION 3.10)
project(museq-gui)

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
add_executable(museq-gui 
    main.cpp
    ${IMGUI_SOURCES}
    ${MUSEQ_ENGINE_SOURCES}
)

target_link_libraries(museq-gui 
    glfw
    ${CMAKE_DL_LIBS} # for Linux
)

# Link audio dependencies (similar to main CMakeLists.txt)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)
target_link_libraries(museq-gui ${SNDFILE_LIBRARIES})
