#ifndef ADSR_ENVELOPE_H
#define ADSR_ENVELOPE_H

class AdsrEnvelope {
public:
    float attack;
    float decay;
    float sustain;
    float release;

    AdsrEnvelope(float attack = 0.1f, float decay = 0.1f, float sustain = 0.8f, float release = 0.2f);
};

#endif // ADSR_ENVELOPE_H
