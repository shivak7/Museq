#include "AudioRenderer.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include "SongElement.h"

#define TSF_IMPLEMENTATION
#include "tsf.h"

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

            for (int i = 0; i < num_note_samples; ++i) {
                float time_in_note = (float)i / sample_rate;
                float current_freq = target_freq;
                if (legato && i < portamento_samples) {
                    float t = (float)i / portamento_samples;
                    current_freq = last_freq + (target_freq - last_freq) * t;
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
                
                float mono_sample = 0.0f;
                if(instrument.type == InstrumentType::SYNTH) 
                    mono_sample = (note.velocity / 127.0f) * 0.5f * envelope * generate_sample_with_phase(instrument.synth.waveform, current_freq, sample_rate, phase);
                else if (instrument.type == InstrumentType::SAMPLER && instrument.sampler)
                    mono_sample = (note.velocity / 127.0f) * 0.5f * envelope * instrument.sampler->get_sample(time_in_note);

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

std::vector<float> render_recursive_stereo(std::shared_ptr<SongElement> element, float sample_rate, std::map<std::string, tsf*>& soundfonts) {
    if (auto inst_elem = std::dynamic_pointer_cast<InstrumentElement>(element)) {
        return render_instrument_stereo(inst_elem->instrument, sample_rate, soundfonts);
    } else if (auto comp_elem = std::dynamic_pointer_cast<CompositeElement>(element)) {
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
