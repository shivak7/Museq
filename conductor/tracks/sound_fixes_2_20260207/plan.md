# Implementation Plan - Sound Fixes 2 (Synth Library Differentiation)

This plan outlines the steps to differentiate the `.museq` files in the `musynths/` directory, providing distinct instrument templates for drums, leads, pads, and sound effects.

## Phase 1: Research and Template Design [checkpoint: 05a19dd]
- [x] Task: Analyze current synth engine capabilities (Waveforms, ADSR, LFO, Filters) by reading `src/Waveform.h`, `src/AdsrEnvelope.h`, and `src/Instrument.h`.
- [x] Task: Design 2-3 specific instrument configurations for each category (Drums, Leads, Pads, SFX).
- [x] Task: Conductor - User Manual Verification 'Research and Template Design' (Protocol in workflow.md)

## Phase 2: Implementation - Drum Synths
- [x] Task: Write a test script `testing/test_synth_drums.cpp` or a `.museq` test file that attempts to play drum-like sounds from the library. [fe3fec0]
- [x] Task: Implement `musynths/drums.museq` with templates for:
    - Kick (Pitch envelope, sine/triangle)
    - Snare/Hi-hat (Noise source, short decay)
- [ ] Task: Conductor - User Manual Verification 'Implementation - Drum Synths' (Protocol in workflow.md)

## Phase 3: Implementation - Lead Synths
- [x] Task: Write a test script or `.museq` file for verifying lead synth sounds. [79fcfe9]
- [x] Task: Implement `musynths/leads.museq` with templates for:
    - Classic Saw Lead (Sawtooth, vibrato)
    - Square/Pulse Lead (Square, PWM-like LFO)
- [ ] Task: Conductor - User Manual Verification 'Implementation - Lead Synths' (Protocol in workflow.md)

## Phase 4: Implementation - Pad Synths
- [ ] Task: Write a test script or `.museq` file for verifying pad synth sounds.
- [ ] Task: Implement `musynths/pads.museq` with templates for:
    - Ambient Sine Pad (Slow attack, sine, low-pass filter)
    - Ethereal Texture (Multiple oscillators, LFO on filter cutoff)
- [ ] Task: Conductor - User Manual Verification 'Implementation - Pad Synths' (Protocol in workflow.md)

## Phase 5: Implementation - SFX Synths
- [ ] Task: Write a test script or `.museq` file for verifying SFX synth sounds.
- [ ] Task: Implement `musynths/sfx.museq` with templates for:
    - Riser/Sweep (LFO on pitch/filter over long duration)
    - Noise Burst/Impact (Noise with complex envelope)
- [ ] Task: Conductor - User Manual Verification 'Implementation - SFX Synths' (Protocol in workflow.md)

## Phase 6: Final Verification
- [ ] Task: Ensure all files in `musynths/` are syntactically valid by running them through the `museq` CLI or `muqomposer`.
- [ ] Task: Verify that none of the files are identical using `diff`.
- [ ] Task: Conductor - User Manual Verification 'Final Verification' (Protocol in workflow.md)
