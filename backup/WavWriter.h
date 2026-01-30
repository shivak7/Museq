#ifndef WAV_WRITER_H
#define WAV_WRITER_H

#include "Song.h"
#include <string>

class AudioRenderer;

class WavWriter {
public:
    void write(AudioRenderer& renderer, const Song& song, const std::string& file_path, float sample_rate = 44100.0f);
};

#endif // WAV_WRITER_H
