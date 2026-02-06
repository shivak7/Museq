#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <vector>
#include "Song.h"
#include "AudioRenderer.h"

// Forward declaration for miniaudio device
struct ma_device;

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    // Initialize the audio device
    bool init();

    // Start playing a song
    void play(const Song& song);

    // Stop playback
    void stop();

    // Check if currently playing
    bool is_playing() const;

    // Visualization
    static const int VIS_BUFFER_SIZE = 1024;
    void get_visualization_data(float* out_buffer, int count);

private:
    // Miniaudio device handle (void* to avoid exposing miniaudio.h in header)
    void* m_device = nullptr;
    
    // Playback state
    bool m_playing = false;
    AudioRenderer m_renderer;

    // Visualization Ring Buffer
    float m_vis_buffer[VIS_BUFFER_SIZE] = {0};
    int m_vis_write_idx = 0;
    
    // Static callback for miniaudio
    static void data_callback(struct ma_device* pDevice, void* pOutput, const void* pInput, unsigned int frameCount);
};

#endif // AUDIO_PLAYER_H
