# Implementation Plan - Splash Screen Refinement

## Phase 1: Window State and Scaling Logic
- [ ] Task: Implement Borderless Initial State
    - [ ] Update GLFW window hints in `muqomposer/main.cpp` to start with `GLFW_DECORATED = GLFW_FALSE`
    - [ ] Calculate monitor resolution using `glfwGetVideoMode(glfwGetPrimaryMonitor())`
- [ ] Task: Implement Dynamic Scaling and Centering
    - [ ] Calculate scaled splash dimensions (50% monitor width)
    - [ ] Update `glfwSetWindowSize` and `glfwSetWindowPos` for the splash phase
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Window Setup' (Protocol in workflow.md)

## Phase 2: Splash Rendering and Copyright
- [ ] Task: Implement Scaled Splash Image Rendering
    - [ ] Update `AddImage` coordinates in the ImGui splash loop to match the scaled window dimensions
- [ ] Task: Add Copyright Attribution Text
    - [ ] Implement `ImGui::Text` centered at the bottom of the splash screen with the specified text
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Visuals' (Protocol in workflow.md)

## Phase 3: Transition and Restoration
- [ ] Task: Implement UI Restoration Logic
    - [ ] After the 2-second loop, call `glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE)`
    - [ ] Restore default window size (1280x720) and re-center on the monitor
- [ ] Task: Final Polish and Flash Prevention
    - [ ] Ensure `glfwShowWindow` is only called after the dark background is cleared
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Final Transition' (Protocol in workflow.md)
