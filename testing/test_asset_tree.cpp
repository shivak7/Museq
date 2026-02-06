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

void print_tree(const AssetNode& node, int depth = 0) {
    for (int i = 0; i < depth; ++i) std::cout << "  ";
    std::cout << node.name << (node.is_directory ? "/" : "") << std::endl;
    for (const auto& child : node.children) {
        print_tree(child, depth + 1);
    }
}

int main() {
    // Setup test env
    if (fs::exists("tree_test")) fs::remove_all("tree_test");
    fs::create_directories("tree_test/A/B");
    
    create_dummy_file("tree_test/root.wav");
    
    // Copy real SF2 if available, otherwise use dummy (which will fail load but we can check logic if we mock TSF, but we aren't mocking)
    // If no real SF2, we can't test SF2 tree population with current implementation.
    // We'll skip SF2 test if copy fails.
    bool sf2_test_possible = false;
    if (fs::exists("../sounds/GeneralUser GS v1.471.sf2")) {
        fs::copy_file("../sounds/GeneralUser GS v1.471.sf2", "tree_test/A/a.sf2");
        sf2_test_possible = true;
    } else {
        std::cout << "Warning: No real SF2 found, skipping SF2 tree test." << std::endl;
    }

    create_dummy_file("tree_test/A/B/b.wav");
    create_dummy_file("tree_test/A/B/ignore.txt");

    // Test Manager
    AssetManager manager;
    manager.add_watched_folder("tree_test");
    manager.refresh_assets();

    // Check Sample Tree
    AssetNode sample_root = manager.get_sample_tree();
    std::cout << "Sample Tree:" << std::endl;
    print_tree(sample_root);

    bool found_tree_test = false;
    bool found_root_wav = false;
    bool found_b_wav = false;
    bool found_a_sf2 = false;

    // Find 'tree_test' root
    const AssetNode* tree_test_node = nullptr;
    for (const auto& child : sample_root.children) {
        if (child.name == "tree_test") {
            tree_test_node = &child;
            found_tree_test = true;
            break;
        }
    }
    assert(found_tree_test);

    // Check content of tree_test
    for (const auto& child : tree_test_node->children) {
        if (child.name == "root.wav") found_root_wav = true;
        if (child.name == "A") {
            for (const auto& grandchild : child.children) {
                if (grandchild.name == "B") {
                    for (const auto& ggrandchild : grandchild.children) {
                        if (ggrandchild.name == "b.wav") found_b_wav = true;
                    }
                }
                if (grandchild.name == "a.sf2") found_a_sf2 = true; // Should NOT be in sample tree
            }
        }
    }

    assert(found_root_wav);
    assert(found_b_wav);
    assert(!found_a_sf2); // SF2 should not be in sample tree

    // Check SF2 Tree
    if (sf2_test_possible) {
        AssetNode sf2_root = manager.get_soundfont_tree();
        std::cout << "SF2 Tree:" << std::endl;
        print_tree(sf2_root);

        // Find 'tree_test' root for SF2
        const AssetNode* sf2_tree_test_node = nullptr;
        for (const auto& child : sf2_root.children) {
            if (child.name == "tree_test") {
                sf2_tree_test_node = &child;
                break;
            }
        }
        
        assert(sf2_tree_test_node != nullptr);

        bool found_sf2 = false;
        for (const auto& child : sf2_tree_test_node->children) {
            if (child.name == "A") {
                for (const auto& grandchild : child.children) {
                    if (grandchild.name == "a.sf2") found_sf2 = true;
                }
            }
        }
        assert(found_sf2);
    }

    // Cleanup
    fs::remove_all("tree_test");

    std::cout << "AssetTree test passed!" << std::endl;
    return 0;
}
