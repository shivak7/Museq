#include <iostream>
#include <cassert>
#include "../src/Scale.h"
#include "../src/Chord.h"

int main() {
    // Test Scale
    Scale c_major(ScaleType::MAJOR, "C");
    assert(c_major.get_pitch(1, 4) == 60); // C4
    assert(c_major.get_pitch(3, 4) == 64); // E4
    assert(c_major.get_pitch(5, 4) == 67); // G4

    // Test Chord
    Chord c_maj_chord("Cmaj");
    std::vector<int> notes = c_maj_chord.get_notes(4);
    assert(notes.size() == 3);
    assert(notes[0] == 60); // C4
    assert(notes[1] == 64); // E4
    assert(notes[2] == 67); // G4

    std::cout << "All music theory tests passed!" << std::endl;
    return 0;
}
