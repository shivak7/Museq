#include <iostream>
#include <cassert>
#include "../src/ScriptParser.h"
#include "../src/SongElement.h"
#include "../src/Instrument.h"

// Helper to find the first instrument's sequence
Sequence get_first_sequence(const Song& song) {
    if (!song.root) return Sequence();
    // Recursively search for InstrumentElement
    std::vector<std::shared_ptr<SongElement>> stack;
    stack.push_back(song.root);
    
    while (!stack.empty()) {
        auto elem = stack.back();
        stack.pop_back();
        
        if (auto inst = std::dynamic_pointer_cast<InstrumentElement>(elem)) {
            return inst->instrument.sequence;
        }
        if (auto comp = std::dynamic_pointer_cast<CompositeElement>(elem)) {
            for (auto child : comp->children) {
                stack.push_back(child);
            }
        }
    }
    return Sequence();
}

int main() {
    std::string script = R"(
        scale C major
        
        instrument TestInst {
            waveform sine
            sequence {
                notes 1, 3, 5, Cmaj, Dmin7
            }
        }
        
        sequential {
            TestInst
        }
    )";

    Song song = ScriptParser::parse_string(script);
    Sequence seq = get_first_sequence(song);

    std::cout << "Sequence size: " << seq.notes.size() << std::endl;
    for (size_t i = 0; i < seq.notes.size(); ++i) {
        std::cout << "Note [" << i << "]: " << seq.notes[i].pitch << " (Advance time: " << (seq.notes[i].advance_time ? "Yes" : "No") << ")" << std::endl;
    }

    // 3 relative notes + 3 notes for Cmaj + 4 notes for Dmin7 = 10 notes
    assert(seq.notes.size() == 10);
    
    // C Major: 1=60, 3=64, 5=67
    assert(seq.notes[0].pitch == 60);
    assert(seq.notes[1].pitch == 64);
    assert(seq.notes[2].pitch == 67);

    // Cmaj: 60, 64, 67
    assert(seq.notes[3].pitch == 60);
    assert(seq.notes[4].pitch == 64);
    assert(seq.notes[5].pitch == 67);

    // Dmin7: 62, 65, 69, 72
    assert(seq.notes[6].pitch == 62);
    assert(seq.notes[7].pitch == 65);
    assert(seq.notes[8].pitch == 69);
    assert(seq.notes[9].pitch == 72);

    std::cout << "Script Scale & Chord test passed!" << std::endl;
    return 0;
}
