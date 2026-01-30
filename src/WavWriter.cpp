#include "WavWriter.h"
#include "sndfile.h"
#include "AudioRenderer.h"

void WavWriter::write(AudioRenderer& renderer, const Song& song, const std::string& file_path, float sample_rate) {
    std::vector<float> pcm_buffer = renderer.render(song, sample_rate);

    // Write to WAV file
    SF_INFO sfinfo;
    sfinfo.frames = pcm_buffer.size() / 2;
    sfinfo.samplerate = sample_rate;
    sfinfo.channels = 2;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    SNDFILE* outfile = sf_open(file_path.c_str(), SFM_WRITE, &sfinfo);
    sf_write_float(outfile, pcm_buffer.data(), pcm_buffer.size());
    sf_close(outfile);
}
