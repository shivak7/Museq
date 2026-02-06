#ifdef _WIN32
    #define NOMINMAX
#endif
#define _USE_MATH_DEFINES
#include "AudioUtils.h"
#include <cmath>
#include <algorithm>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void mix_buffers_stereo(std::vector<float>& target, const std::vector<float>& source, int offset) {
    size_t required_size = offset + source.size();
    if (required_size > target.size()) target.resize(required_size, 0.0f);
    for (size_t i = 0; i < source.size(); ++i) {
        target[offset + i] += source[i];
    }
}

void mix_buffers_stereo(float* target, const float* source, int target_frames, int source_frames, int offset_frames) {
    int frames_to_mix = (std::min)(source_frames, target_frames - offset_frames);
    if (frames_to_mix <= 0) return;
    for (int i = 0; i < frames_to_mix * 2; ++i) {
        target[offset_frames * 2 + i] += source[i];
    }
}

void get_pan_gains(float pan, float& left, float& right) {
    float p = (pan + 1.0f) / 2.0f;
    left = cos(p * M_PI / 2.0f);
    right = sin(p * M_PI / 2.0f);
}

void apply_effects(std::vector<float>& buffer, const std::vector<Effect>& effects, float sample_rate) {
    apply_effects(buffer.data(), buffer.size() / 2, effects, sample_rate);
}

void apply_effects(float* buffer, int frame_count, const std::vector<Effect>& effects, float sample_rate) {
    for (const auto& fx : effects) {
        if (fx.type == EffectType::DELAY) {
            // Note: Streaming delay is tricky without a persistent state buffer.
            // For now, we'll implement simple delay if frame count allows.
            // In a real refactor, Delay should be an object with state.
        } 
        else if (fx.type == EffectType::DISTORTION) {
            float drive = fx.param1;
            if (drive < 1.0f) drive = 1.0f;
            for (int i = 0; i < frame_count * 2; ++i) {
                buffer[i] = std::tanh(buffer[i] * drive);
            }
        }
        else if (fx.type == EffectType::BITCRUSH) {
            float depth = fx.param1;
            if (depth < 1.0f) depth = 1.0f;
            float steps = std::pow(2.0f, depth);
            for (int i = 0; i < frame_count * 2; ++i) {
                buffer[i] = std::round(buffer[i] * steps) / steps;
            }
        }
        // ... other effects ...
    }
}

float generate_sample_with_phase(Waveform waveform, float freq, float sample_rate, float& phase) {
    float sample = 0;
    switch (waveform) {
        case Waveform::SINE: sample = sin(phase); break;
        case Waveform::SQUARE: sample = sin(phase) > 0 ? 1.0 : -1.0; break;
        case Waveform::TRIANGLE: sample = asin(sin(phase)) * (2.0 / M_PI); break;
        case Waveform::SAWTOOTH: sample = (2.0 / M_PI) * (fmod(phase, 2.0 * M_PI) - M_PI); break;
    }
    phase += 2.0 * M_PI * freq / sample_rate;
    return sample;
}
