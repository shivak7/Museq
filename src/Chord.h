#ifndef CHORD_H
#define CHORD_H

#include <string>
#include <vector>

class Chord {
public:
    Chord(const std::string& chord_name);
    
    // Get MIDI pitches for the chord in a given octave
    std::vector<int> get_notes(int octave) const;

private:
    std::string m_root_note;
    std::string m_quality;
    std::vector<int> m_intervals;

    void parse_chord_name(const std::string& name);
    void init_intervals();
};

#endif // CHORD_H
