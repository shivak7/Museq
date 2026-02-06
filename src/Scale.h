#ifndef SCALE_H
#define SCALE_H

#include <string>
#include <vector>
#include <map>

enum class ScaleType {
    MAJOR,
    MINOR,
    DORIAN,
    PHRYGIAN,
    LYDIAN,
    MIXOLYDIAN,
    LOCRIAN
};

class Scale {
public:
    Scale(ScaleType type, const std::string& root_note);
    
    // Get MIDI pitch for a scale degree (1-based) and octave
    int get_pitch(int degree, int octave) const;

private:
    ScaleType m_type;
    std::string m_root_note;
    int m_root_pitch_class; // 0-11
    std::vector<int> m_intervals;

    void init_intervals();
    int get_pitch_class(const std::string& note);
};

#endif // SCALE_H
