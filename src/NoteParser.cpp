#include "NoteParser.h"
#include <map>
#include <cctype>
#include <string>
#include <algorithm>

static std::map<char, int> note_values = {
    {'C', 0}, {'D', 2}, {'E', 4}, {'F', 5}, {'G', 7}, {'A', 9}, {'B', 11}
};

int NoteParser::parse(const std::string& note_name, int default_octave) {
    if (note_name.empty()) return 0;

    std::string upper_name = note_name;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);

    if (upper_name == "R" || upper_name == "REST") {
        return -1;
    }

    // Check if it's already a MIDI number
    if (std::all_of(note_name.begin(), note_name.end(), ::isdigit)) {
        try {
            return std::stoi(note_name);
        } catch (...) {
            return 0;
        }
    }

    size_t i = 0;
    char note_char = std::toupper(note_name[i++]);
    int offset = 0;

    if (i < note_name.length() && (note_name[i] == '#' || note_name[i] == 'b')) {
        if (note_name[i] == '#') offset = 1;
        else if (note_name[i] == 'b') offset = -1;
        i++;
    }

    int octave = default_octave;
    if (i < note_name.length()) {
        try {
            octave = std::stoi(note_name.substr(i));
        } catch (...) {
            // Use default
        }
    }

    if (note_values.count(note_char)) {
        int note_value = note_values[note_char];
        return 12 + (octave * 12) + note_value + offset;
    }
    
    return 0;
}