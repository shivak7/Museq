#include "Voice.h"
#include "AudioUtils.h"
#include <cmath>
#include <algorithm>
#include <cstring>

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

Voice::Voice(const Instrument& inst, double start_samples) 
    : instrument(inst), start_time_samples(start_samples) {
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

    for (int f = 0; f < frame_count; ++f) {
        if (current_note_idx >= notes.size()) {
            is_finished = true;
            break;
        }

        const auto& note = notes[current_note_idx];
        double note_duration_samples = (note.duration / 1000.0f) * sample_rate;
        
        if (note.is_rest) {
            samples_into_note++;
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

        if (instrument.type == InstrumentType::SOUNDFONT) {
            if (!soundfont_instance && soundfonts.count(instrument.soundfont_path)) {
                soundfont_instance = tsf_copy(soundfonts[instrument.soundfont_path]);
                tsf_set_output(soundfont_instance, TSF_STEREO_INTERLEAVED, sample_rate, 0);
                int preset = tsf_get_presetindex(soundfont_instance, instrument.bank_index, instrument.preset_index);
                tsf_channel_set_presetindex(soundfont_instance, 0, (preset < 0 ? 0 : preset));
                tsf_channel_note_on(soundfont_instance, 0, note.pitch, note.velocity / 127.0f);
            }

            if (soundfont_instance) {
                float stereo[2];
                tsf_render_float(soundfont_instance, stereo, 1);
                mono_sample = (stereo[0] + stereo[1]) * 0.5f; // Mix to mono then re-pan
            }
        } else {
            // Synth Logic
            if (last_freq < 0) last_freq = target_freq;
            
            float current_freq = target_freq;
            float portamento_samples = (instrument.portamento_time / 1000.0f) * sample_rate;
            if (legato && samples_into_note < portamento_samples) {
                float t = (float)samples_into_note / portamento_samples;
                current_freq = last_freq + (target_freq - last_freq) * t;
            }

            // LFO
            float lfo_val = 0.0f;
            if (instrument.synth.lfo.target != LFOTarget::NONE) {
                lfo_val = generate_sample_with_phase(instrument.synth.lfo.waveform, instrument.synth.lfo.frequency, sample_rate, lfo_phase);
                lfo_val *= instrument.synth.lfo.amount;
            }
            if (instrument.synth.lfo.target == LFOTarget::PITCH) {
                current_freq *= pow(2.0f, lfo_val / 12.0f);
            }

            // Envelope
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

            float amp_mod = 1.0f;
            if (instrument.synth.lfo.target == LFOTarget::AMPLITUDE) {
                amp_mod += lfo_val;
                if (amp_mod < 0) amp_mod = 0;
            }

            mono_sample = (note.velocity / 127.0f) * 0.5f * envelope_val * amp_mod * generate_sample_with_phase(instrument.synth.waveform, current_freq, sample_rate, phase);
            
            if (instrument.synth.filter.type != FilterType::NONE) {
                float current_cutoff = instrument.synth.filter.cutoff;
                if (instrument.synth.lfo.target == LFOTarget::FILTER_CUTOFF) current_cutoff += lfo_val; 
                filter_state.update(instrument.synth.filter.type, current_cutoff, instrument.synth.filter.resonance, sample_rate);
                mono_sample = filter_state.process(mono_sample);
            }
        }

        buffer[f * 2] += mono_sample * left_gain;
        buffer[f * 2 + 1] += mono_sample * right_gain;

        samples_into_note++;
        if (samples_into_note >= note_duration_samples) {
            samples_into_note = 0;
            current_note_idx++;
            last_freq = target_freq;
            if (soundfont_instance && current_note_idx < notes.size()) {
                tsf_channel_note_off(soundfont_instance, 0, note.pitch);
                tsf_channel_note_on(soundfont_instance, 0, notes[current_note_idx].pitch, notes[current_note_idx].velocity / 127.0f);
            }
        }
    }
}