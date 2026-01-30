#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <string>
#include "Sequence.h"
#include "Waveform.h"
#include "AdsrEnvelope.h"
#include "Sampler.h"

enum class InstrumentType {
    SYNTH,
    SAMPLER,
    SOUNDFONT
};

struct Synth {
    Waveform waveform;
    AdsrEnvelope envelope;
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