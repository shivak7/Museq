# Implementation Plan - Studio Asset Management Revision

This plan outlines the steps to refactor the `AssetManager` and `muqomposer` UI to support dynamic synth template discovery and unified asset scanning.

## Phase 1: AssetManager Enhancements
- [x] Task: Update `AssetManager.h` to include a new `SynthFileInfo` struct (storing path, filename, and a list of extracted instrument names) and a `m_synths` member. [4ab1c2d]
- [x] Task: Implement `.museq` parsing in `AssetManager::process_museq` to extract `instrument <Name> { ... }` blocks using regex or simple string scanning. [4ab1c2d]
- [x] Task: Update `AssetManager::scan_directory` to call `process_museq` for `.museq` files. [4ab1c2d]
- [x] Task: Implement `AssetManager::get_synth_tree` and `AssetManager::get_filtered_synths` to provide the UI with structured synth data. [4ab1c2d]
- [ ] Task: Conductor - User Manual Verification 'AssetManager Enhancements' (Protocol in workflow.md)

## Phase 2: UI Refactoring (muqomposer/main.cpp)
- [ ] Task: Remove hardcoded `synth_templates` and related mock logic from `main.cpp`.
- [ ] Task: Refactor the "Synths" `CollapsingHeader` to use `asset_manager.get_synth_tree`.
- [ ] Task: Update the "Synths" interaction logic:
    - Support clicking/selecting an extracted instrument name.
    - Implement code insertion: For now, we will extract the full definition block from the file and prepend it to the editor (to maintain current behavior but with real data).
- [ ] Task: Ensure the "Samples" tab accurately reflects the results of the unified recursive scan.
- [ ] Task: Conductor - User Manual Verification 'UI Refactoring' (Protocol in workflow.md)

## Phase 3: Robustness and Polish
- [ ] Task: Ensure the "Add Folder" popup correctly calls `refresh_assets` and that all three categories (SF2, Samples, Synths) are populated.
- [ ] Task: Add basic error handling for malformed `.museq` files during the parsing phase.
- [ ] Task: Verify that the search filter correctly prunes all three trees (SoundFonts, Samples, Synths).
- [ ] Task: Conductor - User Manual Verification 'Robustness and Polish' (Protocol in workflow.md)
