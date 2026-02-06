# Implementation Plan - Advanced Asset Management

## Phase 1: Dynamic Folder Loading and Navigation [checkpoint: d769d40]
- [x] Task: Implement Folder Loading logic 04e9dbe
    - [x] Create UI button to 'Add Folder'
    - [x] Store a list of 'Watched Folders' in Muqomposer state
- [x] Task: Enhance Tree-based Directory Scanning f773a04
    - [x] Implement recursive scanning of folders for .sf2 and audio files
    - [x] Optimize scanning to avoid UI freezes (potentially background thread)
- [x] Task: Real-time Asset Filtering f773a04
    - [x] Add Search input box to Left Panel
    - [x] Implement filtering logic for the tree view
- [x] Task: Conductor - User Manual Verification 'Phase 1: Folder Management' (Protocol in workflow.md) d769d40

## Phase 2: Interaction and Previews
- [ ] Task: Implement Audio Preview (Demo)
    - [ ] Add 'Play' icon next to SoundFont presets and samples
    - [ ] Interface with `AudioPlayer` to trigger immediate playback
- [ ] Task: Drag and Drop Support
    - [ ] Implement `ImGui::BeginDragDropSource` for asset items
    - [ ] Update Editor to handle drops and insert code snippets
- [ ] Task: Favorites and UI Polish
    - [ ] Implement simple persistence for favorited assets
    - [ ] Ensure resizable/collapsible consistency with the schematic
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Interaction' (Protocol in workflow.md)
