# Implementation Plan - Muqomposer Visuals: Advanced Code Editor

This plan outlines the integration of `ImGuiColorTextEdit` and the implementation of advanced editor features for Muqomposer.

## Phase 1: Dependency Integration & Basic Setup
- [x] Task: Vendor `ImGuiColorTextEdit` source code into `third_party/ImGuiColorTextEdit`. [725135a]
- [x] Task: Update `muqomposer/CMakeLists.txt` to include the new library files. [725135a]
- [x] Task: In `muqomposer/main.cpp`, replace the `ImGui::InputTextMultiline` widget with the `TextEditor` class from the library. [725135a]
- [x] Task: Configure the editor with a basic color palette and a placeholder "C++-like" language definition to verify integration. [725135a]
- [x] Task: Conductor - User Manual Verification 'Dependency Integration' (Protocol in workflow.md) [725135a]

## Phase 2: Museq Language Support & Styling
- [x] Task: Define a custom `TextEditor::LanguageDefinition` for the Museq scripting language (keywords: `instrument`, `note`, `sequence`, etc.). [fee4cb6]
- [x] Task: Implement a high-contrast dark palette for the editor. [fee4cb6]
- [x] Task: Load a larger monospaced font (e.g., embedded or system font) and apply it to the editor instance. [fee4cb6]
- [x] Task: Conductor - User Manual Verification 'Language Support & Styling' (Protocol in workflow.md) [fc2b1a7]

## Phase 3: Advanced Visual Feedback [checkpoint: 1b5252b]
- [x] Task: Implement `Auto-completion`: Hook into the editor's completion API (if available) or overlay a custom ImGui popup to suggest keywords and instrument names. [93cbd87, b9798f9, 6187944, 45490da, 8d2eaaa]
- [x] Task: Implement `Error Visualization`: Update `ScriptParser` to return line numbers for errors, and use the editor's marker API to flag them. [93cbd87, f6cdde0, 1fe9432]
- [x] Task: Implement `Playback Tracking`:
    - Extend `Song` and `AudioPlayer` to track the current source line index during playback. [93cbd87]
    - Use the editor's line highlighting or execution marker API to show the active line. [93cbd87]
- [x] Task: Conductor - User Manual Verification 'Advanced Visual Feedback' (Protocol in workflow.md) [d7363c6]
