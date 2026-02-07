#ifndef OGG_WRITER_H
#define OGG_WRITER_H

#include "Song.h"
#include <string>

class AudioRenderer;

class OggWriter {
public:
    void write(AudioRenderer& renderer, const Song& song, const std::string& file_path, float sample_rate = 44100.0f, float quality = 0.4f);
};

#endif // OGG_WRITER_H
