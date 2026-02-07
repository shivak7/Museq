# Specification - Studio Functionality and UI Enhancements

## Overview
This track implements a suite of functional and user interface improvements to Muqomposer Studio. It focuses on build system refinement, improved asset discovery, better code editing ergonomics, a professional export workflow, and final splash screen polish.

## Functional Requirements
- **Build System:**
    - Introduce a `BUILD_TESTS` CMake option, defaulting to `OFF`.
    - Unit tests and integration tests will only compile when `-DBUILD_TESTS=ON` is specified.
- **Global Asset Search:**
    - The sidebar search box will perform a real-time recursive filter on all asset categories (SoundFonts, Samples, and Synths).
    - Non-matching nodes and folders will be hidden dynamically.
- **Smart Code Insertion:**
    - Adding an instrument from the sidebar will prepend the definition to the very top of the script buffer, ensuring it appears before any sequential or parallel playback blocks.
- **Editor Ergonomics:**
    - **Line Numbers:** Display a clickable line number gutter to the left of the script editor.
    - **Auto-Indent:** When pressing `Enter`, the new line will automatically inherit the indentation level (spaces/tabs) of the previous line.
- **Advanced Export Dialog:**
    - Add an "Export" button to the transport toolbar.
    - **Dialog Options:**
        - **Format:** Dropdown for WAV, MP3, and OGG.
        - **Quality/Bitrate:** Slider or presets for compressed formats.
        - **Filename:** Integration with a native save file dialog (if possible) or a text input.
- **Keyboard Shortcuts:**
    - Implement global listeners for:
        - `Ctrl+P`: Play
        - `Ctrl+S`: Save
        - `Ctrl+L`: Load
        - `Ctrl+E`: Export
        - `Ctrl+Space`: Stop
- **Splash Screen Polish:**
    - Scale down the splash image (`Museq_logo.png`) to occupy exactly **50% of the primary display's height** (previously 50% width).
    - Maintain the 100px bottom padding, fiery glowing copyright text, and centering logic.

## Non-Functional Requirements
- **Efficiency:** The recursive search must be optimized to prevent UI lag with large sample libraries.
- **Consistency:** Maintain the dark studio theme and Material Design-inspired UI layout.

## Acceptance Criteria
- Running `cmake ..` without arguments does not build any test executables.
- Searching for a specific sound file hides all unrelated SoundFonts and Synths.
- Pressing `Enter` after an indented line correctly positions the cursor on the next line.
- The Export dialog correctly renders a WAV/MP3/OGG file using the selected parameters.
- Shortcuts trigger their respective actions immediately.
- Splash logo is noticeably smaller and fits comfortably within display height.
