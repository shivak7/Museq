#include "Mp3Writer.h"
#include "lame/lame.h"
#include <vector>
#include "AudioRenderer.h"

void Mp3Writer::write(AudioRenderer& renderer, const Song& song, const std::string& file_path, float sample_rate) {
    std::vector<float> pcm_buffer = renderer.render(song, sample_rate);

    // Initialize the LAME encoder
    lame_global_flags* gfp = lame_init();
    lame_set_in_samplerate(gfp, sample_rate);
    lame_set_VBR(gfp, vbr_default);
    lame_init_params(gfp);

    // Encode the PCM data
    int num_samples = pcm_buffer.size();
    std::vector<unsigned char> mp3_buffer(num_samples * 1.25 + 7200);
    int mp3_bytes = lame_encode_buffer_ieee_float(gfp, pcm_buffer.data(), pcm_buffer.data(), num_samples, mp3_buffer.data(), mp3_buffer.size());

    // Write the MP3 data to a file
    FILE* mp3_file = fopen(file_path.c_str(), "wb");
    fwrite(mp3_buffer.data(), 1, mp3_bytes, mp3_file);
    fclose(mp3_file);

    // Clean up
    lame_close(gfp);
}
