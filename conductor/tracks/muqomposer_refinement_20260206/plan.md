# Implementation Plan - Muqomposer Features and Bugfixes

## Phase 1: Branding and Basic UI Polish [checkpoint: b911c75]
- [x] Task: Integrate Window Icons and Splash Screen
    - [x] Implement dark splash screen logic in `muqomposer/main.cpp` using inverted logo colors
    - [x] Set application window icon to `src/Museq.rc` (Windows) and `Museq_logo.png` (Linux/macOS)
- [x] Task: Implement 'Clear All' for Assets
    - [x] Add `clear_watched_folders()` to `AssetManager`
    - [x] Add "Clear All" button to the sidebar UI
- [x] Task: Conductor - User Manual Verification 'Phase 1: UI Polish' (Protocol in workflow.md) b911c75

## Phase 2: Asset Logic and Collision Handling
- [ ] Task: Implement Smart Script Insertion
    - [ ] Write unit tests for `ScriptParser` to check for existing instrument definitions
    - [ ] Update `main.cpp` click/drop logic to skip insertion if the asset path is already defined
- [ ] Task: Implement Automated Conflict Resolution
    - [ ] Write unit tests for name incrementation logic (e.g., `Piano` -> `Piano_1`)
    - [ ] Implement `get_unique_instrument_name` helper in `AssetManager` or `main.cpp`
    - [ ] Integrate helper into the script insertion flow
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Collision Handling' (Protocol in workflow.md)

## Phase 3: Enhanced Previews and Visualization
- [ ] Task: Implement Audio Preview Safety Fade
    - [ ] Update `AudioPlayer` to handle a 5-second hard cap for previews
    - [ ] Implement real-time volume ramp-down starting at 4 seconds in the audio callback
- [ ] Task: Expand Preview Support
    - [ ] Add preview functionality for Synth templates in the sidebar
    - [ ] Add preview functionality for individual Sample files
- [ ] Task: Implement FFT Spectrum Analyzer
    - [ ] Integrate a simple FFT implementation (or use a lightweight library if available)
    - [ ] Add `get_spectrum_data` to `AudioPlayer`
    - [ ] Create an ImGui-based Spectrum visualizer widget in the output panel
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Visualization' (Protocol in workflow.md)

## Phase 4: Transport Toolbar and Final Polish
- [ ] Task: Refactor Transport Toolbar
    - [ ] Redesign footer into a docked toolbar with Play/Stop/Save icons
    - [ ] Add interactive BPM slider and input field
- [ ] Task: Conductor - User Manual Verification 'Phase 4: Final Polish' (Protocol in workflow.md)
