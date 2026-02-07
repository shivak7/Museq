#ifndef MP3_WRITER_H
#define MP3_WRITER_H

#include "Song.h"
#include <string>

class AudioRenderer;

class Mp3Writer {
public:
    void write(AudioRenderer& renderer, const Song& song, const std::string& file_path, float sample_rate = 44100.0f, int bitrate = 192);
};

#endif // MP3_WRITER_H
