# Specification - Splash Aesthetic Refinement

## Overview
This track enhances the visual appeal of the Muqomposer splash screen by improving the copyright message's visibility and adding a dynamic "fiery" animation effect. It also refines the layout to ensure the larger text has adequate breathing room.

## Functional Requirements
- **Layout Refinement:**
    - Increase the splash window height by an additional 100 pixels below the scaled Museq logo.
    - Ensure the window remains centered on the monitor despite the increased height.
- **Enhanced Typography:**
    - Double the font size of the copyright message ("© 2026 Shiva Ratna...") using `ImGui::SetWindowFontScale(2.0f)`.
    - Position the text horizontally centered within the newly created bottom 100px area.
- **Fiery Glow Animation:**
    - Implement a dynamic color oscillation for the copyright text.
    - The color must cycle through fiery tones:
        - **Core Colors:** Bright Orange (`1.0, 0.5, 0.0`), Deep Red (`0.8, 0.1, 0.0`), and Gold (`1.0, 0.8, 0.0`).
    - The intensity (alpha and color mix) should use a sine-wave function based on `glfwGetTime()` to create a flickering "heat" effect.

## Non-Functional Requirements
- **Performance:** The animation logic must be efficient enough to run smoothly at vsync (60Hz) without impacting startup time.
- **Visual Consistency:** Maintain the borderless window and dark background (`0.1, 0.1, 0.1`) established in previous refinements.

## Acceptance Criteria
- Muqomposer splash screen shows a significantly larger, centered copyright message.
- The copyright text flickers between red, orange, and gold colors.
- There is a clear 100px margin of dark space below the logo before the window ends.
- The transition to the main app remains seamless.
