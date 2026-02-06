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
                notes 1, 3, 5
            }
        }
        
        sequential {
            TestInst
        }
    )";

    Song song = ScriptParser::parse_string(script);
    Sequence seq = get_first_sequence(song);

    std::cout << "Sequence size: " << seq.notes.size() << std::endl;
    for (const auto& n : seq.notes) {
        std::cout << "Note: " << n.pitch << std::endl;
    }

    assert(seq.notes.size() == 3);
    
    // C Major: 1=C, 3=E, 5=G. Default octave 4.
    // C4=60, E4=64, G4=67.
    assert(seq.notes[0].pitch == 60);
    assert(seq.notes[1].pitch == 64);
    assert(seq.notes[2].pitch == 67);

    std::cout << "Script Scale test passed!" << std::endl;
    return 0;
}