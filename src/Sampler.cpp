#include "Sampler.h"
#include "sndfile.h"
#include <iostream>

Sampler::Sampler(const std::string& file_path) : sample_rate(0.0f) {
    SF_INFO sfinfo;
    SNDFILE* infile = sf_open(file_path.c_str(), SFM_READ, &sfinfo);
    if (infile) {
        samples.resize(sfinfo.frames);
        sf_read_float(infile, samples.data(), sfinfo.frames);
        sample_rate = sfinfo.samplerate;
        sf_close(infile);
    } else {
        std::cerr << "Error: Could not open sample file " << file_path << std::endl;
    }
}

// Copy constructor
Sampler::Sampler(const Sampler& other)
    : samples(other.samples), sample_rate(other.sample_rate) {
}

float Sampler::get_sample(float time) {
    if (sample_rate == 0.0f || samples.empty()) {
        return 0.0f;
    }
    int index = static_cast<int>(time * sample_rate);
    if (index >= 0 && static_cast<size_t>(index) < samples.size()) {
        return samples[index];
    }
    return 0;
}
