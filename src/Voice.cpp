#ifdef _WIN32
    #define NOMINMAX
    #define _USE_MATH_DEFINES
#endif
#include "Voice.h"
#include "AudioUtils.h"
#include <cmath>
#include <algorithm>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "tsf.h"

void BiquadState::update(FilterType type, float cutoff, float q, float sample_rate) {
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

float BiquadState::process(float in) {
    float out = in * a0 + z1;
    z1 = in * a1 + z2 - b1 * out;
    z2 = in * a2 - b2 * out;
    return out;
}

Voice::Voice(const Instrument& inst, double start_samples, float sample_rate) 
    : instrument(inst), start_time_samples(start_samples) {
    
    double total_ms = 0;
    for (const auto& note : instrument.sequence.notes) {
        total_ms += note.duration;
    }
    total_duration_samples = (total_ms / 1000.0) * sample_rate;

    for (const auto& fx : instrument.effects) {
        if (fx.type == EffectType::DELAY) {
            // Max delay of 2 seconds
            int size = static_cast<int>(sample_rate * 2);
            delay_buffers.push_back(std::vector<float>(size * 2, 0.0f));
            delay_indices.push_back(0);
        } else {
            delay_buffers.push_back({});
            delay_indices.push_back(0);
        }
    }
}

Voice::~Voice() {
    if (soundfont_instance) {
        tsf_close(soundfont_instance);
    }
}

void Voice::render(float* buffer, int frame_count, float sample_rate, std::map<std::string, tsf*>& soundfonts) {
    if (is_finished) return;

    bool legato = instrument.portamento_time > 0;
    const auto& notes = instrument.sequence.notes;

    // Local buffer for this voice
    std::vector<float> local_buffer(frame_count * 2, 0.0f);

    for (int f = 0; f < frame_count; ++f) {
        if (current_note_idx >= notes.size()) {
            is_finished = true;
            break;
        }

        const auto& note = notes[current_note_idx];
        double note_duration_samples = (note.duration / 1000.0f) * sample_rate;
        
        if (note.is_rest) {
            samples_into_note++;
            total_samples_rendered++;
            if (samples_into_note >= note_duration_samples) {
                samples_into_note = 0;
                current_note_idx++;
                last_freq = -1.0f;
            }
            continue;
        }

        // --- RENDER SAMPLE ---
        float mono_sample = 0.0f;
        float left_gain, right_gain;
        get_pan_gains(note.pan, left_gain, right_gain);

        float target_freq = 440.0f * pow(2.0f, (note.pitch - 69.0f) / 12.0f);

        // Universal Envelope Calculation
        const auto& env = instrument.synth.envelope;
        float time_in_note = (float)samples_into_note / sample_rate;
        float envelope_val = 0.0f;
        
        if (legato) {
            bool is_first = (current_note_idx == 0);
            bool is_last = (current_note_idx == notes.size() - 1);
            if (is_first && time_in_note < env.attack) envelope_val = time_in_note / env.attack;
            else if (is_first && time_in_note < env.attack + env.decay) envelope_val = 1.0f - (1.0f - env.sustain) * ((time_in_note - env.attack) / env.decay);
            else if (is_last && time_in_note > (note.duration / 1000.0f) - env.release) envelope_val = env.sustain * (1.0f - ((time_in_note - ((note.duration / 1000.0f) - env.release)) / env.release));
            else envelope_val = env.sustain;
        } else {
            if (time_in_note < env.attack) envelope_val = time_in_note / env.attack;
            else if (time_in_note < env.attack + env.decay) envelope_val = 1.0f - (1.0f - env.sustain) * ((time_in_note - env.attack) / env.decay);
            else if (time_in_note < (note.duration / 1000.0f) - env.release) envelope_val = env.sustain;
            else envelope_val = env.sustain * (1.0f - ((time_in_note - ((note.duration / 1000.0f) - env.release)) / env.release));
        }
        if (envelope_val < 0.0f) envelope_val = 0.0f;

        // Universal LFO / Pitch Calculation
        float current_freq = target_freq;
        float lfo_val = 0.0f;
        if (instrument.synth.lfo.target != LFOTarget::NONE) {
            lfo_val = generate_sample_with_phase(instrument.synth.lfo.waveform, instrument.synth.lfo.frequency, sample_rate, lfo_phase);
            lfo_val *= instrument.synth.lfo.amount;
        }
        if (instrument.synth.lfo.target == LFOTarget::PITCH) {
            current_freq *= pow(2.0f, lfo_val / 12.0f);
        }

        if (instrument.type == InstrumentType::SOUNDFONT) {
            if (!soundfont_instance && soundfonts.count(instrument.soundfont_path)) {
                soundfont_instance = tsf_copy(soundfonts[instrument.soundfont_path]);
                tsf_set_output(soundfont_instance, TSF_STEREO_INTERLEAVED, sample_rate, 0);
                int preset = tsf_get_presetindex(soundfont_instance, instrument.bank_index, instrument.preset_index);
                tsf_channel_set_presetindex(soundfont_instance, 0, (preset < 0 ? 0 : preset));
                tsf_channel_note_on(soundfont_instance, 0, note.pitch, note.velocity / 127.0f);
            }

            if (soundfont_instance) {
                // Adjust pitch if LFO is targeting it
                if (instrument.synth.lfo.target == LFOTarget::PITCH) {
                    tsf_channel_set_pitchwheel(soundfont_instance, 0, (int)(lfo_val * 8192.0f / 2.0f) + 8192); // Basic pitch wheel mapping
                }

                float stereo[2];
                tsf_render_float(soundfont_instance, stereo, 1);
                mono_sample = (stereo[0] + stereo[1]) * 0.5f; // Mix to mono
            }
        } else if (instrument.type == InstrumentType::SAMPLER) {
            if (instrument.sampler) {
                mono_sample = instrument.sampler->get_sample(time_in_note);
                mono_sample *= (note.velocity / 127.0f);
            }
        } else {
            // Synth Logic
            if (last_freq < 0) last_freq = current_freq;
            
            float synth_freq = current_freq;
            float portamento_samples = (instrument.portamento_time / 1000.0f) * sample_rate;
            if (legato && samples_into_note < portamento_samples) {
                float t = (float)samples_into_note / portamento_samples;
                synth_freq = last_freq + (current_freq - last_freq) * t;
            }

            mono_sample = (note.velocity / 127.0f) * 0.5f * generate_sample_with_phase(instrument.synth.waveform, synth_freq, sample_rate, phase);
        }

        // Apply Universal Amplitude Modulation (Envelope + LFO)
        float amp_mod = 1.0f;
        if (instrument.synth.lfo.target == LFOTarget::AMPLITUDE) {
            amp_mod += lfo_val;
            if (amp_mod < 0) amp_mod = 0;
        }
        mono_sample *= envelope_val * amp_mod;

        // Apply Universal Filter
        if (instrument.synth.filter.type != FilterType::NONE) {
            float current_cutoff = instrument.synth.filter.cutoff;
            if (instrument.synth.lfo.target == LFOTarget::FILTER_CUTOFF) current_cutoff += lfo_val; 
            filter_state.update(instrument.synth.filter.type, current_cutoff, instrument.synth.filter.resonance, sample_rate);
            mono_sample = filter_state.process(mono_sample);
        }

        local_buffer[f * 2] = mono_sample * left_gain;
        local_buffer[f * 2 + 1] = mono_sample * right_gain;

        samples_into_note++;
        total_samples_rendered++;
        if (samples_into_note >= note_duration_samples) {
            samples_into_note = 0;
            current_note_idx++;
            last_freq = current_freq;
            if (soundfont_instance && current_note_idx < notes.size()) {
                tsf_channel_note_off(soundfont_instance, 0, note.pitch);
                tsf_channel_note_on(soundfont_instance, 0, notes[current_note_idx].pitch, notes[current_note_idx].velocity / 127.0f);
            }
        }
    }

    // Apply Gain
    if (instrument.gain != 1.0f) {
        for (float& s : local_buffer) s *= instrument.gain;
    }

    // Apply Effects
    for (size_t fx_idx = 0; fx_idx < instrument.effects.size(); ++fx_idx) {
        const auto& fx = instrument.effects[fx_idx];
        if (fx.type == EffectType::DISTORTION) {
            float drive = fx.param1;
            for (float& s : local_buffer) s = std::tanh(s * drive);
        }
        else if (fx.type == EffectType::BITCRUSH) {
            float steps = std::pow(2.0f, fx.param1);
            for (float& s : local_buffer) s = std::round(s * steps) / steps;
        }
        else if (fx.type == EffectType::TREMOLO) {
            float rate = fx.param1;
            float depth = fx.param2;
            for (int f = 0; f < frame_count; ++f) {
                float mod = 1.0f - depth * (0.5f * (1.0f + std::sin(2.0f * M_PI * rate * (total_samples_rendered - frame_count + f) / sample_rate)));
                local_buffer[f * 2] *= mod;
                local_buffer[f * 2 + 1] *= mod;
            }
        }
        else if (fx.type == EffectType::FADE_IN) {
            float duration_samples = (fx.param1 / 1000.0f) * sample_rate;
            for (int f = 0; f < frame_count; ++f) {
                double absolute_sample = total_samples_rendered - frame_count + f;
                if (absolute_sample < duration_samples) {
                    float gain = static_cast<float>(absolute_sample / duration_samples);
                    local_buffer[f * 2] *= gain;
                    local_buffer[f * 2 + 1] *= gain;
                }
            }
        }
        else if (fx.type == EffectType::FADE_OUT) {
            float duration_samples = (fx.param1 / 1000.0f) * sample_rate;
            float start_fade_sample = static_cast<float>(total_duration_samples - duration_samples);
            for (int f = 0; f < frame_count; ++f) {
                double absolute_sample = total_samples_rendered - frame_count + f;
                if (absolute_sample > start_fade_sample) {
                    float gain = 1.0f - static_cast<float>((absolute_sample - start_fade_sample) / duration_samples);
                    if (gain < 0) gain = 0;
                    local_buffer[f * 2] *= gain;
                    local_buffer[f * 2 + 1] *= gain;
                }
            }
        }
        else if (fx.type == EffectType::DELAY) {
            float time_ms = fx.param1;
            float feedback = fx.param2;
            int delay_samples = static_cast<int>((time_ms / 1000.0f) * sample_rate);
            auto& delay_buf = delay_buffers[fx_idx];
            int& delay_idx = delay_indices[fx_idx];

            for (int f = 0; f < frame_count; ++f) {
                int read_idx = (delay_idx - delay_samples + (delay_buf.size() / 2)) % (delay_buf.size() / 2);
                local_buffer[f * 2] += delay_buf[read_idx * 2] * feedback;
                local_buffer[f * 2 + 1] += delay_buf[read_idx * 2 + 1] * feedback;

                delay_buf[delay_idx * 2] = local_buffer[f * 2];
                delay_buf[delay_idx * 2 + 1] = local_buffer[f * 2 + 1];
                delay_idx = (delay_idx + 1) % (delay_buf.size() / 2);
            }
        }
    }

    // Mix into main buffer
    for (size_t i = 0; i < local_buffer.size(); ++i) {
        buffer[i] += local_buffer[i];
    }
}