#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>

// Mocking the logic I intend to implement in main.cpp or a helper class

struct AssetInsertion {
    std::string type; // "soundfont" or "sample"
    std::string path;
    int bank = 0;
    int preset = 0;
};

// Helper function to check if asset is defined
bool is_asset_defined(const std::string& script, const AssetInsertion& asset) {
    std::stringstream ss(script);
    std::string line;
    bool inside_instrument = false;
    
    // Simple state machine to parse instrument blocks
    // This mimics a lightweight parser
    
    std::string current_inst_type;
    std::string current_inst_path;
    int current_inst_bank = -1;
    int current_inst_preset = -1;
    
    while (std::getline(ss, line)) {
        if (line.find("instrument ") != std::string::npos && line.find("{") != std::string::npos) {
            inside_instrument = true;
            current_inst_type = "";
            current_inst_path = "";
            current_inst_bank = -1;
            current_inst_preset = -1;
            continue;
        }
        
        if (inside_instrument) {
            if (line.find("}") != std::string::npos) {
                inside_instrument = false;
                // End of instrument, check match
                if (asset.type == "soundfont") {
                    if (current_inst_type == "soundfont" && 
                        current_inst_path == asset.path &&
                        current_inst_bank == asset.bank &&
                        current_inst_preset == asset.preset) {
                        return true;
                    }
                } else if (asset.type == "sample") {
                    if (current_inst_type == "sample" && 
                        current_inst_path == asset.path) {
                        return true;
                    }
                }
                continue;
            }
            
            // Parse fields
            if (line.find("soundfont ") != std::string::npos) {
                size_t start = line.find(""") + 1;
                size_t end = line.find(""", start);
                if (start != std::string::npos && end != std::string::npos) {
                    current_inst_path = line.substr(start, end - start);
                    current_inst_type = "soundfont";
                }
            }
            else if (line.find("sample ") != std::string::npos) {
                size_t start = line.find(""") + 1;
                size_t end = line.find(""", start);
                if (start != std::string::npos && end != std::string::npos) {
                    current_inst_path = line.substr(start, end - start);
                    current_inst_type = "sample";
                }
            }
            else if (line.find("bank ") != std::string::npos) {
                std::stringstream ls(line);
                std::string temp;
                int val;
                ls >> temp >> val; // Assuming "    bank 0" format
                current_inst_bank = val;
            }
            else if (line.find("preset ") != std::string::npos) {
                std::stringstream ls(line);
                std::string temp;
                int val;
                ls >> temp >> val;
                current_inst_preset = val;
            }
        }
    }
    
    return false;
}

int main() {
    // Test 1: Empty script
    assert(is_asset_defined("", {"sample", "test.wav"}) == false);
    
    // Test 2: Sample match
    std::string script_sample = 
        "instrument Kick {
"
        "    sample "sounds/kick.wav"
"
        "}";
    assert(is_asset_defined(script_sample, {"sample", "sounds/kick.wav"}) == true);
    assert(is_asset_defined(script_sample, {"sample", "sounds/snare.wav"}) == false);
    
    // Test 3: SoundFont match
    std::string script_sf = 
        "instrument Piano {
"
        "    soundfont "sounds/gm.sf2"
"
        "    bank 0
"
        "    preset 1
"
        "}";
    
    assert(is_asset_defined(script_sf, {"soundfont", "sounds/gm.sf2", 0, 1}) == true);
    assert(is_asset_defined(script_sf, {"soundfont", "sounds/gm.sf2", 0, 2}) == false); // Diff preset
    assert(is_asset_defined(script_sf, {"soundfont", "sounds/other.sf2", 0, 1}) == false); // Diff file

    std::cout << "Smart Insertion logic tests passed!" << std::endl;
    return 0;
}
