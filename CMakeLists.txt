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

# --- Dependency Discovery ---

# 1. sndfile
# vcpkg provides Config support
find_package(SndFile CONFIG QUIET)
if(TARGET SndFile::sndfile)
    message(STATUS "Found SndFile via Config")
    target_link_libraries(museq PRIVATE SndFile::sndfile)
else()
    find_package(PkgConfig QUIET)
    pkg_check_modules(PC_SNDFILE sndfile)
    find_path(SNDFILE_INCLUDE_DIR sndfile.h HINTS ${PC_SNDFILE_INCLUDEDIR} ${PC_SNDFILE_INCLUDE_DIRS})
    find_library(SNDFILE_LIBRARY NAMES sndfile libsndfile HINTS ${PC_SNDFILE_LIBDIR} ${PC_SNDFILE_LIBRARY_DIRS})
    
    if(SNDFILE_INCLUDE_DIR AND SNDFILE_LIBRARY)
        message(STATUS "Found sndfile manually: ${SNDFILE_LIBRARY}")
        target_include_directories(museq PRIVATE ${SNDFILE_INCLUDE_DIR})
        target_link_libraries(museq PRIVATE ${SNDFILE_LIBRARY})
    else()
        message(FATAL_ERROR "libsndfile not found. Install libsndfile1-dev or vcpkg install libsndfile.")
    endif()
endif()

# 2. Vorbis/Ogg
find_package(Vorbis CONFIG QUIET)
find_package(Ogg CONFIG QUIET)

if(TARGET Vorbis::vorbis AND TARGET Ogg::ogg)
    message(STATUS "Found Vorbis/Ogg via Config")
    target_link_libraries(museq PRIVATE Vorbis::vorbis Vorbis::vorbisenc Ogg::ogg)
else()
    find_package(PkgConfig QUIET)
    pkg_check_modules(PC_VORBIS vorbis)
    pkg_check_modules(PC_VORBISENC vorbisenc)
    pkg_check_modules(PC_OGG ogg)
    
    find_path(VORBIS_INC vorbis/codec.h HINTS ${PC_VORBIS_INCLUDEDIR} ${PC_VORBIS_INCLUDE_DIRS})
    find_path(VORBISENC_INC vorbis/vorbisenc.h HINTS ${PC_VORBISENC_INCLUDEDIR} ${PC_VORBISENC_INCLUDE_DIRS})
    find_path(OGG_INC ogg/ogg.h HINTS ${PC_OGG_INCLUDEDIR} ${PC_OGG_INCLUDE_DIRS})
    
    find_library(VORBIS_LIB NAMES vorbis libvorbis HINTS ${PC_VORBIS_LIBDIR} ${PC_VORBIS_LIBRARY_DIRS})
    find_library(VORBISENC_LIB NAMES vorbisenc libvorbisenc HINTS ${PC_VORBISENC_LIBDIR} ${PC_VORBISENC_LIBRARY_DIRS})
    find_library(OGG_LIB NAMES ogg libogg HINTS ${PC_OGG_LIBDIR} ${PC_OGG_LIBRARY_DIRS})
    
    if(VORBIS_LIB AND VORBISENC_LIB AND OGG_LIB)
        message(STATUS "Found Vorbis/Ogg manually")
        target_include_directories(museq PRIVATE ${VORBIS_INC} ${VORBISENC_INC} ${OGG_INC})
        target_link_libraries(museq PRIVATE ${VORBIS_LIB} ${VORBISENC_LIB} ${OGG_LIB})
    else()
        message(FATAL_ERROR "Vorbis/Ogg not found. Install libvorbis-dev libogg-dev or vcpkg install libvorbis libogg.")
    endif()
endif()

# 3. LAME
find_package(mp3lame CONFIG QUIET)
if(TARGET mp3lame::mp3lame)
    message(STATUS "Found mp3lame via Config")
    target_link_libraries(museq PRIVATE mp3lame::mp3lame)
else()
    find_package(PkgConfig QUIET)
    pkg_check_modules(PC_LAME lame)
    find_path(LAME_INC lame/lame.h HINTS ${PC_LAME_INCLUDEDIR} ${PC_LAME_INCLUDE_DIRS})
    # Note: added libmp3lame for Windows static libs
    find_library(LAME_LIB NAMES mp3lame libmp3lame lame HINTS ${PC_LAME_LIBDIR} ${PC_LAME_LIBRARY_DIRS})
    
    if(LAME_INC AND LAME_LIB)
        message(STATUS "Found LAME manually: ${LAME_LIB}")
        target_include_directories(museq PRIVATE ${LAME_INC})
        target_link_libraries(museq PRIVATE ${LAME_LIB})
    else()
        message(FATAL_ERROR "LAME not found. Install libmp3lame-dev or vcpkg install mp3lame.")
    endif()
endif()

if(WIN32)
    target_link_libraries(museq PRIVATE ws2_32 winmm shlwapi)
endif()