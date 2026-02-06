#include "Scale.h"
#include "NoteParser.h"
#include <stdexcept>

Scale::Scale(ScaleType type, const std::string& root_note) 
    : m_type(type), m_root_note(root_note) {
    
    // Calculate root pitch class (0-11)
    // We use octave 0 to get the base pitch class
    int midi = NoteParser::parse(root_note, 0);
    m_root_pitch_class = midi % 12;
    
    init_intervals();
}

void Scale::init_intervals() {
    switch (m_type) {
        case ScaleType::MAJOR:
            m_intervals = {0, 2, 4, 5, 7, 9, 11};
            break;
        case ScaleType::MINOR:
            m_intervals = {0, 2, 3, 5, 7, 8, 10};
            break;
        case ScaleType::DORIAN:
            m_intervals = {0, 2, 3, 5, 7, 9, 10};
            break;
        case ScaleType::PHRYGIAN:
            m_intervals = {0, 1, 3, 5, 7, 8, 10};
            break;
        case ScaleType::LYDIAN:
            m_intervals = {0, 2, 4, 6, 7, 9, 11};
            break;
        case ScaleType::MIXOLYDIAN:
            m_intervals = {0, 2, 4, 5, 7, 9, 10};
            break;
        case ScaleType::LOCRIAN:
            m_intervals = {0, 1, 3, 5, 6, 8, 10};
            break;
    }
}

int Scale::get_pitch(int degree, int octave) const {
    if (degree < 1) degree = 1;
    
    // 0-based index in the scale
    int index = degree - 1;
    
    // Calculate how many octaves up we are relative to the input octave
    int octave_shift = index / 7;
    int interval_index = index % 7;
    
    int interval = m_intervals[interval_index];
    
    // Base pitch is root_pitch_class + 12 * octave + 12 (NoteParser offset)
    int base_pitch = m_root_pitch_class + (12 * octave) + 12;
    
    return base_pitch + interval + (12 * octave_shift);
}
