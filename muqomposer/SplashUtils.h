#ifndef SPLASH_UTILS_H
#define SPLASH_UTILS_H

struct SplashDimensions {
    int width;
    int height;
};

inline SplashDimensions calculate_splash_dims(int monitor_w, int monitor_h, int img_w, int img_h) {
    if (img_w <= 0) return {0, 0};
    float scale = (monitor_w * 0.5f) / img_w;
    int final_w = static_cast<int>(img_w * scale);
    int final_h = static_cast<int>(img_h * scale);
    
    // Add vertical padding for text
    final_h += 100;
    
    return {final_w, final_h};
}

#endif
