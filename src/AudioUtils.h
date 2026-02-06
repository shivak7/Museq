#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <vector>
#include "Waveform.h"
#include "Effect.h"

void mix_buffers_stereo(std::vector<float>& target, const std::vector<float>& source, int offset = 0);
void mix_buffers_stereo(float* target, const float* source, int target_frames, int source_frames, int offset_frames);

void get_pan_gains(float pan, float& left, float& right);

void apply_effects(std::vector<float>& buffer, const std::vector<Effect>& effects, float sample_rate);
void apply_effects(float* buffer, int frame_count, const std::vector<Effect>& effects, float sample_rate);

float generate_sample_with_phase(Waveform waveform, float freq, float sample_rate, float& phase);

#endif // AUDIO_UTILS_H
