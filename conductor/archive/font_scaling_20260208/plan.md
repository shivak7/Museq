# Implementation Plan - Customizable Font Sizes and Persistence

This plan outlines the implementation of a Settings system for Muqomposer, focusing on customizable font sizes and persistence via JSON.

## Phase 1: Persistence and Settings Management
- [x] Task: Define a `Settings` struct and implement `load_settings()` / `save_settings()` helpers using `nlohmann/json`. [5d69634]
- [x] Task: Ensure the application loads `settings.json` on startup and falls back to defaults (UI: 15, Editor: 18) if not found. [5d69634]
- [x] Task: Conductor - User Manual Verification 'Settings Persistence' (Protocol in workflow.md) [5d69634]

## Phase 2: Settings UI and Dialog
- [x] Task: Add a "Settings" button (or gear icon) to the transport toolbar in `muqomposer/main.cpp`. [6a4e3c4, 496b122]
- [x] Task: Implement the "Settings" modal popup with sliders for "UI Font Size" and "Editor Font Size". [6a4e3c4]
- [x] Task: Implement the "Apply" logic which saves settings to disk and sets a flag for font rebuilding. [6a4e3c4]
- [x] Task: Conductor - User Manual Verification 'Settings UI' (Protocol in workflow.md) [2b94ba2]

## Phase 3: Dynamic Font Rebuilding [checkpoint: 2cf873c]
- [x] Task: Refactor font loading in `main.cpp` into a reusable function `RebuildFonts(float uiSize, float editorSize)`. [6a4e3c4]
- [x] Task: Implement the mechanism to clear the ImGui font atlas and reload fonts when the rebuild flag is set. [6a4e3c4, 2b94ba2]
- [x] Task: Ensure both the main font and the editor's monospaced font are updated correctly. [2b94ba2]
- [x] Task: Conductor - User Manual Verification 'Dynamic Font Scaling' (Protocol in workflow.md) [2b94ba2]
