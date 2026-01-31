cmake_minimum_required(VERSION 3.10)

project(Museq CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Add include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party")

# Add executable
add_executable(museq ${SOURCES})

# Dependency Discovery using find_path/find_library
# This works for both vcpkg (Windows) and system installs (Linux) if hints are correct.

find_package(PkgConfig)

# 1. sndfile
pkg_check_modules(PC_SNDFILE sndfile)
find_path(SNDFILE_INCLUDE_DIR sndfile.h HINTS ${PC_SNDFILE_INCLUDEDIR} ${PC_SNDFILE_INCLUDE_DIRS})
find_library(SNDFILE_LIBRARY NAMES sndfile libsndfile HINTS ${PC_SNDFILE_LIBDIR} ${PC_SNDFILE_LIBRARY_DIRS})

# 2. vorbis
pkg_check_modules(PC_VORBIS vorbis)
find_path(VORBIS_INCLUDE_DIR vorbis/codec.h HINTS ${PC_VORBIS_INCLUDEDIR} ${PC_VORBIS_INCLUDE_DIRS})
find_library(VORBIS_LIBRARY NAMES vorbis libvorbis HINTS ${PC_VORBIS_LIBDIR} ${PC_VORBIS_LIBRARY_DIRS})

# 3. vorbisenc
pkg_check_modules(PC_VORBISENC vorbisenc)
find_path(VORBISENC_INCLUDE_DIR vorbis/vorbisenc.h HINTS ${PC_VORBISENC_INCLUDEDIR} ${PC_VORBISENC_INCLUDE_DIRS})
find_library(VORBISENC_LIBRARY NAMES vorbisenc libvorbisenc HINTS ${PC_VORBISENC_LIBDIR} ${PC_VORBISENC_LIBRARY_DIRS})

# 4. ogg
pkg_check_modules(PC_OGG ogg)
find_path(OGG_INCLUDE_DIR ogg/ogg.h HINTS ${PC_OGG_INCLUDEDIR} ${PC_OGG_INCLUDE_DIRS})
find_library(OGG_LIBRARY NAMES ogg libogg HINTS ${PC_OGG_LIBDIR} ${PC_OGG_LIBRARY_DIRS})

# 5. lame
pkg_check_modules(PC_LAME lame) 
find_path(LAME_INCLUDE_DIR lame/lame.h HINTS ${PC_LAME_INCLUDEDIR} ${PC_LAME_INCLUDE_DIRS})
find_library(LAME_LIBRARY NAMES mp3lame lame HINTS ${PC_LAME_LIBDIR} ${PC_LAME_LIBRARY_DIRS})

# Linking
if (SNDFILE_INCLUDE_DIR AND SNDFILE_LIBRARY)
    message(STATUS "Found sndfile: ${SNDFILE_LIBRARY}")
    target_include_directories(museq PRIVATE ${SNDFILE_INCLUDE_DIR})
    target_link_libraries(museq PRIVATE ${SNDFILE_LIBRARY})
else()
    message(FATAL_ERROR "libsndfile not found. Please install libsndfile1-dev or vcpkg install libsndfile.")
endif()

if (VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY AND VORBISENC_LIBRARY AND OGG_LIBRARY)
    message(STATUS "Found Vorbis/Ogg")
    target_include_directories(museq PRIVATE ${VORBIS_INCLUDE_DIR} ${VORBISENC_INCLUDE_DIR} ${OGG_INCLUDE_DIR})
    target_link_libraries(museq PRIVATE ${VORBIS_LIBRARY} ${VORBISENC_LIBRARY} ${OGG_LIBRARY})
else()
    message(FATAL_ERROR "Vorbis/Ogg not found. Please install libvorbis-dev libogg-dev or vcpkg install libvorbis libogg.")
endif()

if (LAME_INCLUDE_DIR AND LAME_LIBRARY)
    message(STATUS "Found LAME: ${LAME_LIBRARY}")
    target_include_directories(museq PRIVATE ${LAME_INCLUDE_DIR})
    target_link_libraries(museq PRIVATE ${LAME_LIBRARY})
else()
    message(FATAL_ERROR "LAME not found. Please install libmp3lame-dev or vcpkg install mp3lame.")
endif()

# Windows system libs
if(WIN32)
    target_link_libraries(museq PRIVATE ws2_32 winmm shlwapi)
endif()
