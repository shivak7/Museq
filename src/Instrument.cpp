#include "Instrument.h"

Instrument::Instrument(std::string name, Waveform waveform, AdsrEnvelope envelope) {
    this->name = name;
    this->type = InstrumentType::SYNTH;
    this->synth.waveform = waveform;
    this->synth.envelope = envelope;
    this->synth.filter = Filter();
    this->synth.lfo = LFO();
    this->sampler = nullptr;
    this->portamento_time = 0.0f;
    this->pan = 0.0f;
}

Instrument::Instrument(std::string name, const std::string& sample_path) {
    this->name = name;
    this->type = InstrumentType::SAMPLER;
    this->sampler = new Sampler(sample_path);
    this->portamento_time = 0.0f;
    this->pan = 0.0f;
}

Instrument::Instrument(std::string name, const std::string& soundfont_path, int bank_index, int preset_index) {
    this->name = name;
    this->type = InstrumentType::SOUNDFONT;
    this->soundfont_path = soundfont_path;
    this->bank_index = bank_index;
    this->preset_index = preset_index;
    this->sampler = nullptr;
    this->portamento_time = 0.0f;
    this->pan = 0.0f;
}

Instrument::~Instrument() {
    if (sampler) {
        delete sampler;
    }
}

// Copy constructor
Instrument::Instrument(const Instrument& other)
    : name(other.name),
      sequence(other.sequence),
      type(other.type),
      synth(other.synth),
      sampler(nullptr),
      soundfont_path(other.soundfont_path),
      bank_index(other.bank_index),
      preset_index(other.preset_index),
      portamento_time(other.portamento_time),
      pan(other.pan) {
    if (other.sampler) {
        sampler = new Sampler(*other.sampler);
    }
}

// Copy assignment operator
Instrument& Instrument::operator=(const Instrument& other) {
    if (this == &other) {
        return *this;
    }

    name = other.name;
    sequence = other.sequence;
    type = other.type;
    synth = other.synth;
    soundfont_path = other.soundfont_path;
    bank_index = other.bank_index;
    preset_index = other.preset_index;
    portamento_time = other.portamento_time;
    pan = other.pan;

    if (sampler) {
        delete sampler;
        sampler = nullptr;
    }

    if (other.sampler) {
        sampler = new Sampler(*other.sampler);
    }

    return *this;
}

// Move constructor
Instrument::Instrument(Instrument&& other) noexcept
    : name(std::move(other.name)),
      sequence(std::move(other.sequence)),
      type(other.type),
      synth(std::move(other.synth)),
      sampler(other.sampler),
      soundfont_path(std::move(other.soundfont_path)),
      bank_index(other.bank_index),
      preset_index(other.preset_index),
      portamento_time(other.portamento_time),
      pan(other.pan) {
    other.sampler = nullptr;
}

// Move assignment operator
Instrument& Instrument::operator=(Instrument&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    name = std::move(other.name);
    sequence = std::move(other.sequence);
    type = other.type;
    synth = std::move(other.synth);
    soundfont_path = std::move(other.soundfont_path);
    bank_index = other.bank_index;
    preset_index = other.preset_index;
    portamento_time = other.portamento_time;
    pan = other.pan;

    if (sampler) {
        delete sampler;
    }
    sampler = other.sampler;
    other.sampler = nullptr;

    return *this;
}
