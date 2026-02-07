#include <iostream>
#include <cassert>
#include "../muqomposer/SplashUtils.h"

int main() {
    // Test Case 1: 1920x1080 monitor, 1008x1008 image
    // Target Height (Image): 50% of 1080 = 540.
    // Target Width (Image): 1008 * (540/1008) = 540.
    // Window Height: 540 + 100 = 640.
    // Window Width: 540.
    
    auto dims = calculate_splash_dims(1920, 1080, 1008, 1008);
    std::cout << "1920x1080 monitor: " << dims.width << "x" << dims.height << std::endl;
    
    assert(dims.width == 540);
    assert(dims.height == 640);

    std::cout << "Splash scaling logic tests passed!" << std::endl;
    return 0;
}