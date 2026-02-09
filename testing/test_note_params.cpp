#include <iostream>
#include <cassert>
#include "../src/ScriptParser.h"
#include "../src/SongElement.h"

int main() {
    std::cout << "Testing Note Params..." << std::endl;

    std::string script = R"(
        instrument Test { waveform sine }
        
        sequential {
            Test {
                note C4(1000, 50, 0.8)
                note D4_2(200) // Override denominator
            }
        }
    )";

    Song song = ScriptParser::parse_string(script);
    
    auto seq = std::dynamic_pointer_cast<CompositeElement>(song.root->children[0]);
    auto inst = std::dynamic_pointer_cast<InstrumentElement>(seq->children[0]);
    
    auto& notes = inst->instrument.sequence.notes;
    
    if (notes.size() != 2) {
         std::cerr << "Expected 2 notes, got " << notes.size() << std::endl;
         return 1;
    }
    
    // Note 1
    if (notes[0].duration != 1000) {
        std::cerr << "Note 1 duration error: " << notes[0].duration << std::endl;
        return 1;
    }
    if (notes[0].velocity != 50) {
        std::cerr << "Note 1 velocity error: " << notes[0].velocity << std::endl;
        return 1;
    }
    // Float comparison with epsilon
    if (std::abs(notes[0].pan - 0.8f) > 0.001f) {
        std::cerr << "Note 1 pan error: " << notes[0].pan << std::endl;
        return 1;
    }
    
    // Note 2
    // D4_2 is 1/2 note (1000ms at 120bpm), but (200) overrides it.
    if (notes[1].duration != 200) {
        std::cerr << "Note 2 duration error: " << notes[1].duration << " (Expected 200 override)" << std::endl;
        return 1;
    }

    std::cout << "SUCCESS: Note params parsed correctly." << std::endl;
    return 0;
}
