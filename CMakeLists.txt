cmake_minimum_required(VERSION 3.10)

project(Museq CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output all executables to a 'bin' subdirectory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- Core Engine Library ---

# Define source files for the library (everything except main.cpp)
set(ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AdsrEnvelope.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AudioPlayer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AudioRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AudioUtils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Instrument.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/JsonSerializer.cpp"
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

# Add include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniaudio")

# Create the library target
add_library(museq_engine STATIC ${ENGINE_SOURCES})

# --- Dependency Discovery & Linking ---

# 1. sndfile
find_package(SndFile CONFIG QUIET)
if(TARGET SndFile::sndfile)
    message(STATUS "Found SndFile via Config")
    target_link_libraries(museq_engine PUBLIC SndFile::sndfile)
else()
    find_package(PkgConfig QUIET)
    pkg_check_modules(SNDFILE IMPORTED_TARGET sndfile)
    if(SNDFILE_FOUND)
        message(STATUS "Found sndfile via PkgConfig")
        target_link_libraries(museq_engine PUBLIC PkgConfig::SNDFILE)
    else()
        find_path(SNDFILE_INCLUDE_DIR sndfile.h HINTS /opt/homebrew/include /usr/local/include)
        find_library(SNDFILE_LIBRARY NAMES sndfile libsndfile HINTS /opt/homebrew/lib /usr/local/lib)
        if(SNDFILE_INCLUDE_DIR AND SNDFILE_LIBRARY)
            target_include_directories(museq_engine PUBLIC ${SNDFILE_INCLUDE_DIR})
            target_link_libraries(museq_engine PUBLIC ${SNDFILE_LIBRARY})
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
    target_link_libraries(museq_engine PUBLIC Vorbis::vorbis Vorbis::vorbisenc Ogg::ogg)
else()
    find_package(PkgConfig QUIET)
    pkg_check_modules(VORBIS IMPORTED_TARGET vorbis vorbisenc ogg)
    if(VORBIS_FOUND)
        message(STATUS "Found Vorbis/Ogg via PkgConfig")
        target_link_libraries(museq_engine PUBLIC PkgConfig::VORBIS)
    else()
        find_path(VORBIS_INC vorbis/codec.h HINTS /opt/homebrew/include /usr/local/include)
        find_path(OGG_INC ogg/ogg.h HINTS /opt/homebrew/include /usr/local/include)
        find_library(VORBIS_LIB NAMES vorbis libvorbis HINTS /opt/homebrew/lib /usr/local/lib)
        find_library(VORBISENC_LIB NAMES vorbisenc libvorbisenc HINTS /opt/homebrew/lib /usr/local/lib)
        find_library(OGG_LIB NAMES ogg libogg HINTS /opt/homebrew/lib /usr/local/lib)
        if(VORBIS_LIB AND VORBISENC_LIB AND OGG_LIB)
            target_include_directories(museq_engine PUBLIC ${VORBIS_INC} ${OGG_INC})
            target_link_libraries(museq_engine PUBLIC ${VORBIS_LIB} ${VORBISENC_LIB} ${OGG_LIB})
        else()
            message(FATAL_ERROR "Vorbis/Ogg not found. Install libvorbis-dev libogg-dev or vcpkg install libvorbis libogg.")
        endif()
    endif()
endif()

# 3. LAME
find_package(mp3lame CONFIG QUIET)
if(TARGET mp3lame::mp3lame)
    message(STATUS "Found mp3lame via Config")
    target_link_libraries(museq_engine PUBLIC mp3lame::mp3lame)
else()
    find_package(PkgConfig QUIET)
    pkg_check_modules(LAME IMPORTED_TARGET lame)
    if(LAME_FOUND)
        message(STATUS "Found LAME via PkgConfig")
        target_link_libraries(museq_engine PUBLIC PkgConfig::LAME)
    else()
        find_path(LAME_INC NAMES lame/lame.h lame.h HINTS /opt/homebrew/include /usr/local/include)
        find_library(LAME_LIB NAMES mp3lame libmp3lame lame HINTS /opt/homebrew/lib /usr/local/lib)
        if(LAME_INC AND LAME_LIB)
            target_include_directories(museq_engine PUBLIC ${LAME_INC})
            target_link_libraries(museq_engine PUBLIC ${LAME_LIB})
        else()
            message(FATAL_ERROR "LAME not found. Install libmp3lame-dev or vcpkg install mp3lame.")
        endif()
    endif()
endif()

if(WIN32)
    target_link_libraries(museq_engine PUBLIC ws2_32 winmm shlwapi)
endif()

if(UNIX)
    target_link_libraries(museq_engine PUBLIC pthread dl m)
endif()

# --- CLI Application ---

# Windows Resource File (Icon)
set(MUSEQ_CLI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
if(WIN32)
    list(APPEND MUSEQ_CLI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/museq.rc")
endif()

add_executable(museq ${MUSEQ_CLI_SOURCES})
target_link_libraries(museq PRIVATE museq_engine)

# --- GUI Application ---
add_subdirectory(muqomposer)