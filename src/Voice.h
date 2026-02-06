#ifndef VOICE_H
#define VOICE_H

#include "Instrument.h"
#include "tsf.h"
#include <map>
#include <string>

// Biquad Filter for Synth
struct BiquadState {
    float z1 = 0.0f, z2 = 0.0f;
    double a0, a1, a2, b1, b2;

    void update(FilterType type, float cutoff, float q, float sample_rate);
    float process(float in);
};

class Voice {
public:
    Instrument instrument;
    double start_time_samples;
    bool is_active = false;
    bool is_finished = false;

    // State
    size_t current_note_idx = 0;
    double samples_into_note = 0;
    float last_freq = -1.0f;
    float phase = 0.0f;
    float lfo_phase = 0.0f;
    BiquadState filter_state;
    tsf* soundfont_instance = nullptr;

    // Effect State
    double total_samples_rendered = 0;
    double total_duration_samples = 0;
    std::vector<std::vector<float>> delay_buffers; // One per delay effect
    std::vector<int> delay_indices;

    Voice(const Instrument& inst, double start_samples, float sample_rate);
    ~Voice();

    // Render a block of stereo samples
    void render(float* buffer, int frame_count, float sample_rate, std::map<std::string, tsf*>& soundfonts);
};

#endif // VOICE_H
