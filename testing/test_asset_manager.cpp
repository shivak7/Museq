#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include "../muqomposer/AssetManager.h"

namespace fs = std::filesystem;

void create_dummy_file(const std::string& path) {
    std::ofstream f(path);
    f << "dummy";
    f.close();
}

int main() {
    // Setup test env
    if (fs::exists("test_assets")) fs::remove_all("test_assets");
    fs::create_directory("test_assets");
    fs::create_directory("test_assets/sub");
    
    create_dummy_file("test_assets/sample.wav");
    create_dummy_file("test_assets/sub/loop.ogg");
    create_dummy_file("test_assets/ignore.txt");

    // Test Manager
    AssetManager manager;
    manager.add_watched_folder("test_assets");
    
    manager.refresh_assets();
    
    // Check samples
    const auto& samples = manager.get_samples();
    
    bool found_wav = false;
    bool found_ogg = false;
    
    for (const auto& s : samples) {
        if (s.find("test_assets/sample.wav") != std::string::npos) found_wav = true;
        if (s.find("test_assets/sub/loop.ogg") != std::string::npos) found_ogg = true;
    }
    
    assert(found_wav);
    assert(found_ogg);

    // Cleanup
    fs::remove_all("test_assets");

    std::cout << "AssetManager test passed!" << std::endl;
    return 0;
}
