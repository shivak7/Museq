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

# Set search paths if vcpkg triplet is set
if(VCPKG_TARGET_TRIPLET)
    set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}")
    message(STATUS "Using VCPKG_ROOT: ${VCPKG_ROOT}")
    
    # Brute force for Windows cross-compilation
    set(SNDFILE_INC "${VCPKG_ROOT}/include")
    set(SNDFILE_LIB "${VCPKG_ROOT}/lib/libsndfile.a")
    
    set(LAME_INC "${VCPKG_ROOT}/include")
    set(LAME_LIB "${VCPKG_ROOT}/lib/libmp3lame.a")
    
    set(VORBIS_INC "${VCPKG_ROOT}/include")
    set(VORBIS_LIB "${VCPKG_ROOT}/lib/libvorbis.a")
    
    set(VORBISENC_INC "${VCPKG_ROOT}/include")
    set(VORBISENC_LIB "${VCPKG_ROOT}/lib/libvorbisenc.a")
    
    set(OGG_INC "${VCPKG_ROOT}/include")
    set(OGG_LIB "${VCPKG_ROOT}/lib/libogg.a")
    
    set(FLAC_LIB "${VCPKG_ROOT}/lib/libFLAC.a")
    set(OPUS_LIB "${VCPKG_ROOT}/lib/libopus.a")
    set(MPG123_LIB "${VCPKG_ROOT}/lib/libmpg123.a")

    target_include_directories(museq PRIVATE ${SNDFILE_INC} ${LAME_INC} ${VORBIS_INC} ${VORBISENC_INC} ${OGG_INC})
    target_link_libraries(museq PRIVATE 
        ${SNDFILE_LIB} ${LAME_LIB} ${VORBIS_LIB} ${VORBISENC_LIB} ${OGG_LIB}
        ${FLAC_LIB} ${OPUS_LIB} ${MPG123_LIB}
        ws2_32 winmm shlwapi
    )
else()
    # Linux (using PkgConfig or find_library as before)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SNDFILE REQUIRED sndfile)
    pkg_check_modules(VORBIS REQUIRED vorbis)
    pkg_check_modules(VORBISENC REQUIRED vorbisenc)
    pkg_check_modules(OGG REQUIRED ogg)
    find_path(LAME_INC NAMES lame/lame.h REQUIRED)
    find_library(LAME_LIB NAMES mp3lame lame REQUIRED)

    target_include_directories(museq PRIVATE 
        ${SNDFILE_INCLUDE_DIRS} ${VORBIS_INCLUDE_DIRS} ${VORBISENC_INCLUDE_DIRS} ${OGG_INCLUDE_DIRS} ${LAME_INC}
    )
    target_link_libraries(museq PRIVATE
        ${SNDFILE_LIBRARIES} ${VORBIS_LIBRARIES} ${VORBISENC_LIBRARIES} ${OGG_LIBRARIES} ${LAME_LIB}
    )
endif()