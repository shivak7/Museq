#include "AdsrEnvelope.h"

AdsrEnvelope::AdsrEnvelope(float attack, float decay, float sustain, float release) {
    this->attack = attack;
    this->decay = decay;
    this->sustain = sustain;
    this->release = release;
}
