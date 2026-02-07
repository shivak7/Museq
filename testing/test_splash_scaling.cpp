#include <iostream>
#include <cassert>
#include "../muqomposer/SplashUtils.h"

int main() {
    // Test Case 1: 1920x1080 monitor, 1008x1008 image
    auto dims = calculate_splash_dims(1920, 1080, 1008, 1008);
    std::cout << "1920 monitor: " << dims.width << "x" << dims.height << std::endl;
    assert(dims.width == 960);
    assert(dims.height == 960);

    // Test Case 2: 4K monitor
    dims = calculate_splash_dims(3840, 2160, 1008, 1008);
    std::cout << "3840 monitor: " << dims.width << "x" << dims.height << std::endl;
    assert(dims.width == 1920);
    assert(dims.height == 1920);

    std::cout << "Splash scaling logic tests passed!" << std::endl;
    return 0;
}