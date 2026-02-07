# Specification - Muqomposer Features and Bugfixes

## Overview
This track addresses several UI/UX issues and adds missing studio features to the `muqomposer` GUI. The focus is on branding (splash screen, icons), workflow safety (instrument collision handling), asset management (clearing folders), and interactive previews.

## Functional Requirements
- **Branding & Splash:** 
    - Integrate `Museq_logo.png` into the application window icon.
    - Replace the initial white screen flash with a dark animated splash screen using inverted colors of the Museq logo.
- **Asset Management:**
    - **Clear Button:** Add a "Clear All" button to the sidebar to remove all currently watched asset directories.
    - **Smart Insertion:** Prevent adding instrument code to the editor if the exact asset is already defined in the script.
    - **Conflict Resolution:** If an instrument with the same name exists but the code is different, automatically rename the new insertion by appending an underscore and incrementing a number (e.g., `Piano_1`, `Piano_2`).
- **Enhanced Previews:**
    - Extend the `[>]` preview button to support synth templates and individual samples.
    - **Safety Fade:** Implement a hard limit of 5 seconds for all previews. If a sample is longer than 5 seconds, the engine must perform an automated fade-out starting at the 4-second mark.
- **Visual Feedback:**
    - Implement a real-time Spectrum Analyzer (FFT) widget in the output panel.
- **Transport Toolbar:**
    - Refactor footer buttons into a more robust transport toolbar with BPM controls and potentially icons.

## Non-Functional Requirements
- **Startup Speed:** The splash screen must appear immediately to prevent the "white flash" common in GLFW/OpenGL initialization on some platforms.
- **Engine Safety:** The preview fade-out must be handled in the audio thread to ensure it is glitch-free.

## Acceptance Criteria
- Muqomposer starts with a dark, branded splash screen.
- Clicking "Clear" in the sidebar empties the asset tree.
- Dragging/clicking an asset into the editor handles name collisions gracefully.
- Audio previews for long files are automatically capped and faded out at 5 seconds.
- A functional FFT visualizer is visible during playback.
