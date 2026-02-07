#include <iostream>
#include <cassert>
#include <filesystem>
#include "../muqomposer/AssetManager.h"

namespace fs = std::filesystem;

int main() {
    AssetManager manager;
    
    // Add something
    manager.add_watched_folder(".");
    assert(manager.get_watched_folders().size() > 0);
    
    // Clear
    manager.clear_watched_folders();
    assert(manager.get_watched_folders().size() == 0);
    assert(manager.get_samples().size() == 0);
    assert(manager.get_soundfonts().size() == 0);

    std::cout << "AssetClear test passed!" << std::endl;
    return 0;
}
