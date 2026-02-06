# Implementation Plan - Music Theory Features

## Phase 1: Core Data Structures and Scale Support [checkpoint: 4029469]
- [x] Task: Define Scale and Chord data structures in the engine 1751fe8
    - [x] Create `Scale.h` and `Chord.h`
    - [x] Implement scale degree to MIDI pitch conversion logic
- [x] Task: Update NoteParser for relative pitches 92dea1b
    - [x] Write unit tests for relative pitch parsing
    - [x] Implement scale-aware parsing in `NoteParser.cpp`
- [x] Task: Implement 'scale' keyword in ScriptParser f2596db
    - [x] Update `ScriptParser.cpp` to handle scale definitions
    - [x] Verify scale propagation through hierarchical blocks
- [x] Task: Conductor - User Manual Verification 'Phase 1: Scale Support' (Protocol in workflow.md) 4029469

## Phase 2: Chord Support and Integration [checkpoint: c8bfc13]
- [x] Task: Implement Chord expansion logic ebdf5d0
    - [x] Define built-in chord formulas (maj, min, aug, dim, 7, etc.)
    - [x] Write unit tests for chord expansion
- [x] Task: Update ScriptParser for chord names dd374d6
    - [x] Implement chord name recognition in compact note syntax
    - [x] Ensure chords work within `parallel` and `sequential` blocks
- [x] Task: Comprehensive Integration Testing d5df2d1
    - [x] Create test scripts using combined scales and chords
    - [x] Verify correct rendering to WAV/MP3/OGG
- [x] Task: Conductor - User Manual Verification 'Phase 2: Chord Support' (Protocol in workflow.md) c8bfc13
