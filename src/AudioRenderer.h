#ifndef AUDIO_RENDERER_H
#define AUDIO_RENDERER_H

#include "Song.h"
#include <vector>
#include <map>

// Forward declaration for TinySoundFont
struct tsf;

class AudioRenderer {
public:
    AudioRenderer();
    ~AudioRenderer();
    
    // Returns interleaved stereo samples (L, R, L, R...)
    std::vector<float> render(const Song& song, float sample_rate = 44100.0f);

private:
    std::map<std::string, tsf*> m_soundfonts;
};

#endif // AUDIO_RENDERER_H