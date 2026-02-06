#include <iostream>
#include <cassert>
#include "../src/Scale.h"
#include "../src/Chord.h"
#include "../src/NoteParser.h"

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

    // Test NoteParser with Scale
    // "1" in C Major octave 4 -> C4 (60)
    // "3" in C Major octave 4 -> E4 (64)
    // "5" in C Major octave 4 -> G4 (67)
    // "b3" in C Major? Maybe not for now. Just plain degrees.
    
    int p1 = NoteParser::parse("1", c_major, 4);
    assert(p1 == 60);
    
    int p3 = NoteParser::parse("3", c_major, 4);
    assert(p3 == 64);

    std::cout << "All music theory tests passed!" << std::endl;
    return 0;
}