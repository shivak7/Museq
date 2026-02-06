# Implementation Plan - Music Theory Features

## Phase 1: Core Data Structures and Scale Support
- [x] Task: Define Scale and Chord data structures in the engine 1751fe8
    - [ ] Create `Scale.h` and `Chord.h`
    - [ ] Implement scale degree to MIDI pitch conversion logic
- [x] Task: Update NoteParser for relative pitches 92dea1b
    - [ ] Write unit tests for relative pitch parsing
    - [ ] Implement scale-aware parsing in `NoteParser.cpp`
- [ ] Task: Implement 'scale' keyword in ScriptParser
    - [ ] Update `ScriptParser.cpp` to handle scale definitions
    - [ ] Verify scale propagation through hierarchical blocks
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Scale Support' (Protocol in workflow.md)

## Phase 2: Chord Support and Integration
- [ ] Task: Implement Chord expansion logic
    - [ ] Define built-in chord formulas (maj, min, aug, dim, 7, etc.)
    - [ ] Write unit tests for chord expansion
- [ ] Task: Update ScriptParser for chord names
    - [ ] Implement chord name recognition in compact note syntax
    - [ ] Ensure chords work within `parallel` and `sequential` blocks
- [ ] Task: Comprehensive Integration Testing
    - [ ] Create test scripts using combined scales and chords
    - [ ] Verify correct rendering to WAV/MP3/OGG
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Chord Support' (Protocol in workflow.md)
