#include <iostream>
#include <cassert>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

namespace fs = std::filesystem;

int main() {
    const char* logo_path = "../Museq_logo.png"; // Relative to build/bin usually? Or project root?
    // Tests are run from build/bin/test_... usually? 
    // I will try a few paths.
    
    std::string paths[] = {
        "../Museq_logo.png",
        "Museq_logo.png",
        "../../Museq_logo.png"
    };

    bool found = false;
    std::string valid_path;

    for (const auto& p : paths) {
        if (fs::exists(p)) {
            found = true;
            valid_path = p;
            break;
        }
    }

    if (!found) {
        std::cerr << "Museq_logo.png not found!" << std::endl;
        return 1;
    }

    int width, height, channels;
    unsigned char* data = stbi_load(valid_path.c_str(), &width, &height, &channels, 4); // Force RGBA
    
    assert(data != nullptr);
    assert(width > 0);
    assert(height > 0);
    assert(channels == 4 || channels == 3); // We requested 4, so stbi should give 4 if possible

    stbi_image_free(data);

    std::cout << "GUI Resources test passed! Loaded " << valid_path << " (" << width << "x" << height << ")" << std::endl;
    return 0;
}
