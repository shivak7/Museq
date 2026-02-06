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

    // Test more chords
    Chord d_min7("Dmin7");
    std::vector<int> d_min7_notes = d_min7.get_notes(4);
    assert(d_min7_notes.size() == 4);
    assert(d_min7_notes[0] == 62);
    assert(d_min7_notes[1] == 65);
    assert(d_min7_notes[2] == 69);
    assert(d_min7_notes[3] == 72);

    // Test sus4 (NOT YET IMPLEMENTED)
    Chord c_sus4("Csus4");
    std::vector<int> c_sus4_notes = c_sus4.get_notes(4);
    assert(c_sus4_notes.size() == 3);
    assert(c_sus4_notes[0] == 60);
    assert(c_sus4_notes[1] == 65); // F4
    assert(c_sus4_notes[2] == 67); // G4

    // Test NoteParser with Scale
    int p1 = NoteParser::parse("1", c_major, 4);
    assert(p1 == 60);
    
    int p3 = NoteParser::parse("3", c_major, 4);
    assert(p3 == 64);

    std::cout << "All music theory tests passed!" << std::endl;
    return 0;
}