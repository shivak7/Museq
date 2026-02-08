#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "../src/ScriptParser.h"

int main() {
    // Case 1: Simple float in duration
    std::string script1 = "instrument Test { waveform sine }\nTest { notes C(500.5) }";
    std::cout << "Testing Case 1: notes C(500.5)" << std::endl;
    ScriptParser::parse_string(script1);
    
    // Case 2: Float in velocity
    std::string script2 = "instrument Test { waveform sine }\nTest { notes C(500, 100.5) }";
    std::cout << "Testing Case 2: notes C(500, 100.5)" << std::endl;
    ScriptParser::parse_string(script2);

    // Case 3: Float in note name
    std::string script3 = "instrument Test { waveform sine }\nTest { notes C4.5(500) }";
    std::cout << "Testing Case 3: notes C4.5(500)" << std::endl;
    ScriptParser::parse_string(script3);

    // Case 4: Garbage in parentheses
    std::string script4 = "instrument Test { waveform sine }\nTest { notes C(abc) }";
    std::cout << "Testing Case 4: notes C(abc)" << std::endl;
    ScriptParser::parse_string(script4);

    // Case 5: Float in 'note' keyword
    std::string script5 = "instrument Test { waveform sine }\nTest { note C4 500.5 100 }";
    std::cout << "Testing Case 5: note C4 500.5 100" << std::endl;
    ScriptParser::parse_string(script5);

    std::cout << "All test cases passed (no crash)." << std::endl;
    return 0;
}