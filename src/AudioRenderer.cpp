#define _USE_MATH_DEFINES
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

void apply_effects(std::vector<float>& buffer, const std::vector<Effect>& effects, float sample_rate) {
    for (const auto& fx : effects) {
        if (fx.type == EffectType::DELAY) {
            float time_ms = fx.param1;
            float feedback = fx.param2;
            int delay_frames = (int)(time_ms * sample_rate / 1000.0f);
            if (delay_frames <= 0) continue;
            
            size_t original_frames = buffer.size() / 2;
            size_t tail_frames = delay_frames * 4; 
            buffer.resize((original_frames + tail_frames) * 2, 0.0f);
            
            for (size_t i = 0; i < buffer.size(); i += 2) { 
                int prev_idx = i - delay_frames * 2;
                if (prev_idx >= 0) {
                    buffer[i] += buffer[prev_idx] * feedback;
                    buffer[i+1] += buffer[prev_idx+1] * feedback;
                }
            }
        } 
        else if (fx.type == EffectType::DISTORTION) {
            float drive = fx.param1;
            if (drive < 1.0f) drive = 1.0f;
            for (float& s : buffer) {
                s = std::tanh(s * drive);
            }
        }
        else if (fx.type == EffectType::BITCRUSH) {
            float depth = fx.param1;
            if (depth < 1.0f) depth = 1.0f;
            float steps = std::pow(2.0f, depth);
            for (float& s : buffer) {
                s = std::round(s * steps) / steps;
            }
        }
        else if (fx.type == EffectType::FADE_IN) {
            float duration = fx.param1;
            int frames = (int)(duration * sample_rate / 1000.0f);
            for (size_t i = 0; i < buffer.size(); i += 2) {
                size_t frame_idx = i / 2;
                if (frame_idx < frames) {
                    float gain = (float)frame_idx / frames;
                    buffer[i] *= gain;
                    buffer[i+1] *= gain;
                }
            }
        }
        else if (fx.type == EffectType::FADE_OUT) {
            float duration = fx.param1;
            int frames = (int)(duration * sample_rate / 1000.0f);
            size_t total_frames = buffer.size() / 2;
            size_t start_frame = (total_frames > frames) ? total_frames - frames : 0;
            for (size_t i = 0; i < buffer.size(); i += 2) {
                size_t frame_idx = i / 2;
                if (frame_idx >= start_frame) {
                    float gain = 1.0f - ((float)(frame_idx - start_frame) / frames);
                    if (gain < 0.0f) gain = 0.0f;
                    buffer[i] *= gain;
                    buffer[i+1] *= gain;
                }
            }
        }
        else if (fx.type == EffectType::TREMOLO) {
             float rate = fx.param1;
             float depth = fx.param2;
             float phase = 0.0f;
             float phase_inc = 2.0f * M_PI * rate / sample_rate;
             for (size_t i = 0; i < buffer.size(); i += 2) {
                 float mod = 1.0f - depth * (0.5f * (1.0f + std::sin(phase)));
                 buffer[i] *= mod;
                 buffer[i+1] *= mod;
                 phase += phase_inc;
             }
        }
    }
}

std::vector<float> render_instrument_stereo(const Instrument& instrument, float sample_rate, std::map<std::string, tsf*>& soundfonts) {
    float instrument_duration = 0;
    for (const auto& note : instrument.sequence.notes) {
        instrument_duration += note.duration / 1000.0f;
    }
    
    int num_samples = static_cast<int>(instrument_duration * sample_rate);
    std::vector<float> buffer(num_samples * 2, 0.0f);

    float current_time = 0;
    float last_freq = -1.0f;
    bool legato = instrument.portamento_time > 0;

    if (instrument.type == InstrumentType::SOUNDFONT) {
        if (soundfonts.count(instrument.soundfont_path) && soundfonts[instrument.soundfont_path]) {
            tsf* inst_tsf = tsf_copy(soundfonts[instrument.soundfont_path]);
            tsf_set_output(inst_tsf, TSF_STEREO_INTERLEAVED, sample_rate, 0);
            int actual_preset = tsf_get_presetindex(inst_tsf, instrument.bank_index, instrument.preset_index);
            if (actual_preset < 0) actual_preset = 0;

            tsf_channel_set_presetindex(inst_tsf, 0, actual_preset);
            tsf_channel_set_pitchrange(inst_tsf, 0, 24);

            for (const auto& note : instrument.sequence.notes) {
                int start_sample = static_cast<int>(current_time * sample_rate);
                int note_samples = static_cast<int>((note.duration / 1000.0f) * sample_rate);
                
                float target_freq = 440.0 * pow(2.0, (note.pitch - 69.0) / 12.0);
                if (last_freq < 0) last_freq = target_freq;
                float portamento_samples = (instrument.portamento_time / 1000.0f) * sample_rate;

                if (note_samples > 0 && !note.is_rest) {
                    float tsf_pan = (note.pan + 1.0f) / 2.0f;
                    tsf_channel_set_pan(inst_tsf, 0, tsf_pan);

                    tsf_channel_note_on(inst_tsf, 0, note.pitch, note.velocity / 127.0f);
                    
                    if (legato) {
                        const int BLOCK_SIZE = 64;
                        int samples_processed = 0;
                        while (samples_processed < note_samples) {
                            int chunk = std::min(BLOCK_SIZE, note_samples - samples_processed);
                            float current_freq = target_freq;
                            if (samples_processed < portamento_samples) {
                                float t = (float)samples_processed / portamento_samples;
                                current_freq = last_freq + (target_freq - last_freq) * t;
                            }
                            if (std::abs(current_freq - target_freq) > 0.1f) {
                                 float ratio = current_freq / target_freq;
                                 float cents = 1200.0f * log2(ratio);
                                 float normalized = cents / 2400.0f; 
                                 int wheel = 8192 + static_cast<int>(normalized * 8192.0f);
                                 if (wheel < 0) wheel = 0; if (wheel > 16383) wheel = 16383;
                                 tsf_channel_set_pitchwheel(inst_tsf, 0, wheel);
                            } else {
                                 tsf_channel_set_pitchwheel(inst_tsf, 0, 8192);
                            }
                            std::vector<float> chunk_buffer(chunk * 2);
                            tsf_render_float(inst_tsf, chunk_buffer.data(), chunk);
                            for(int k = 0; k < chunk * 2; ++k) {
                                if ((start_sample * 2) + (samples_processed * 2) + k < buffer.size()) 
                                    buffer[(start_sample * 2) + (samples_processed * 2) + k] += chunk_buffer[k];
                            }
                            samples_processed += chunk;
                        }
                    } else {
                        tsf_channel_set_pitchwheel(inst_tsf, 0, 8192);
                        std::vector<float> note_buffer(note_samples * 2);
                        tsf_render_float(inst_tsf, note_buffer.data(), note_samples);
                        for(int i = 0; i < note_samples * 2; ++i) {
                            if ((start_sample * 2) + i < buffer.size()) 
                                buffer[(start_sample * 2) + i] += note_buffer[i];
                        }
                    }
                    tsf_channel_note_off(inst_tsf, 0, note.pitch);
                } else if (note.is_rest) {
                    last_freq = -1.0f;
                }
                current_time += note.duration / 1000.0f;
                last_freq = target_freq;
            }
            tsf_close(inst_tsf);
        }
    } else {
        float phase = 0.0f;
        float lfo_phase = 0.0f; 
        for (size_t n_idx = 0; n_idx < instrument.sequence.notes.size(); ++n_idx) {
            const auto& note = instrument.sequence.notes[n_idx];
            bool is_first_note = (n_idx == 0);
            bool is_last_note = (n_idx == instrument.sequence.notes.size() - 1);
            if (note.is_rest) {
                current_time += note.duration / 1000.0f;
                last_freq = -1.0f;
                continue;
            }
            int start_sample = static_cast<int>(current_time * sample_rate);
            int num_note_samples = static_cast<int>((note.duration / 1000.0f) * sample_rate);
            float target_freq = 440.0 * pow(2.0, (note.pitch - 69.0) / 12.0);
            if (last_freq < 0) last_freq = target_freq;
            float portamento_samples = (instrument.portamento_time / 1000.0f) * sample_rate;
            const auto& env = instrument.synth.envelope;
            float left_gain, right_gain;
            get_pan_gains(note.pan, left_gain, right_gain);
            Biquad filter;
            if (instrument.synth.filter.type != FilterType::NONE) {
                filter.update(instrument.synth.filter.type, instrument.synth.filter.cutoff, instrument.synth.filter.resonance, sample_rate);
            }
            for (int i = 0; i < num_note_samples; ++i) {
                float time_in_note = (float)i / sample_rate;
                float lfo_val = 0.0f;
                if (instrument.synth.lfo.target != LFOTarget::NONE) {
                    lfo_val = generate_sample_with_phase(instrument.synth.lfo.waveform, instrument.synth.lfo.frequency, sample_rate, lfo_phase);
                    lfo_val *= instrument.synth.lfo.amount;
                }
                float current_freq = target_freq;
                if (legato && i < portamento_samples) {
                    float t = (float)i / portamento_samples;
                    current_freq = last_freq + (target_freq - last_freq) * t;
                }
                if (instrument.synth.lfo.target == LFOTarget::PITCH) {
                    current_freq *= pow(2.0f, lfo_val / 12.0f);
                }
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
                float amp_mod = 1.0f;
                if (instrument.synth.lfo.target == LFOTarget::AMPLITUDE) {
                    amp_mod += lfo_val;
                    if(amp_mod < 0) amp_mod = 0;
                }
                float mono_sample = 0.0f;
                if(instrument.type == InstrumentType::SYNTH) 
                    mono_sample = (note.velocity / 127.0f) * 0.5f * envelope * amp_mod * generate_sample_with_phase(instrument.synth.waveform, current_freq, sample_rate, phase);
                else if (instrument.type == InstrumentType::SAMPLER && instrument.sampler)
                    mono_sample = (note.velocity / 127.0f) * 0.5f * envelope * amp_mod * instrument.sampler->get_sample(time_in_note);
                if (instrument.synth.filter.type != FilterType::NONE) {
                    float current_cutoff = instrument.synth.filter.cutoff;
                    if (instrument.synth.lfo.target == LFOTarget::FILTER_CUTOFF) current_cutoff += lfo_val; 
                    if (instrument.synth.lfo.target == LFOTarget::FILTER_CUTOFF) filter.update(instrument.synth.filter.type, current_cutoff, instrument.synth.filter.resonance, sample_rate);
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
    
    apply_effects(buffer, instrument.effects, sample_rate);

    return buffer;
}

std::vector<float> render_recursive_stereo(std::shared_ptr<SongElement> element, float sample_rate, std::map<std::string, tsf*>& soundfonts);

std::vector<float> render_auto_loop(std::shared_ptr<CompositeElement> loop_elem, float sample_rate, std::map<std::string, tsf*>& soundfonts) {
    if (loop_elem->children.empty()) return {};
    auto leader_buffer = render_recursive_stereo(loop_elem->children[0], sample_rate, soundfonts);
    size_t target_samples = leader_buffer.size() / 2;
    if (target_samples == 0) return {};
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
        if (comp_elem->type == CompositeType::AUTO_LOOP) return render_auto_loop(comp_elem, sample_rate, soundfonts);
        std::vector<float> buffer;
        if (comp_elem->type == CompositeType::SEQUENTIAL) {
            for (auto child : comp_elem->children) {
                auto child_buffer = render_recursive_stereo(child, sample_rate, soundfonts);
                buffer.insert(buffer.end(), child_buffer.begin(), child_buffer.end());
            }
        } else {
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
    float max_val = 0.0f;
    for (float s : buffer) if (std::abs(s) > max_val) max_val = std::abs(s);
    if (max_val > 0.0f) {
        float gain = 0.9f / max_val;
        for (float& s : buffer) s *= gain;
    }
    return buffer;
}

void AudioRenderer::print_soundfont_presets(const std::string& path) {
    tsf* f = tsf_load_filename(path.c_str());
    if (!f) {
        std::cerr << "Error: Could not load SoundFont file: " << path << std::endl;
        return;
    }
    int count = tsf_get_presetcount(f);
    std::cout << "Found " << count << " instruments in " << path << ":" << std::endl;
    std::cout << "Index\tBank\tPreset\tName" << std::endl;
    std::cout << "-----\t----\t------\t----" << std::endl;
    for (int i = 0; i < count; ++i) {
        std::cout << i << "\t" 
                  << f->presets[i].bank << "\t" 
                  << f->presets[i].preset << "\t" 
                  << f->presets[i].presetName << std::endl;
    }
    tsf_close(f);
}