#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include "../muqomposer/AssetManager.h"

namespace fs = std::filesystem;

void create_dummy_museq(const std::string& path) {
    std::ofstream f(path);
    f << "instrument MyKick {\n    waveform sine\n}\n\n";
    f << "instrument MySnare {\n    waveform noise\n}\n";
    f.close();
}

int main() {
    // Setup test env
    if (fs::exists("test_synths")) fs::remove_all("test_synths");
    fs::create_directory("test_synths");
    
    create_dummy_museq("test_synths/drums.museq");

    if(!fs::exists("test_synths/drums.museq")) {
        std::cerr << "File not created!" << std::endl;
        return 1;
    }

    // Test Manager
    AssetManager manager;
    manager.clear_watched_folders();
    manager.add_watched_folder("test_synths");
    manager.refresh_assets();
    
    // Check Synths
    const auto& synths = manager.get_synths(); 
    std::cout << "Found synths: " << synths.size() << std::endl;
    
    assert(synths.size() == 1);
    assert(synths[0].filename == "drums.museq");
    assert(synths[0].instruments.size() == 2);
    
    bool found_kick = false;
    bool found_snare = false;
    for(const auto& name : synths[0].instruments) {
        if(name == "MyKick") found_kick = true;
        if(name == "MySnare") found_snare = true;
    }
    assert(found_kick);
    assert(found_snare);

    // Cleanup
    fs::remove_all("test_synths");

    std::cout << "AssetManager Synths test passed!" << std::endl;
    return 0;
}