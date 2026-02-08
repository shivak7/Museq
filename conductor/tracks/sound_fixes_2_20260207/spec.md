# Specification: Sound Fixes 2 - Synth Library Differentiation

## Overview
Currently, all `.museq` files within the `musynths/` directory (`drums.museq`, `leads.museq`, `pads.museq`, `sfx.museq`) contain identical code. This track aims to differentiate these files so that the synthesis parameters and instrument definitions in each file produce sounds characteristic of their filenames.

## Functional Requirements
- **`musynths/drums.museq`**: Implement drum synthesis templates. Focus on noise-based percussion (snare, hi-hats) and percussive oscillators with sharp pitch envelopes (kick).
- **`musynths/leads.museq`**: Implement lead synth templates. Use rich waveforms (Saw, Square), sharp attack envelopes, and modulation like vibrato or portamento.
- **`musynths/pads.museq`**: Implement ambient pad templates. Use soft waveforms (Sine, Triangle), slow ADSR envelopes (long attack and release), and heavy filtering or LFO-driven movement.
- **`musynths/sfx.museq`**: Implement sound effect templates. Utilize extreme modulation, pitch sweeps, and noise to create transitional or atmospheric sounds.
- **Instrument Templates**: Each file should define clear, reusable `instrument` blocks that users can reference or copy into their own scripts.

## Non-Functional Requirements
- **Characteristic Accuracy**: The presets should clearly represent their respective categories (e.g., a "pad" should not sound like a "kick drum").
- **Scripting Idioms**: Use standard Museq scripting conventions for definitions and blocks.

## Acceptance Criteria
- [ ] `drums.museq`, `leads.museq`, `pads.museq`, and `sfx.museq` no longer contain identical code.
- [ ] Each file contains at least 2-3 distinct instrument definitions matching its category.
- [ ] The files are syntactically correct and can be parsed by the Museq engine.
- [ ] A test script is provided to verify the audio output of these new synth definitions.

## Out of Scope
- Modification of the C++ synthesis engine or audio backend.
- Adding new keywords or features to the Museq language.
