# Specification - Studio UI and UX Refinement

## Overview
This track addresses several issues and suggestions reported by the user, focusing on editor alignment, indentation stability, asset discovery, and overall UI robustness.

## Functional Requirements
- **Editor & Line Numbers:**
    - Enforce a monospace font for both the line number gutter and the main script editor.
    - Synchronize vertical alignment so each line number perfectly corresponds to its respective text line.
- **Improved Auto-Indent:**
    - Re-implement auto-indentation using the `ImGuiInputTextFlags_CallbackAlways` (or similar) callback.
    - When a newline is detected, automatically insert the leading indentation (spaces/tabs) from the previous line at the new cursor position.
- **Enhanced Asset Search:**
    - Modify the SoundFont tree building logic to filter the actual `presets` list within `SF2Info` objects based on the search query.
    - Only matching presets will be passed to and rendered by the UI.
- **Export Quality Control:**
    - Add a "Quality / Bitrate" slider to the Export Song modal.
    - **WAV:** Slider hidden or disabled.
    - **MP3:** Maps to bitrate (64kbps to 320kbps).
    - **OGG:** Maps to quality factor (0.0 to 1.0).
- **Shortcut Stability & Popups:**
    - Prevent application hangs by ensuring only one modal dialog can be open at a time.
    - Pressing a shortcut (e.g., `Ctrl+L`) while another dialog (e.g., Save) is open will automatically close the current dialog and open the new one.
    - **Escape Key:** Global listener to close or cancel any open modal dialog.
- **Play/Pause Toggle:**
    - Update `Ctrl+P` logic to toggle between Play and Stop (Pause) states.

## Non-Functional Requirements
- **Robustness:** Eliminate race conditions or deadlocks when rapidly switching between modal popups.
- **Visual Accuracy:** Line numbers must align exactly with text even after wrapping (if applicable) or scrolling.

## Acceptance Criteria
- Line numbers 1, 2, 3... are vertically centered with their respective text lines.
- Pressing `Enter` on an indented line correctly preserves the indentation.
- Searching for "Strings" only displays SoundFont presets containing the word "Strings."
- The Export dialog correctly applies the selected quality/bitrate setting to the output file.
- Pressing `Ctrl+S` then `Ctrl+L` smoothly transitions between the Save and Load popups without hanging.
- Pressing `Escape` closes any open modal.
- `Ctrl+P` starts the song if stopped, and stops it if playing.
