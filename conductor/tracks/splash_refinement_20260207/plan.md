# Implementation Plan - Splash Screen Refinement

## Phase 1: Window State and Scaling Logic [checkpoint: ab50424]
- [x] Task: Implement Borderless Initial State
    - [x] Update GLFW window hints in `muqomposer/main.cpp` to start with `GLFW_DECORATED = GLFW_FALSE`
    - [x] Calculate monitor resolution using `glfwGetVideoMode(glfwGetPrimaryMonitor())`
- [x] Task: Implement Dynamic Scaling and Centering
    - [x] Calculate scaled splash dimensions (50% monitor width)
    - [x] Update `glfwSetWindowSize` and `glfwSetWindowPos` for the splash phase
- [x] Task: Conductor - User Manual Verification 'Phase 1: Window Setup' (Protocol in workflow.md) ab50424

## Phase 2: Splash Rendering and Copyright
- [x] Task: Implement Scaled Splash Image Rendering
    - [x] Update `AddImage` coordinates in the ImGui splash loop to match the scaled window dimensions
- [x] Task: Add Copyright Attribution Text
    - [x] Implement `ImGui::Text` centered at the bottom of the splash screen with the specified text
- [x] Task: Conductor - User Manual Verification 'Phase 2: Visuals' (Protocol in workflow.md)

## Phase 3: Transition and Restoration
- [x] Task: Implement UI Restoration Logic
    - [x] After the 2-second loop, call `glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE)`
    - [x] Restore default window size (1280x720) and re-center on the monitor
- [x] Task: Final Polish and Flash Prevention
    - [x] Ensure `glfwShowWindow` is only called after the dark background is cleared
- [x] Task: Conductor - User Manual Verification 'Phase 3: Final Transition' (Protocol in workflow.md)
