#ifdef _WIN32
    #define NOMINMAX
#endif
#define MINIAUDIO_IMPLEMENTATION
#include "../third_party/miniaudio/miniaudio.h"
#include "AudioPlayer.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <complex>
#include <vector>
#include <cmath>

namespace {
    const double PI = 3.14159265358979323846;

    // Iterative FFT for better performance
    void fft_iterative(std::vector<std::complex<double>>& a) {
        int n = a.size();
        for (int i = 1, j = 0; i < n; i++) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) std::swap(a[i], a[j]);
        }
        for (int len = 2; len <= n; len <<= 1) {
            double ang = 2 * PI / len;
            std::complex<double> wlen(cos(ang), sin(ang));
            for (int i = 0; i < n; i += len) {
                std::complex<double> w(1);
                for (int j = 0; j < len / 2; j++) {
                    std::complex<double> u = a[i + j], v = a[i + j + len / 2] * w;
                    a[i + j] = u + v;
                    a[i + j + len / 2] = u - v;
                    w *= wlen;
                }
            }
        }
    }
}

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

void AudioPlayer::play(const Song& song, bool is_preview) {
    stop();

    m_renderer.load(song, 44100);
    m_playing = true;
    m_is_preview = is_preview;
    m_preview_samples_elapsed = 0;

    if (ma_device_start((ma_device*)m_device) != MA_SUCCESS) {
        std::cerr << "Failed to start playback device." << std::endl;
    }
}

void AudioPlayer::stop() {
    m_playing = false;
    m_is_preview = false;
    ma_device_stop((ma_device*)m_device);
}

bool AudioPlayer::is_playing() const {
    return m_playing && !m_renderer.is_finished();
}

double AudioPlayer::get_playback_position_ms() const {
    return m_renderer.get_current_time_ms();
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

void AudioPlayer::get_spectrum_data(float* out_magnitudes, int count) {
    const int N = 1024;
    std::vector<std::complex<double>> data(N);
    
    float buffer[N];
    get_visualization_data(buffer, N);
    
    for (int i = 0; i < N; ++i) {
        // Hann window to reduce leakage
        double window = 0.5 * (1.0 - cos(2.0 * PI * i / (N - 1)));
        data[i] = std::complex<double>(buffer[i] * window, 0);
    }
    
    fft_iterative(data);
    
    // Magnitudes (only first half)
    for (int i = 0; i < count && i < N/2; ++i) {
        // Normalize by N and apply a simple logarithmic-like scaling for better visual
        float mag = (float)std::abs(data[i]) / N;
        out_magnitudes[i] = mag;
    }
}

void AudioPlayer::data_callback(ma_device* pDevice, void* pOutput, const void* pInput, unsigned int frameCount) {
    AudioPlayer* player = (AudioPlayer*)pDevice->pUserData;
    if (!player || !player->m_playing) {
        std::memset(pOutput, 0, frameCount * 2 * sizeof(float));
        return;
    }

    player->m_renderer.render_block((float*)pOutput, frameCount);

    // Apply Preview Fade-out if needed
    if (player->m_is_preview) {
        float* out = (float*)pOutput;
        double fade_start = 4.0 * 44100.0;
        double preview_end = 5.0 * 44100.0;

        for (unsigned int i = 0; i < frameCount; ++i) {
            float gain = 1.0f;
            if (player->m_preview_samples_elapsed >= preview_end) {
                gain = 0.0f;
                player->m_playing = false;
            } else if (player->m_preview_samples_elapsed >= fade_start) {
                gain = 1.0f - (float)((player->m_preview_samples_elapsed - fade_start) / (preview_end - fade_start));
            }
            
            out[i * 2] *= gain;
            out[i * 2 + 1] *= gain;
            player->m_preview_samples_elapsed++;
        }
    }

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