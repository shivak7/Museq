#include <iostream>
#include <cassert>
#include "../muqomposer/SplashUtils.h"

int main() {
    // Test Case 1: 1920x1080 monitor, 1008x1008 image
    // Target width: 50% of 1920 = 960.
    // Target height: (scaled image height) + 100 padding.
    // Image is square, so scaled image height = 960.
    // Total height = 960 + 100 = 1060.
    
    auto dims = calculate_splash_dims(1920, 1080, 1008, 1008);
    std::cout << "1920 monitor: " << dims.width << "x" << dims.height << std::endl;
    
    assert(dims.width == 960);
    assert(dims.height == 1060);

    std::cout << "Splash scaling logic tests passed!" << std::endl;
    return 0;
}
