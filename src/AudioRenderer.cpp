#ifdef _WIN32
    #define NOMINMAX
#endif
#define _USE_MATH_DEFINES
#define TSF_IMPLEMENTATION
#include "AudioRenderer.h"
#include "AudioUtils.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cstring>
#include "SongElement.h"

AudioRenderer::AudioRenderer() {}

AudioRenderer::~AudioRenderer() {
    for (auto const& [key, val] : m_soundfonts) {
        tsf_close(val);
    }
}

void AudioRenderer::load(const Song& song, float sample_rate) {
    m_sample_rate = sample_rate;
    m_current_sample = 0;
    m_scheduled_voices.clear();
    m_active_voices.clear();

    // 1. Calculate Total Duration and Flatten Tree
    m_total_samples = static_cast<long>((song.root->get_duration_ms() / 1000.0f) * m_sample_rate);
    flatten_song(song.root, 0.0);

    // 2. Preload Soundfonts
    auto preloader = [&](auto self, std::shared_ptr<SongElement> element) -> void {
        if (auto inst_elem = std::dynamic_pointer_cast<InstrumentElement>(element)) {
            const auto& instrument = inst_elem->instrument;
            if (instrument.type == InstrumentType::SOUNDFONT && !instrument.soundfont_path.empty()) {
                if (m_soundfonts.find(instrument.soundfont_path) == m_soundfonts.end()) {
                    tsf* f = tsf_load_filename(instrument.soundfont_path.c_str());
                    if (f) {
                        tsf_set_output(f, TSF_STEREO_INTERLEAVED, m_sample_rate, 0);
                        m_soundfonts[instrument.soundfont_path] = f;
                    }
                }
            }
        } else if (auto comp_elem = std::dynamic_pointer_cast<CompositeElement>(element)) {
            for (auto child : comp_elem->children) self(self, child);
        }
    };
    preloader(preloader, song.root);
}

void AudioRenderer::flatten_song(std::shared_ptr<SongElement> element, double current_time_ms) {
    double start_time = current_time_ms + element->start_offset_ms;

    if (auto inst_elem = std::dynamic_pointer_cast<InstrumentElement>(element)) {
        double start_samples = (start_time / 1000.0) * m_sample_rate;
        m_scheduled_voices.push_back(std::make_unique<Voice>(inst_elem->instrument, start_samples));
    } 
    else if (auto comp_elem = std::dynamic_pointer_cast<CompositeElement>(element)) {
        if (comp_elem->type == CompositeType::SEQUENTIAL) {
            double local_time = start_time;
            for (auto child : comp_elem->children) {
                flatten_song(child, local_time);
                local_time += child->start_offset_ms + child->get_duration_ms();
            }
        } else if (comp_elem->type == CompositeType::PARALLEL) {
            for (auto child : comp_elem->children) {
                flatten_song(child, start_time);
            }
        } else if (comp_elem->type == CompositeType::AUTO_LOOP) {
            if (!comp_elem->children.empty()) {
                auto leader = comp_elem->children[0];
                double leader_dur = leader->get_duration_ms();
                flatten_song(leader, start_time);
                
                for (size_t i = 1; i < comp_elem->children.size(); ++i) {
                    auto follower = comp_elem->children[i];
                    double follower_dur = follower->get_duration_ms();
                    if (follower_dur <= 0) continue;
                    
                    double loop_time = 0;
                    while (loop_time < leader_dur) {
                        flatten_song(follower, start_time + loop_time);
                        loop_time += follower_dur;
                    }
                }
            }
        }
    }
}

void AudioRenderer::render_block(float* output, int frame_count) {
    std::memset(output, 0, frame_count * 2 * sizeof(float));

    // 1. Activate new voices
    for (auto& v : m_scheduled_voices) {
        if (!v->is_active && !v->is_finished && m_current_sample >= v->start_time_samples) {
            v->is_active = true;
            m_active_voices.push_back(v.get());
        }
    }

    // 2. Render active voices
    for (auto it = m_active_voices.begin(); it != m_active_voices.end(); ) {
        Voice* v = *it;
        v->render(output, frame_count, m_sample_rate, m_soundfonts);
        
        if (v->is_finished) {
            it = m_active_voices.erase(it);
        } else {
            ++it;
        }
    }

    m_current_sample += frame_count;
}

bool AudioRenderer::is_finished() const {
    return m_current_sample >= m_total_samples && m_active_voices.empty();
}

std::vector<float> AudioRenderer::render(const Song& song, float sample_rate) {
    load(song, sample_rate);
    
    std::vector<float> full_buffer;
    full_buffer.reserve(m_total_samples * 2);

    const int CHUNK_SIZE = 512;
    float chunk[CHUNK_SIZE * 2];

    while (!is_finished()) {
        render_block(chunk, CHUNK_SIZE);
        full_buffer.insert(full_buffer.end(), chunk, chunk + (CHUNK_SIZE * 2));
    }

    // Normalization
    float max_val = 0.0f;
    for (float s : full_buffer) if (std::abs(s) > max_val) max_val = std::abs(s);
    if (max_val > 0.0f) {
        float gain = 0.9f / max_val;
        for (float& s : full_buffer) s *= gain;
    }

    return full_buffer;
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
