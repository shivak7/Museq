#ifndef AUDIO_RENDERER_H
#define AUDIO_RENDERER_H

#include "Song.h"
#include "Voice.h"
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <mutex>

class AudioRenderer {
public:
    AudioRenderer();
    ~AudioRenderer();
    
    // --- Legacy Interface ---
    std::vector<float> render(const Song& song, float sample_rate = 44100.0f);

    // --- Streaming Interface ---
    void load(const Song& song, float sample_rate = 44100.0f);
    void render_block(float* output, int frame_count);
    bool is_finished() const;
    double get_current_time_ms() const { return (double)m_current_sample / m_sample_rate * 1000.0; }
    double get_total_duration_ms() const { return (double)m_total_samples / m_sample_rate * 1000.0; }
    size_t get_active_voice_count() const { std::lock_guard<std::mutex> lock(m_mutex); return m_active_voices.size(); }
    size_t get_scheduled_voice_count() const { std::lock_guard<std::mutex> lock(m_mutex); return m_scheduled_voices.size(); }

    // Helper to query soundfont
    static void print_soundfont_presets(const std::string& path);

private:
    float m_sample_rate = 44100.0f;
    long m_current_sample = 0;
    long m_total_samples = 0;
    
    std::map<std::string, tsf*> m_soundfonts;
    std::vector<std::unique_ptr<Voice>> m_scheduled_voices;
    std::vector<Voice*> m_active_voices;
    mutable std::mutex m_mutex;

    void flatten_song(std::shared_ptr<SongElement> element, double current_time_ms, const std::vector<Effect>& parent_effects);
};

#endif // AUDIO_RENDERER_H