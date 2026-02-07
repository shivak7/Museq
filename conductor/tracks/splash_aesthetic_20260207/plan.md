# Implementation Plan - Splash Aesthetic Refinement

## Phase 1: Layout and Typography
- [ ] Task: Adjust Splash Window Layout
    - [ ] Update `calculate_splash_dims` or the logic in `main.cpp` to add 100px vertical padding
    - [ ] Recalculate centering logic to account for the new height
- [ ] Task: Implement Scaled Copyright Text
    - [ ] Use `ImGui::SetWindowFontScale(2.0f)` before rendering the copyright text
    - [ ] Update horizontal centering calculation for the larger font
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Layout' (Protocol in workflow.md)

## Phase 2: Fiery Animation Effect
- [ ] Task: Implement Fiery Color Cycle
    - [ ] Define fire color constants (Orange, Red, Gold)
    - [ ] Create a time-based interpolation function using `glfwGetTime()` and `sin()`
- [ ] Task: Apply Pulsing Alpha/Intensity
    - [ ] Integrate the dynamic color into `ImGui::TextColored`
    - [ ] Add a slight alpha flicker to simulate "heat"
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Visuals' (Protocol in workflow.md)

## Phase 3: Final Integration and Cleanup
- [ ] Task: Final Polish and Transition
    - [ ] Ensure all resources (textures) are cleaned up correctly if necessary
    - [ ] Verify transition timing remains at 2.0 seconds
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Final Polish' (Protocol in workflow.md)
