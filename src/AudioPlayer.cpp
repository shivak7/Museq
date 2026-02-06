#ifdef _WIN32
    #define NOMINMAX
#endif
#define MINIAUDIO_IMPLEMENTATION
#include "../third_party/miniaudio/miniaudio.h"
#include "AudioPlayer.h"
#include <iostream>
#include <cstring>
#include <algorithm>

AudioPlayer::AudioPlayer() {
    m_device = new ma_device;
}

AudioPlayer::~AudioPlayer() {
    if (m_device) {
        ma_device_uninit((ma_device*)m_device);
        delete (ma_device*)m_device;
    }
}

bool AudioPlayer::init() {
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate        = 44100;
    config.dataCallback      = data_callback;
    config.pUserData         = this;

    if (ma_device_init(NULL, &config, (ma_device*)m_device) != MA_SUCCESS) {
        std::cerr << "Failed to initialize playback device." << std::endl;
        return false;
    }

    return true;
}

void AudioPlayer::play(const Song& song) {
    stop();

    m_renderer.load(song, 44100);
    m_playing = true;

    if (ma_device_start((ma_device*)m_device) != MA_SUCCESS) {
        std::cerr << "Failed to start playback device." << std::endl;
    }
}

void AudioPlayer::stop() {
    m_playing = false;
    ma_device_stop((ma_device*)m_device);
}

bool AudioPlayer::is_playing() const {
    return m_playing && !m_renderer.is_finished();
}

void AudioPlayer::get_visualization_data(float* out_buffer, int count) {
    if (count > VIS_BUFFER_SIZE) count = VIS_BUFFER_SIZE;
    
    // Copy samples from the ring buffer in the correct order
    int read_idx = (m_vis_write_idx - count + VIS_BUFFER_SIZE) % VIS_BUFFER_SIZE;
    for (int i = 0; i < count; ++i) {
        out_buffer[i] = m_vis_buffer[read_idx];
        read_idx = (read_idx + 1) % VIS_BUFFER_SIZE;
    }
}

void AudioPlayer::data_callback(ma_device* pDevice, void* pOutput, const void* pInput, unsigned int frameCount) {
    AudioPlayer* player = (AudioPlayer*)pDevice->pUserData;
    if (!player || !player->m_playing) {
        std::memset(pOutput, 0, frameCount * 2 * sizeof(float));
        return;
    }

    player->m_renderer.render_block((float*)pOutput, frameCount);

    // Update Visualization Buffer (Mono downmix of the block)
    float* out = (float*)pOutput;
    for (unsigned int i = 0; i < frameCount; ++i) {
        float mono = (out[i * 2] + out[i * 2 + 1]) * 0.5f;
        player->m_vis_buffer[player->m_vis_write_idx] = mono;
        player->m_vis_write_idx = (player->m_vis_write_idx + 1) % VIS_BUFFER_SIZE;
    }

    if (player->m_renderer.is_finished()) {
        player->m_playing = false;
    }
}