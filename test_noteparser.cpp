#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>

// Mocking NoteParser parts
static std::map<char, int> note_values = {
    {'C', 0}, {'D', 2}, {'E', 4}, {'F', 5}, {'G', 7}, {'A', 9}, {'B', 11}
};

int parse(const std::string& note_name) {
    if (note_name.empty()) return 0;

    std::string upper_name = note_name;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);

    if (upper_name == "R" || upper_name == "REST") {
        return -1;
    }

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

    int octave = 4;
    if (i < note_name.length()) {
        try {
            octave = std::stoi(note_name.substr(i));
        } catch (...) {
            // Default to 4
        }
    }

    if (note_values.count(note_char)) {
        int note_value = note_values[note_char];
        return 12 + (octave * 12) + note_value + offset;
    }
    
    return 0;
}

int main() {
    std::string test = "D}";
    int result = parse(test);
    std::cout << "Parsing 'D}': " << result << std::endl;
    
    std::string test2 = "}";
    int result2 = parse(test2);
    std::cout << "Parsing '}': " << result2 << std::endl;

    return 0;
}
