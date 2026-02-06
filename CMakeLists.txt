cmake_minimum_required(VERSION 3.10)

project(Museq CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files explicitly to avoid GLOB issues on Windows
set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AdsrEnvelope.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AudioPlayer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AudioRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AudioUtils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Instrument.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/JsonSerializer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Mp3Writer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Note.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NoteParser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OggWriter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Sampler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ScriptParser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Sequence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Voice.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/WavWriter.cpp"
)

# Windows Resource File (Icon)
if(WIN32)
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/museq.rc")
endif()

# Add include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniaudio")

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
    pkg_check_modules(SNDFILE sndfile)
    if(SNDFILE_FOUND)
        message(STATUS "Found sndfile via PkgConfig")
        target_include_directories(museq PRIVATE ${SNDFILE_INCLUDE_DIRS})
        target_link_directories(museq PRIVATE ${SNDFILE_LIBRARY_DIRS})
        target_link_libraries(museq PRIVATE ${SNDFILE_LIBRARIES})
    else()
        find_path(SNDFILE_INCLUDE_DIR sndfile.h)
        find_library(SNDFILE_LIBRARY NAMES sndfile libsndfile)
        if(SNDFILE_INCLUDE_DIR AND SNDFILE_LIBRARY)
            set(SNDFILE_LIBRARIES ${SNDFILE_LIBRARY})
            target_include_directories(museq PRIVATE ${SNDFILE_INCLUDE_DIR})
            target_link_libraries(museq PRIVATE ${SNDFILE_LIBRARIES})
        else()
            message(FATAL_ERROR "libsndfile not found. Install libsndfile1-dev or vcpkg install libsndfile.")
        endif()
    endif()
endif()

# 2. Vorbis/Ogg
if(WIN32)
    find_package(Vorbis CONFIG QUIET)
    find_package(Ogg CONFIG QUIET)
endif()

if(TARGET Vorbis::vorbis AND TARGET Ogg::ogg)
    message(STATUS "Found Vorbis/Ogg via Config")
    target_link_libraries(museq PRIVATE Vorbis::vorbis Vorbis::vorbisenc Ogg::ogg)
else()
    find_package(PkgConfig QUIET)
    pkg_check_modules(VORBIS vorbis vorbisenc ogg)
    if(VORBIS_FOUND)
        message(STATUS "Found Vorbis/Ogg via PkgConfig")
        target_include_directories(museq PRIVATE ${VORBIS_INCLUDE_DIRS})
        target_link_directories(museq PRIVATE ${VORBIS_LIBRARY_DIRS})
        target_link_libraries(museq PRIVATE ${VORBIS_LIBRARIES})
    else()
        find_path(VORBIS_INC vorbis/codec.h)
        find_path(OGG_INC ogg/ogg.h)
        find_library(VORBIS_LIB NAMES vorbis libvorbis)
        find_library(VORBISENC_LIB NAMES vorbisenc libvorbisenc)
        find_library(OGG_LIB NAMES ogg libogg)
        if(VORBIS_LIB AND VORBISENC_LIB AND OGG_LIB)
            set(VORBIS_LIBRARIES ${VORBIS_LIB} ${VORBISENC_LIB} ${OGG_LIB})
            target_include_directories(museq PRIVATE ${VORBIS_INC} ${OGG_INC})
            target_link_libraries(museq PRIVATE ${VORBIS_LIBRARIES})
        else()
            message(FATAL_ERROR "Vorbis/Ogg not found. Install libvorbis-dev libogg-dev or vcpkg install libvorbis libogg.")
        endif()
    endif()
endif()

# 3. LAME
find_package(mp3lame CONFIG QUIET)
if(TARGET mp3lame::mp3lame)
    message(STATUS "Found mp3lame via Config")
    target_link_libraries(museq PRIVATE mp3lame::mp3lame)
else()
    find_package(PkgConfig QUIET)
    pkg_check_modules(LAME lame)
    if(LAME_FOUND)
        message(STATUS "Found LAME via PkgConfig")
        target_include_directories(museq PRIVATE ${LAME_INCLUDE_DIRS})
        target_link_directories(museq PRIVATE ${LAME_LIBRARY_DIRS})
        target_link_libraries(museq PRIVATE ${LAME_LIBRARIES})
    else()
        find_path(LAME_INC lame/lame.h)
        find_library(LAME_LIB NAMES mp3lame libmp3lame lame)
        if(LAME_INC AND LAME_LIB)
            set(LAME_LIBRARIES ${LAME_LIB})
            target_include_directories(museq PRIVATE ${LAME_INC})
            target_link_libraries(museq PRIVATE ${LAME_LIBRARIES})
        else()
            message(FATAL_ERROR "LAME not found. Install libmp3lame-dev or vcpkg install mp3lame.")
        endif()
    endif()
endif()

if(WIN32)
    target_link_libraries(museq PRIVATE ws2_32 winmm shlwapi)
endif()

if(UNIX)
    target_link_libraries(museq PRIVATE pthread dl m)
endif()

# --- GUI Application ---
add_subdirectory(muqomposer)
