#ifndef EFFECT_H
#define EFFECT_H

#include <vector>

enum class EffectType {
    NONE,
    DELAY,      // Echo
    DISTORTION, // Overdrive/Fuzz
    BITCRUSH,   // Glitch/Lo-fi
    FADE_IN,    // Volume ramp up
    FADE_OUT,   // Volume ramp down
    TREMOLO,    // Amplitude modulation
    REVERB      // Reverb
};

struct Effect {
    EffectType type = EffectType::NONE;
    float param1 = 0.0f;
    float param2 = 0.0f;
    float param3 = 0.0f;
    
    // Helper constructors could be added here
};

#endif // EFFECT_H
