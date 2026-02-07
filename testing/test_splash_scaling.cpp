#include <iostream>
#include <cassert>
#include "../muqomposer/SplashUtils.h"

int main() {
    // Test Case 1: 1920x1080 monitor, 1008x1008 image
    // Target Height (Image): 60% of 1080 = 648.
    // Window Height: 648 + 100 = 748.
    // Window Width: 648.
    
    auto dims = calculate_splash_dims(1920, 1080, 1008, 1008);
    std::cout << "1920x1080 monitor: " << dims.width << "x" << dims.height << std::endl;
    
    assert(dims.width == 648);
    assert(dims.height == 748);

    std::cout << "Splash scaling logic tests passed!" << std::endl;
    return 0;
}
