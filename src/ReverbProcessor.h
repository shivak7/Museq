#ifndef REVERB_PROCESSOR_H
#define REVERB_PROCESSOR_H

#include <vector>

// Simple Comb Filter for Reverb
struct CombFilter {
    std::vector<float> buffer;
    int write_idx = 0;
    float feedback = 0.5f;
    float filter_state = 0.0f;
    float dampening = 0.2f;

    CombFilter(int size) : buffer(size, 0.0f) {}

    float process(float in) {
        float out = buffer[write_idx];
        filter_state = (out * (1.0f - dampening)) + (filter_state * dampening);
        buffer[write_idx] = in + (filter_state * feedback);
        if (++write_idx >= buffer.size()) write_idx = 0;
        return out;
    }
};

// Simple All-pass Filter
struct AllPassFilter {
    std::vector<float> buffer;
    int write_idx = 0;
    float feedback = 0.5f;

    AllPassFilter(int size) : buffer(size, 0.0f) {}

    float process(float in) {
        float buf_out = buffer[write_idx];
        float out = -in + buf_out;
        buffer[write_idx] = in + (buf_out * feedback);
        if (++write_idx >= buffer.size()) write_idx = 0;
        return out;
    }
};

class ReverbProcessor {
public:
    ReverbProcessor(float sample_rate) {
        // Freeverb-ish constants
        const int comb_sizes[] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
        const int allpass_sizes[] = {556, 441, 341, 225};
        
        for (int s : comb_sizes) m_combs.emplace_back(static_cast<int>(s * (sample_rate / 44100.0f)));
        for (int s : allpass_sizes) m_allpasses.emplace_back(static_cast<int>(s * (sample_rate / 44100.0f)));
    }

    void set_params(float room_size, float damp) {
        for (auto& c : m_combs) {
            c.feedback = room_size;
            c.dampening = damp;
        }
    }

    void process(float* left, float* right, int frames) {
        for (int i = 0; i < frames; ++i) {
            float in = (left[i] + right[i]) * 0.5f;
            float out = 0;
            
            // Parallel combs
            for (auto& c : m_combs) out += c.process(in);
            
            // Serial all-passes
            for (auto& a : m_allpasses) out = a.process(out);
            
            // Mix (Wet/Dry handled outside or here?)
            // For now, simple additive mix
            float wet = 0.2f;
            left[i] += out * wet;
            right[i] += out * wet;
        }
    }

private:
    std::vector<CombFilter> m_combs;
    std::vector<AllPassFilter> m_allpasses;
};

#endif
