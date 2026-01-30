#ifndef SAMPLER_H
#define SAMPLER_H

#include <string>
#include <vector>

class Sampler {
public:
    Sampler(const std::string& file_path);
    Sampler(const Sampler& other); // Copy constructor
    float get_sample(float time);

private:
    std::vector<float> samples;
    float sample_rate;
};

#endif // SAMPLER_H
