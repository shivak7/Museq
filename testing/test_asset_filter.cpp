#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include "../muqomposer/AssetManager.h"

namespace fs = std::filesystem;

void create_dummy_file(const std::string& path) {
    fs::create_directories(fs::path(path).parent_path());
    std::ofstream f(path);
    f << "dummy";
    f.close();
}

bool tree_contains(const AssetNode& node, const std::string& name) {
    if (node.name == name) return true;
    for (const auto& child : node.children) {
        if (tree_contains(child, name)) return true;
    }
    return false;
}

int main() {
    // Setup test env
    if (fs::exists("filter_test")) fs::remove_all("filter_test");
    fs::create_directories("filter_test");
    
    create_dummy_file("filter_test/kick.wav");
    create_dummy_file("filter_test/snare.wav");
    create_dummy_file("filter_test/hihat.wav");

    // Test Manager
    AssetManager manager;
    manager.add_watched_folder("filter_test");
    manager.refresh_assets();

    // Test Filter
    // Should find 'kick.wav' but not 'snare.wav'
    AssetNode filtered_tree = manager.get_sample_tree("kick");
    
    assert(tree_contains(filtered_tree, "kick.wav"));
    assert(!tree_contains(filtered_tree, "snare.wav"));
    assert(!tree_contains(filtered_tree, "hihat.wav"));

    // Cleanup
    fs::remove_all("filter_test");

    std::cout << "AssetFilter test passed!" << std::endl;
    return 0;
}
