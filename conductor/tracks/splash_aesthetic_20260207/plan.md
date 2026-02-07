# Implementation Plan - Splash Aesthetic Refinement

## Phase 1: Layout and Typography
- [x] Task: Adjust Splash Window Layout
    - [x] Update `calculate_splash_dims` or the logic in `main.cpp` to add 100px vertical padding
    - [x] Recalculate centering logic to account for the new height
- [x] Task: Implement Scaled Copyright Text
    - [x] Use `ImGui::SetWindowFontScale(2.0f)` before rendering the copyright text
    - [x] Update horizontal centering calculation for the larger font
- [x] Task: Conductor - User Manual Verification 'Phase 1: Layout' (Protocol in workflow.md)

## Phase 2: Fiery Animation Effect
- [x] Task: Implement Fiery Color Cycle
    - [x] Define fire color constants (Orange, Red, Gold)
    - [x] Create a time-based interpolation function using `glfwGetTime()` and `sin()`
- [x] Task: Apply Pulsing Alpha/Intensity
    - [x] Integrate the dynamic color into `ImGui::TextColored`
    - [x] Add a slight alpha flicker to simulate "heat"
- [x] Task: Conductor - User Manual Verification 'Phase 2: Visuals' (Protocol in workflow.md)

## Phase 3: Final Integration and Cleanup
- [x] Task: Final Polish and Transition
    - [x] Ensure all resources (textures) are cleaned up correctly if necessary
    - [x] Verify transition timing remains at 2.0 seconds
- [x] Task: Conductor - User Manual Verification 'Phase 3: Final Polish' (Protocol in workflow.md)
