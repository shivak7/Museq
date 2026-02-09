#include <iostream>
#include <cassert>
#include "../src/ScriptParser.h"
#include "../src/SongElement.h"

int main() {
    std::cout << "Testing Tempo Consistency..." << std::endl;

    // Set initial global BPM to 120
    ScriptParser::set_global_bpm(120);

    // Script changes tempo to 60.
    // At 60 BPM, a quarter note (_4) should be 1000ms.
    // At 120 BPM, it would be 500ms.
    std::string script = R"(
        tempo 60
        instrument Test {
            waveform sine
        }
        sequential {
            Test {
                notes C4_4
            }
        }
    )";

    Song song = ScriptParser::parse_string(script);
    
    if (song.root->children.empty()) {
        std::cerr << "Error: Song root has no children." << std::endl;
        return 1;
    }

    auto seq = std::dynamic_pointer_cast<CompositeElement>(song.root->children[0]);
    if (!seq || seq->children.empty()) {
         std::cerr << "Error: Sequential block empty." << std::endl;
         return 1;
    }

    auto inst = std::dynamic_pointer_cast<InstrumentElement>(seq->children[0]);
    if (!inst || inst->instrument.sequence.notes.empty()) {
        std::cerr << "Error: Instrument or sequence empty." << std::endl;
        return 1;
    }

    int duration = inst->instrument.sequence.notes[0].duration;
    std::cout << "Duration at 60 BPM (expected 1000): " << duration << std::endl;

    if (duration != 1000) {
        std::cerr << "FAILURE: Duration was " << duration << " instead of 1000." << std::endl;
        return 1;
    }

    std::cout << "SUCCESS: Tempo change respected." << std::endl;
    return 0;
}
