#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <string>
#include <vector>
#include "Sequence.h"
#include "Waveform.h"
#include "AdsrEnvelope.h"
#include "Sampler.h"
#include "Effect.h"

enum class InstrumentType {
    SYNTH,
    SAMPLER,
    SOUNDFONT
};

enum class FilterType {
    NONE,
    LOWPASS,
    HIGHPASS,
    BANDPASS
};

struct Filter {
    FilterType type = FilterType::NONE;
    float cutoff = 20000.0f; // Hz
    float resonance = 0.0f;  // 0.0 to 1.0ish
};

enum class LFOTarget {
    NONE,
    PITCH,
    AMPLITUDE,
    FILTER_CUTOFF
};

struct LFO {
    LFOTarget target = LFOTarget::NONE;
    Waveform waveform = Waveform::SINE;
    float frequency = 0.0f; // Hz
    float amount = 0.0f;    // Context dependent
};

struct Synth {
    Waveform waveform;
    float frequency = 1.0f; // Multiplier
    AdsrEnvelope envelope;
    Filter filter;
    LFO lfo;
};

class Instrument {
public:
    std::string name;
    Sequence sequence;
    InstrumentType type = InstrumentType::SYNTH;

    // For SYNTH
    Synth synth;

    // For SAMPLER
    Sampler* sampler = nullptr;

    // For SOUNDFONT
    std::string soundfont_path;
    int bank_index = 0;
    int preset_index = 0;

    float portamento_time = 0.0f; // In milliseconds
    float pan = 0.0f; // -1.0 to 1.0
    float gain = 1.0f; // 0.0 to 1.0+
    
    // Effects Chain
    std::vector<Effect> effects;

    Instrument() = default;
    Instrument(std::string name, Waveform waveform = Waveform::SINE, AdsrEnvelope envelope = AdsrEnvelope());
    Instrument(std::string name, const std::string& sample_path);
    Instrument(std::string name, const std::string& soundfont_path, int bank_index, int preset_index);
    
    // Rule of Five
    ~Instrument();
    Instrument(const Instrument& other);
    Instrument& operator=(const Instrument& other);
    Instrument(Instrument&& other) noexcept;
    Instrument& operator=(Instrument&& other) noexcept;
};

#endif // INSTRUMENT_H
