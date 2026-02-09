#include <iostream>
#include <cassert>
#include "../src/ScriptParser.h"
#include "../src/SongElement.h"

int main() {
    std::cout << "Testing Sequence Keyword Feature..." << std::endl;

    std::string script = R"(
        instrument Kick { waveform sine }
        
        sequence Beat {
            0.0 Kick
            1.0 Kick
        }
        
        Beat
    )";

    Song song = ScriptParser::parse_string(script);
    
    if (song.root->children.empty()) {
        std::cerr << "Error: Song root has no children." << std::endl;
        return 1;
    }

    // Root -> Beat Call (PARALLEL Container)
    auto beat_container = std::dynamic_pointer_cast<CompositeElement>(song.root->children[0]);
    if (!beat_container || beat_container->type != CompositeType::PARALLEL) {
        std::cerr << "Error: Expected PARALLEL container for sequence call." << std::endl;
        return 1;
    }
    
    if (beat_container->children.size() != 2) {
        std::cerr << "Error: Expected 2 children in Beat sequence, got " << beat_container->children.size() << std::endl;
        return 1;
    }

    auto kick1 = std::dynamic_pointer_cast<InstrumentElement>(beat_container->children[0]);
    auto kick2 = std::dynamic_pointer_cast<InstrumentElement>(beat_container->children[1]);

    if (!kick1 || !kick2) {
        std::cerr << "Error: Children are not InstrumentElements." << std::endl;
        return 1;
    }

    std::cout << "Kick 1 Offset: " << kick1->start_offset_ms << " (Expected 0)" << std::endl;
    std::cout << "Kick 2 Offset: " << kick2->start_offset_ms << " (Expected 500)" << std::endl;

    if (kick1->start_offset_ms != 0) {
        std::cerr << "FAILURE: Kick 1 offset incorrect." << std::endl;
        return 1;
    }
    // Default 120 BPM -> 500ms per beat. 1.0 beat = 500ms.
    if (kick2->start_offset_ms != 500) {
        std::cerr << "FAILURE: Kick 2 offset incorrect." << std::endl;
        return 1;
    }

    std::cout << "SUCCESS: Sequence keyword and timestamps working." << std::endl;
    return 0;
}
