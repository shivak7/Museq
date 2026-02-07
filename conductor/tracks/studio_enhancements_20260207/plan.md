# Implementation Plan - Studio Functionality and UI Enhancements

## Phase 1: Build System and Splash Polish [checkpoint: 7f73ccb]
- [x] Task: Configure Test Build Option [63bc09f]
    - [x] Update `CMakeLists.txt` to add `option(BUILD_TESTS "Build test suite" OFF)`
    - [x] Wrap all `add_executable` calls for tests in `if(BUILD_TESTS)`
- [x] Task: Refine Splash Scaling [01df4fd]
    - [x] Update `muqomposer/SplashUtils.h` to calculate `final_h` as 60% of `monitor_h` + 100px padding
    - [x] Recalculate `final_w` to maintain aspect ratio based on the new height
- [x] Task: Conductor - User Manual Verification 'Phase 1: Build & Splash' (Protocol in workflow.md) 7f73ccb

## Phase 2: Editor Ergonomics and Code Insertion [checkpoint: 808260b]
- [x] Task: Implement Auto-Indent [808260b]
    - [x] Add `ImGuiInputTextFlags_EnterReturnsTrue` to script editor
    - [x] Calculate indentation of the previous line and insert it automatically on new line
- [x] Task: Add Line Numbers [808260b]
    - [x] Render a child window or column to the left of the editor displaying line indices
- [x] Task: Update Smart Insertion Logic [808260b]
    - [x] Modify `main.cpp` instrument addition logic to prepend code to the start of `script_buffer` instead of appending
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Editor' (Protocol in workflow.md) 808260b

## Phase 3: Asset Search and Shortcuts
- [ ] Task: Implement Global Asset Search
    - [ ] Add search filtering logic to `AssetManager` or sidebar render loop
    - [ ] Recursively filter tree nodes based on the search query
- [ ] Task: Implement Keyboard Shortcuts
    - [ ] Add `if (io.KeyCtrl && ImGui::IsKeyPressed(...))` checks in the main loop for P, S, L, E, Space
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Assets & Keys' (Protocol in workflow.md)

## Phase 4: Export Workflow
- [ ] Task: Create Export Dialog
    - [ ] Create a modal popup "Export Song" with format dropdown (WAV, MP3, OGG)
    - [ ] Add "Export" button to the transport toolbar
- [ ] Task: Integrate Audio Writers
    - [ ] Link `Mp3Writer`, `OggWriter`, `WavWriter` to the UI logic
    - [ ] Implement the `AudioRenderer::render` to file pipeline in the GUI thread (or background thread with progress)
- [ ] Task: Conductor - User Manual Verification 'Phase 4: Export' (Protocol in workflow.md)
