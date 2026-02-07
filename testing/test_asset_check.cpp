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
    assert(AssetManager::check_asset_exists_in_script(script_sf, "soundfont", "sounds/gm.sf2", 0, 2) == false);
    
    // Test 4: Path normalization match (Forward vs Backward slashes)
    assert(AssetManager::check_asset_exists_in_script(script_sf, "soundfont", "sounds\\gm.sf2", 0, 1) == true);

    // Test 5: Instrument with spaces in name (Checking if existence check still works)
    std::string script_space = 
        "instrument Grand Piano {\n"
        "    soundfont \"sounds/gm.sf2\"\n"
        "    bank 0\n"
        "    preset 0\n"
        "}";
    assert(AssetManager::check_asset_exists_in_script(script_space, "soundfont", "sounds/gm.sf2", 0, 0) == true);

    std::cout << "Asset check integration test passed!" << std::endl;
    return 0;
}
