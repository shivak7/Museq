#include <iostream>
#include <cassert>
#include <cmath>
#include "../src/ScriptParser.h"
#include "../src/Song.h"
#include "../src/SongElement.h"

void test_duration_notation() {
    std::cout << "Testing Musical Denominator Duration Notation..." << std::endl;

    // Test Case 1: Standard Quarter Note at 120 BPM
    {
        std::cout << "  Running Test Case 1..." << std::endl;
        ScriptParser::set_global_bpm(120);
        std::string script = "instrument Test {\n  waveform sine\n}\nTest {\n  notes C4_4\n}";
        Song song = ScriptParser::parse_string(script);
        assert(song.root != nullptr);
        std::cout << "    Song root OK. Children size: " << song.root->children.size() << std::endl;
        assert(song.root->children.size() > 0);
        auto inst_elem = std::dynamic_pointer_cast<InstrumentElement>(song.root->children[0]);
        assert(inst_elem != nullptr);
        assert(!inst_elem->instrument.sequence.notes.empty());
        int dur = inst_elem->instrument.sequence.notes[0].duration;
        std::cout << "    120 BPM, C4_4: " << dur << "ms (Expected: 500ms)" << std::endl;
        assert(dur == 500);
    }

    // Test Case 2: Whole Note at 60 BPM
    {
        ScriptParser::set_global_bpm(60);
        std::string script = "instrument Test {\n waveform sine\n }\n Test {\n notes C4_1\n }";
        Song song = ScriptParser::parse_string(script);
        int dur = std::dynamic_pointer_cast<InstrumentElement>(song.root->children[0])->instrument.sequence.notes[0].duration;
        std::cout << "  60 BPM, C4_1: " << dur << "ms (Expected: 4000ms)" << std::endl;
        assert(dur == 4000);
    }

    // Test Case 3: Quarter-note Triplet at 120 BPM
    {
        ScriptParser::set_global_bpm(120);
        std::string script = "instrument Test {\n waveform sine\n }\n Test {\n notes C4_6\n }";
        Song song = ScriptParser::parse_string(script);
        int dur = std::dynamic_pointer_cast<InstrumentElement>(song.root->children[0])->instrument.sequence.notes[0].duration;
        // Formula: (4/6) * (60000/120) = 0.666 * 500 = 333ms
        std::cout << "  120 BPM, C4_6: " << dur << "ms (Expected: 333ms)" << std::endl;
        assert(dur == 333);
    }

    // Test Case 4: Individual 'note' command with underscore
    {
        ScriptParser::set_global_bpm(120);
        std::string script = "instrument Test {\n waveform sine\n sequence {\n note C4_8 100\n }\n }\n Test";
        Song song = ScriptParser::parse_string(script);
        auto inst_elem = std::dynamic_pointer_cast<InstrumentElement>(song.root->children[0]);
        int dur = inst_elem->instrument.sequence.notes[0].duration;
        std::cout << "  120 BPM, note C4_8: " << dur << "ms (Expected: 250ms)" << std::endl;
        assert(dur == 250);
    }

    // Test Case 5: MIDI with underscore
    {
        ScriptParser::set_global_bpm(120);
        std::string script = "instrument Test {\n waveform sine\n }\n Test {\n notes 36_4\n }";
        Song song = ScriptParser::parse_string(script);
        int dur = std::dynamic_pointer_cast<InstrumentElement>(song.root->children[0])->instrument.sequence.notes[0].duration;
        int pitch = std::dynamic_pointer_cast<InstrumentElement>(song.root->children[0])->instrument.sequence.notes[0].pitch;
        std::cout << "  120 BPM, 36_4: " << dur << "ms, pitch " << pitch << " (Expected: 500ms, 36)" << std::endl;
        assert(dur == 500);
        assert(pitch == 36);
    }

    // Test Case 6: Rest with underscore
    {
        ScriptParser::set_global_bpm(120);
        std::string script = "instrument Test {\n waveform sine\n }\n Test {\n notes R_2\n }";
        Song song = ScriptParser::parse_string(script);
        auto note = std::dynamic_pointer_cast<InstrumentElement>(song.root->children[0])->instrument.sequence.notes[0];
        std::cout << "  120 BPM, R_2: " << note.duration << "ms, is_rest: " << note.is_rest << " (Expected: 1000ms, true)" << std::endl;
        assert(note.duration == 1000);
        assert(note.is_rest == true);
    }

    std::cout << "All Duration Notation tests passed!" << std::endl;
}

int main() {
    try {
        test_duration_notation();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
