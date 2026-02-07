#include <iostream>
#include <cassert>
#include <string>
#include "../muqomposer/AssetManager.h"

int main() {
    // Test 1: Empty script
    assert(AssetManager::check_asset_exists_in_script("", "sample", "test.wav") == false);
    
    // Test 2: Sample match
    std::string script_sample = 
        "instrument Kick {\n"
        "    sample \"sounds/kick.wav\"\n"
        "}";
    assert(AssetManager::check_asset_exists_in_script(script_sample, "sample", "sounds/kick.wav") == true);
    assert(AssetManager::check_asset_exists_in_script(script_sample, "sample", "sounds/snare.wav") == false);
    
    // Test 3: SoundFont match
    std::string script_sf = 
        "instrument Piano {\n"
        "    soundfont \"sounds/gm.sf2\"\n"
        "    bank 0\n"
        "    preset 1\n"
        "}";
    
    assert(AssetManager::check_asset_exists_in_script(script_sf, "soundfont", "sounds/gm.sf2", 0, 1) == true);
    assert(AssetManager::check_asset_exists_in_script(script_sf, "soundfont", "sounds/gm.sf2", 0, 2) == false); // Diff preset
    assert(AssetManager::check_asset_exists_in_script(script_sf, "soundfont", "sounds/other.sf2", 0, 1) == false); // Diff file

    std::cout << "Asset check integration test passed!" << std::endl;
    return 0;
}