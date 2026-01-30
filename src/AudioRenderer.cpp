#include "AudioRenderer.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include "SongElement.h"

#define TSF_IMPLEMENTATION
#include "tsf.h"

// Biquad Filter Structure
struct Biquad {
    float z1 = 0.0f, z2 = 0.0f;
    double a0, a1, a2, b1, b2;

    void update(FilterType type, float cutoff, float q, float sample_rate) {
        if (type == FilterType::NONE) return;
        
        // Clamp cutoff to be below Nyquist
        if (cutoff > sample_rate * 0.49f) cutoff = sample_rate * 0.49f;
        if (cutoff < 10.0f) cutoff = 10.0f;
        if (q < 0.1f) q = 0.1f;

        double w0 = 2.0 * M_PI * cutoff / sample_rate;
        double alpha = sin(w0) / (2.0 * q);
        double cosw0 = cos(w0);

        if (type == FilterType::LOWPASS) {
            double norm = 1.0 / (1.0 + alpha);
            a0 = ((1.0 - cosw0) / 2.0) * norm;
            a1 = (1.0 - cosw0) * norm;
            a2 = ((1.0 - cosw0) / 2.0) * norm;
            b1 = -2.0 * cosw0 * norm;
            b2 = (1.0 - alpha) * norm;
        } else if (type == FilterType::HIGHPASS) {
            double norm = 1.0 / (1.0 + alpha);
            a0 = ((1.0 + cosw0) / 2.0) * norm;
            a1 = -(1.0 + cosw0) * norm;
            a2 = ((1.0 + cosw0) / 2.0) * norm;
            b1 = -2.0 * cosw0 * norm;
            b2 = (1.0 - alpha) * norm;
        } else if (type == FilterType::BANDPASS) {
            double norm = 1.0 / (1.0 + alpha);
            a0 = alpha * norm;
            a1 = 0;
            a2 = -alpha * norm;
            b1 = -2.0 * cosw0 * norm;
            b2 = (1.0 - alpha) * norm;
        }
    }

    float process(float in) {
        float out = in * a0 + z1;
        z1 = in * a1 + z2 - b1 * out;
        z2 = in * a2 - b2 * out;
        return out;
    }
};

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

AudioRenderer::AudioRenderer() {}

AudioRenderer::~AudioRenderer() {
    for (auto const& [key, val] : m_soundfonts) {
        tsf_close(val);
    }
}

void mix_buffers_stereo(std::vector<float>& target, const std::vector<float>& source) {
    if (source.size() > target.size()) target.resize(source.size(), 0.0f);
    for (size_t i = 0; i < source.size(); ++i) target[i] += source[i];
}

void get_pan_gains(float pan, float& left, float& right) {
    float p = (pan + 1.0f) / 2.0f;
    left = cos(p * M_PI / 2.0f);
    right = sin(p * M_PI / 2.0f);
}

std::vector<float> render_instrument_stereo(const Instrument& instrument, float sample_rate, std::map<std::string, tsf*>& soundfonts) {
    float instrument_duration = 0;
    for (const auto& note : instrument.sequence.notes) {
        instrument_duration += note.duration / 1000.0f;
    }
    
    int num_samples = static_cast<int>(instrument_duration * sample_rate);
    std::vector<float> buffer(num_samples * 2, 0.0f);

    if (instrument.type == InstrumentType::SOUNDFONT) {
        if (soundfonts.count(instrument.soundfont_path) && soundfonts[instrument.soundfont_path]) {
            tsf* inst_tsf = tsf_copy(soundfonts[instrument.soundfont_path]);
            tsf_set_output(inst_tsf, TSF_STEREO_INTERLEAVED, sample_rate, 0);
            int actual_preset = tsf_get_presetindex(inst_tsf, instrument.bank_index, instrument.preset_index);
            if (actual_preset < 0) actual_preset = 0;

            tsf_channel_set_presetindex(inst_tsf, 0, actual_preset);

            float current_time = 0;
            for (const auto& note : instrument.sequence.notes) {
                int start_sample = static_cast<int>(current_time * sample_rate);
                int note_samples = static_cast<int>((note.duration / 1000.0f) * sample_rate);
                if (note_samples > 0) {
                    float tsf_pan = (note.pan + 1.0f) / 2.0f;
                    tsf_channel_set_pan(inst_tsf, 0, tsf_pan);

                    tsf_channel_note_on(inst_tsf, 0, note.pitch, note.velocity / 127.0f);
                    
                    std::vector<float> note_buffer(note_samples * 2);
                    tsf_render_float(inst_tsf, note_buffer.data(), note_samples);
                    
                    for(int i = 0; i < note_samples * 2; ++i) {
                        if ((start_sample * 2) + i < buffer.size()) buffer[(start_sample * 2) + i] += note_buffer[i];
                    }
                    tsf_channel_note_off(inst_tsf, 0, note.pitch);
                }
                current_time += note.duration / 1000.0f;
            }
            tsf_close(inst_tsf);
        }
    } else {
        float current_time = 0;
        float last_freq = -1.0f;
        float phase = 0.0f;
        float lfo_phase = 0.0f; // LFO Phase
        
        bool legato = instrument.portamento_time > 0;

        for (size_t n_idx = 0; n_idx < instrument.sequence.notes.size(); ++n_idx) {
            const auto& note = instrument.sequence.notes[n_idx];
            bool is_first_note = (n_idx == 0);
            bool is_last_note = (n_idx == instrument.sequence.notes.size() - 1);
            
            int start_sample = static_cast<int>(current_time * sample_rate);
            int num_note_samples = static_cast<int>((note.duration / 1000.0f) * sample_rate);
            float target_freq = 440.0 * pow(2.0, (note.pitch - 69.0) / 12.0);
            
            if (last_freq < 0) last_freq = target_freq;
            float portamento_samples = (instrument.portamento_time / 1000.0f) * sample_rate;

            const auto& env = instrument.synth.envelope;
            float left_gain, right_gain;
            get_pan_gains(note.pan, left_gain, right_gain);

            // Initialize filter for this note (or instrument if we wanted continuity, but keeping simple)
            Biquad filter;
            if (instrument.synth.filter.type != FilterType::NONE) {
                filter.update(instrument.synth.filter.type, instrument.synth.filter.cutoff, instrument.synth.filter.resonance, sample_rate);
            }

            for (int i = 0; i < num_note_samples; ++i) {
                float time_in_note = (float)i / sample_rate;
                
                // LFO Calculation
                float lfo_val = 0.0f;
                if (instrument.synth.lfo.target != LFOTarget::NONE) {
                    lfo_val = generate_sample_with_phase(instrument.synth.lfo.waveform, instrument.synth.lfo.frequency, sample_rate, lfo_phase);
                    lfo_val *= instrument.synth.lfo.amount;
                }

                // Portamento Frequency
                float current_freq = target_freq;
                if (legato && i < portamento_samples) {
                    float t = (float)i / portamento_samples;
                    current_freq = last_freq + (target_freq - last_freq) * t;
                }

                // Apply LFO to Pitch
                if (instrument.synth.lfo.target == LFOTarget::PITCH) {
                    current_freq *= pow(2.0f, lfo_val / 12.0f);
                }

                // Envelope
                float envelope = 0.0f;
                if (legato) {
                    if (is_first_note && time_in_note < env.attack) envelope = time_in_note / env.attack;
                    else if (is_first_note && time_in_note < env.attack + env.decay) envelope = 1.0f - (1.0f - env.sustain) * ((time_in_note - env.attack) / env.decay);
                    else if (is_last_note && time_in_note > (note.duration / 1000.0f) - env.release) envelope = env.sustain * (1.0f - ((time_in_note - ((note.duration / 1000.0f) - env.release)) / env.release));
                    else envelope = env.sustain;
                } else {
                    if (time_in_note < env.attack) envelope = time_in_note / env.attack;
                    else if (time_in_note < env.attack + env.decay) envelope = 1.0f - (1.0f - env.sustain) * ((time_in_note - env.attack) / env.decay);
                    else if (time_in_note < (note.duration / 1000.0f) - env.release) envelope = env.sustain;
                    else envelope = env.sustain * (1.0f - ((time_in_note - ((note.duration / 1000.0f) - env.release)) / env.release));
                }
                if (envelope < 0.0f) envelope = 0.0f;
                
                // Modulate Amplitude LFO
                float amp_mod = 1.0f;
                if (instrument.synth.lfo.target == LFOTarget::AMPLITUDE) {
                    amp_mod += lfo_val;
                    if(amp_mod < 0) amp_mod = 0;
                }

                // Generate Raw Sample
                float mono_sample = 0.0f;
                if(instrument.type == InstrumentType::SYNTH) 
                    mono_sample = (note.velocity / 127.0f) * 0.5f * envelope * amp_mod * generate_sample_with_phase(instrument.synth.waveform, current_freq, sample_rate, phase);
                else if (instrument.type == InstrumentType::SAMPLER && instrument.sampler)
                    mono_sample = (note.velocity / 127.0f) * 0.5f * envelope * amp_mod * instrument.sampler->get_sample(time_in_note);

                // Apply Filter
                if (instrument.synth.filter.type != FilterType::NONE) {
                    float current_cutoff = instrument.synth.filter.cutoff;
                    // Modulate Cutoff LFO
                    if (instrument.synth.lfo.target == LFOTarget::FILTER_CUTOFF) {
                        current_cutoff += lfo_val; 
                    }
                    // Update filter coefficients if cutoff changed (LFO)
                    if (instrument.synth.lfo.target == LFOTarget::FILTER_CUTOFF) {
                         filter.update(instrument.synth.filter.type, current_cutoff, instrument.synth.filter.resonance, sample_rate);
                    }
                    mono_sample = filter.process(mono_sample);
                }

                if ((start_sample + i) * 2 + 1 < buffer.size()) {
                    buffer[(start_sample + i) * 2] += mono_sample * left_gain;
                    buffer[(start_sample + i) * 2 + 1] += mono_sample * right_gain;
                }
            }
            current_time += note.duration / 1000.0f;
            last_freq = target_freq;
        }
    }
    return buffer;
}

// Forward declaration
std::vector<float> render_recursive_stereo(std::shared_ptr<SongElement> element, float sample_rate, std::map<std::string, tsf*>& soundfonts);

std::vector<float> render_auto_loop(std::shared_ptr<CompositeElement> loop_elem, float sample_rate, std::map<std::string, tsf*>& soundfonts) {
    if (loop_elem->children.empty()) return {};

    // 1. Render Leader (Child 0)
    auto leader_buffer = render_recursive_stereo(loop_elem->children[0], sample_rate, soundfonts);
    size_t target_samples = leader_buffer.size() / 2; // Stereo frames

    if (target_samples == 0) return {};

    // 2. Render Followers
    for (size_t i = 1; i < loop_elem->children.size(); ++i) {
        auto follower_buffer = render_recursive_stereo(loop_elem->children[i], sample_rate, soundfonts);
        size_t follower_frames = follower_buffer.size() / 2;
        if (follower_frames == 0) continue;

        for (size_t f = 0; f < target_samples; ++f) {
            size_t src_f = f % follower_frames;
            leader_buffer[f * 2] += follower_buffer[src_f * 2];
            leader_buffer[f * 2 + 1] += follower_buffer[src_f * 2 + 1];
        }
    }
    return leader_buffer;
}

std::vector<float> render_recursive_stereo(std::shared_ptr<SongElement> element, float sample_rate, std::map<std::string, tsf*>& soundfonts) {
    if (auto inst_elem = std::dynamic_pointer_cast<InstrumentElement>(element)) {
        return render_instrument_stereo(inst_elem->instrument, sample_rate, soundfonts);
    } else if (auto comp_elem = std::dynamic_pointer_cast<CompositeElement>(element)) {
        if (comp_elem->type == CompositeType::AUTO_LOOP) {
            return render_auto_loop(comp_elem, sample_rate, soundfonts);
        }
        
        std::vector<float> buffer;
        if (comp_elem->type == CompositeType::SEQUENTIAL) {
            for (auto child : comp_elem->children) {
                auto child_buffer = render_recursive_stereo(child, sample_rate, soundfonts);
                buffer.insert(buffer.end(), child_buffer.begin(), child_buffer.end());
            }
        } else { // PARALLEL
            for (auto child : comp_elem->children) {
                auto child_buffer = render_recursive_stereo(child, sample_rate, soundfonts);
                mix_buffers_stereo(buffer, child_buffer);
            }
        }
        return buffer;
    }
    return {};
}

std::vector<float> AudioRenderer::render(const Song& song, float sample_rate) {
    auto preloader = [&](auto self, std::shared_ptr<SongElement> element) -> void {
        if (auto inst_elem = std::dynamic_pointer_cast<InstrumentElement>(element)) {
            const auto& instrument = inst_elem->instrument;
            if (instrument.type == InstrumentType::SOUNDFONT && instrument.soundfont_path != "") {
                if (m_soundfonts.find(instrument.soundfont_path) == m_soundfonts.end()) {
                    m_soundfonts[instrument.soundfont_path] = tsf_load_filename(instrument.soundfont_path.c_str());
                    if (m_soundfonts[instrument.soundfont_path]) {
                        tsf_set_output(m_soundfonts[instrument.soundfont_path], TSF_STEREO_INTERLEAVED, sample_rate, 0);
                    }
                }
            }
        } else if (auto comp_elem = std::dynamic_pointer_cast<CompositeElement>(element)) {
            for (auto child : comp_elem->children) self(self, child);
        }
    };
    preloader(preloader, song.root);

    auto buffer = render_recursive_stereo(song.root, sample_rate, m_soundfonts);
    
    // Normalization (handles stereo)
    float max_val = 0.0f;
    for (float s : buffer) if (std::abs(s) > max_val) max_val = std::abs(s);
    if (max_val > 0.0f) {
        float gain = 0.9f / max_val;
        for (float& s : buffer) s *= gain;
    }

    return buffer;
}
