# Specification - Splash Screen Refinement

## Overview
This track fixes persistent issues with the Muqomposer splash screen, specifically the presence of window borders/titles during launch and the cropping of the splash image. It also implements a more professional "borderless" transition into the main application and adds branding information.

## Functional Requirements
- **Borderless Initialization:**
    - The initial application window must be created without decorations (no title bar, no borders) using GLFW window hints.
- **Dynamic Image Scaling:**
    - The splash image (`Museq_logo.png`) must be scaled to occupy exactly 50% of the primary display's width while maintaining its aspect ratio.
    - Centering logic must ensure the window is positioned in the middle of the screen.
- **Copyright Attribution:**
    - Display the text "© 2026 Shiva Ratna (https://shivak7.github.io/)" at the bottom of the splash screen.
    - Use a clean, readable font (standard ImGui font is acceptable).
- **Visual Transition:**
    - After the 2-second splash sequence finishes, the window must be "restored" to its standard state:
        - Enable window decorations (title bar, borders).
        - Resize to the default 1280x720 dimension.
        - Re-center the window on the display.
- **Dark Inverted Aesthetic:**
    - Maintain the dark background (`0.1f, 0.1f, 0.1f`) and inverted logo colors.

## Non-Functional Requirements
- **No White Flash:** Ensure the window is hidden or cleared to dark immediately after creation.
- **Display Awareness:** Correctly identify the primary monitor's resolution using GLFW for scaling calculations.

## Acceptance Criteria
- Muqomposer launches with a perfectly borderless, re-centered window.
- The splash image occupies 50% of the monitor width and is fully visible (no cropping).
- The specified copyright text is clearly visible during the splash sequence.
- After the splash, the window title "Muqomposer" and standard buttons appear correctly.
