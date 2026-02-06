#include "Chord.h"
#include "NoteParser.h"
#include <algorithm>

Chord::Chord(const std::string& chord_name) {
    parse_chord_name(chord_name);
    init_intervals();
}

void Chord::parse_chord_name(const std::string& name) {
    // Basic parsing: split into root and quality
    // Example: Cmaj -> Root: C, Quality: maj
    // Example: F#min7 -> Root: F#, Quality: min7
    
    if (name.empty()) return;
    
    // Find where the note name ends
    size_t len = 1;
    if (name.length() > 1 && (name[1] == '#' || name[1] == 'b')) {
        len = 2;
    }
    
    m_root_note = name.substr(0, len);
    if (name.length() > len) {
        m_quality = name.substr(len);
    } else {
        m_quality = "maj"; // Default to major if no quality specified? Or strictly require it?
        // Let's assume major if just "C"
    }
}

void Chord::init_intervals() {
    // Basic qualities
    if (m_quality == "maj" || m_quality == "M" || m_quality == "") {
        m_intervals = {0, 4, 7};
    } else if (m_quality == "min" || m_quality == "m") {
        m_intervals = {0, 3, 7};
    } else if (m_quality == "7" || m_quality == "dom7") {
        m_intervals = {0, 4, 7, 10};
    } else if (m_quality == "maj7" || m_quality == "M7") {
        m_intervals = {0, 4, 7, 11};
    } else if (m_quality == "min7" || m_quality == "m7") {
        m_intervals = {0, 3, 7, 10};
    } else if (m_quality == "dim") {
        m_intervals = {0, 3, 6};
    } else if (m_quality == "aug") {
        m_intervals = {0, 4, 8};
    } else if (m_quality == "sus4") {
        m_intervals = {0, 5, 7};
    } else if (m_quality == "sus2") {
        m_intervals = {0, 2, 7};
    } else if (m_quality == "maj9" || m_quality == "M9") {
        m_intervals = {0, 4, 7, 11, 14};
    } else if (m_quality == "min9" || m_quality == "m9") {
        m_intervals = {0, 3, 7, 10, 14};
    } else if (m_quality == "9") {
        m_intervals = {0, 4, 7, 10, 14};
    } else if (m_quality == "add9") {
        m_intervals = {0, 4, 7, 14};
    }
}

std::vector<int> Chord::get_notes(int octave) const {
    std::vector<int> notes;
    int root_midi = NoteParser::parse(m_root_note, octave);
    
    for (int interval : m_intervals) {
        notes.push_back(root_midi + interval);
    }
    
    return notes;
}
