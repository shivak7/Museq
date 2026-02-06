# Museq Build System Architecture Reference

This document serves as a "golden" reference for the Museq build system, which integrates a C++ CLI engine and a Dear ImGui-based Studio GUI (`muqomposer`). This configuration has been optimized to resolve cross-platform linker issues, specifically on macOS ARM64 (Apple Silicon).

## 1. Root CMake Configuration (`CMakeLists.txt`)

The root configuration unifies the project by creating a central `museq_engine` library. This prevents code duplication and ensures that all targets use identical audio logic.

```cmake
# Standard Project Setup
cmake_minimum_required(VERSION 3.10)
project(Museq CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- Core Engine Library ---
# All engine .cpp files are bundled here. 
# src/main.cpp is EXCLUDED because it is specific to the CLI.
add_library(museq_engine STATIC ${ENGINE_SOURCES})

# --- Dependency Discovery Strategy ---
# We use a 3-tier discovery strategy for SNDFILE, VORBIS, and LAME:
# 1. find_package(CONFIG) - Best for vcpkg/Windows.
# 2. pkg_check_modules(IMPORTED_TARGET) - Best for Linux and standard Homebrew.
# 3. find_path/find_library(HINTS) - Fallback for non-standard paths (e.g., /opt/homebrew).

# Example: LAME Discovery with Homebrew Hints
find_package(mp3lame CONFIG QUIET)
if(TARGET mp3lame::mp3lame)
    target_link_libraries(museq_engine PUBLIC mp3lame::mp3lame)
else()
    find_package(PkgConfig QUIET)
    pkg_check_modules(LAME IMPORTED_TARGET lame)
    if(LAME_FOUND)
        target_link_libraries(museq_engine PUBLIC PkgConfig::LAME)
    else()
        find_path(LAME_INC NAMES lame/lame.h lame.h HINTS /opt/homebrew/include /usr/local/include)
        find_library(LAME_LIB NAMES mp3lame libmp3lame lame HINTS /opt/homebrew/lib /usr/local/lib)
        if(LAME_INC AND LAME_LIB)
            target_include_directories(museq_engine PUBLIC ${LAME_INC})
            target_link_libraries(museq_engine PUBLIC ${LAME_LIB})
        endif()
    endif()
endif()

# --- CLI Target ---
add_executable(museq src/main.cpp)
target_link_libraries(museq PRIVATE museq_engine)

# --- Subdirectories ---
add_subdirectory(muqomposer)
```

## 2. GUI Build Configuration (`muqomposer/CMakeLists.txt`)

The GUI project is designed to be minimal. It **inherits** all audio dependencies from `museq_engine` via `PUBLIC` linking, which avoids "library not found" errors caused by redundant discovery.

```cmake
# Inherit Engine
add_executable(muqomposer main.cpp ${IMGUI_SOURCES})

# muqomposer ONLY needs to find graphics libraries
find_package(OpenGL REQUIRED)
# (GLFW discovery logic here...)

# Linking
target_link_libraries(muqomposer 
    PRIVATE
    museq_engine    # Inherits SndFile, Vorbis, LAME automatically
    ${GLFW_TARGET}
    OpenGL::GL
)
```

## 3. GitHub Actions CI Configuration (`.github/workflows/build.yml`)

The macOS ARM64 build requires explicit pathing to locate Homebrew-installed dependencies.

```yaml
  build-macos-arm64:
    runs-on: macos-latest
    steps:
    - name: Install dependencies
      run: brew install cmake pkg-config libsndfile libvorbis libogg lame flac opus mpg123 glfw

    - name: Configure CMake
      # CMAKE_PREFIX_PATH helps find_package/find_path
      # PKG_CONFIG_PATH helps pkg_check_modules
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/homebrew
      env:
        PKG_CONFIG_PATH: /opt/homebrew/lib/pkgconfig:/usr/local/lib/pkgconfig
```

## 4. Key Architectural Decisions

1.  **`museq_engine` as STATIC Library:** Prevents the same source files from being compiled twice. Audio dependencies are linked to this library as `PUBLIC` so that `museq` and `muqomposer` receive the full dependency graph automatically.
2.  **`IMPORTED_TARGET`:** Using `PkgConfig::TARGET` instead of variable names (`${LAME_LIBRARIES}`) is safer because it preserves absolute paths to `.dylib` or `.so` files, preventing the linker from searching in the wrong places.
3.  **Homebrew HINTS:** Explicitly adding `/opt/homebrew/lib` and `/opt/homebrew/include` to `find_` calls is the most reliable way to support Apple Silicon machines where these paths are not in the default compiler search path.
4.  **Consolidated Output:** `CMAKE_RUNTIME_OUTPUT_DIRECTORY` ensures all binaries end up in `build/bin/` regardless of where their `CMakeLists.txt` is located.
