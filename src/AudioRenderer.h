#ifndef AUDIO_RENDERER_H
#define AUDIO_RENDERER_H

#include "Song.h"
#include "Voice.h"
#include <vector>
#include <map>
#include <string>
#include <memory>

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

    // Helper to query soundfont
    static void print_soundfont_presets(const std::string& path);

private:
    float m_sample_rate = 44100.0f;
    long m_current_sample = 0;
    long m_total_samples = 0;
    
    std::map<std::string, tsf*> m_soundfonts;
    std::vector<std::unique_ptr<Voice>> m_scheduled_voices;
    std::vector<Voice*> m_active_voices;

    void flatten_song(std::shared_ptr<SongElement> element, double current_time_ms);
};

#endif // AUDIO_RENDERER_H