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

    // Temporary: Render fully to RAM first
    AudioRenderer renderer;
    m_playback_buffer = renderer.render(song, 44100);
    m_playback_cursor = 0;
    m_playing = true;

    if (ma_device_start((ma_device*)m_device) != MA_SUCCESS) {
        std::cerr << "Failed to start playback device." << std::endl;
    }
}

void AudioPlayer::stop() {
    m_playing = false;
    ma_device_stop((ma_device*)m_device);
    m_playback_cursor = 0;
}

bool AudioPlayer::is_playing() const {
    return m_playing;
}

void AudioPlayer::data_callback(ma_device* pDevice, void* pOutput, const void* pInput, unsigned int frameCount) {
    AudioPlayer* player = (AudioPlayer*)pDevice->pUserData;
    if (!player || !player->m_playing) return;

    float* out = (float*)pOutput;
    size_t samples_to_read = frameCount * 2; // Stereo
    size_t samples_available = player->m_playback_buffer.size() - player->m_playback_cursor;

    if (samples_available == 0) {
        player->m_playing = false; // Finished
        return;
    }

    size_t read_count = (std::min)(samples_to_read, samples_available);
    std::memcpy(out, &player->m_playback_buffer[player->m_playback_cursor], read_count * sizeof(float));
    player->m_playback_cursor += read_count;

    // Fill remainder with silence if we reached the end
    if (read_count < samples_to_read) {
        std::memset(out + read_count, 0, (samples_to_read - read_count) * sizeof(float));
    }
}
