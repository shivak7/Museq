#include "JsonSerializer.h"
#include <fstream>
#include "SongElement.h"

// Forward declarations
void to_json(json& j, const Note& note);
void from_json(const json& j, Note& note);
void to_json(json& j, const Sequence& sequence);
void from_json(const json& j, Sequence& sequence);
void to_json(json& j, const AdsrEnvelope& env);
void from_json(const json& j, AdsrEnvelope& env);
void to_json(json& j, const Instrument& instrument);
void from_json(const json& j, Instrument& instrument);

void to_json(json& j, const Note& note) {
    j = json{{"pitch", note.pitch}, {"duration", note.duration}, {"velocity", note.velocity}, {"pan", note.pan}};
}
void from_json(const json& j, Note& note) {
    note.pitch = j.at("pitch").get<int>();
    note.duration = j.at("duration").get<int>();
    note.velocity = j.at("velocity").get<int>();
    if (j.contains("pan")) note.pan = j.at("pan").get<float>();
}

void to_json(json& j, const Sequence& sequence) {
    j = json{{"notes", sequence.notes}};
}
void from_json(const json& j, Sequence& sequence) {
    sequence.notes = j.at("notes").get<std::vector<Note>>();
}

void to_json(json& j, const AdsrEnvelope& env) {
    j = json{{"attack", env.attack}, {"decay", env.decay}, {"sustain", env.sustain}, {"release", env.release}};
}
void from_json(const json& j, AdsrEnvelope& env) {
    env.attack = j.at("attack").get<float>();
    env.decay = j.at("decay").get<float>();
    env.sustain = j.at("sustain").get<float>();
    env.release = j.at("release").get<float>();
}

void to_json(json& j, const Instrument& instrument) {
    j = json{{"name", instrument.name}, {"sequence", instrument.sequence}, {"type", (int)instrument.type}, {"portamento", instrument.portamento_time}, {"pan", instrument.pan}};
    if (instrument.type == InstrumentType::SYNTH) {
        j["synth"] = {{"waveform", (int)instrument.synth.waveform}, {"envelope", instrument.synth.envelope}};
    } else if (instrument.type == InstrumentType::SAMPLER) {
        // No sample path serialization yet
    } else if (instrument.type == InstrumentType::SOUNDFONT) {
        j["soundfont"] = {{"path", instrument.soundfont_path}, {"bank", instrument.bank_index}, {"preset", instrument.preset_index}};
    }
}
void from_json(const json& j, Instrument& instrument) {
    instrument.name = j.at("name").get<std::string>();
    instrument.sequence = j.at("sequence").get<Sequence>();
    instrument.type = (InstrumentType)j.at("type").get<int>();
}

json element_to_json(std::shared_ptr<SongElement> element) {
    if (auto inst = std::dynamic_pointer_cast<InstrumentElement>(element)) {
        return {{"type", "instrument"}, {"data", inst->instrument}};
    } else if (auto comp = std::dynamic_pointer_cast<CompositeElement>(element)) {
        json j_children = json::array();
        for (auto child : comp->children) j_children.push_back(element_to_json(child));
        return {{"type", "composite"}, {"composite_type", (int)comp->type}, {"children", j_children}};
    }
    return json();
}

void JsonSerializer::save(const Song& song, const std::string& file_path) {
    json j = element_to_json(song.root);
    std::ofstream o(file_path);
    o << std::setw(4) << j << std::endl;
}

Song JsonSerializer::load(const std::string& file_path) {
    // Loading tree is more complex, skipping for now
    return Song();
}