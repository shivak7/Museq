# Implementation Plan - Multi-Tab Editor Interface

This plan outlines the transition from a single-file editor to a robust multi-tab system in Muqomposer.

## Phase 1: Multi-Tab Foundation
- [x] Task: Define an `EditorTab` struct to encapsulate `TextEditor`, `Song` (for error markers), file path, and "dirty" status. [2508ef7]
- [x] Task: Replace the single global `TextEditor` with a `std::vector<EditorTab>` and an `active_tab_index` pointer. [2508ef7]
- [x] Task: Implement the `ImGui::TabBar` UI at the top of the editor area to display and switch between tabs. [2508ef7]
- [x] Task: Implement "Smart Reuse" logic: opening a file from the sidebar or "Load" menu will focus an existing tab or create a new one. [2508ef7]
- [x] Conductor - User Manual Verification 'Basic Tab Switching' (Protocol in workflow.md) [21f22da]

## Phase 2: Tab Lifecycle and Dirty State
- [x] Task: Implement "Dirty" state tracking: detect text changes and mark the active tab with an asterisk (`*`). [21f22da, 66d8915]
- [x] Task: Implement the close button (`x`) on tabs. [21f22da, 66d8915]
- [x] Task: Create a "Save Changes?" modal popup that appears when closing a dirty tab, offering Save, Discard, and Cancel. [66d8915]
- [x] Task: Add "New Tab" functionality (keyboard shortcut or sidebar action) creating an "Untitled" empty script. [66d8915]
- [x] Conductor - User Manual Verification 'Dirty State & Closing' (Protocol in workflow.md) [240760c]

## Phase 3: Global Integration & Refinement
- [x] Task: Update the transport toolbar (Play, Stop, Export) to always operate on the `last_parsed_song` and script content of the *active* tab. [240760c]
- [x] Task: Ensure real-time syntax error markers and playback highlighting are correctly isolated to each tab's editor instance. [240760c]
- [x] Task: Refactor the "Save" and "Save As" logic to correctly update the specific metadata of the active tab. [240760c]
- [x] Conductor - User Manual Verification 'Multi-Tab Playback' (Protocol in workflow.md) [0c6231a]
