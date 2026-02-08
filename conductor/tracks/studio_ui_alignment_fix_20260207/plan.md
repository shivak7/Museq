# Implementation Plan - Studio UI Alignment and Scrolling Fix

## Phase 1: Scrolling Synchronization
- [ ] Task: Move the `scroll_y` calculation logic before the rendering of both children or use a more robust shared state mechanism.
- [ ] Task: Ensure `ImGui::SetScrollY` is called consistently for both `LineNumbers` and `EditorColumn` containers.

## Phase 2: Alignment Drift Fix
- [ ] Task: Research and verify the exact `InputTextMultiline` internal padding and line spacing.
- [ ] Task: Adjust the `SetCursorPosY` calculation in the line number loop to match the editor's internal rendering.
- [ ] Task: Consider using a shared `ImGuiTable` or `ImGui::Columns` if simple cursor adjustment remains brittle.

## Phase 3: Verification
- [ ] Task: Manually verify alignment with a long script (50+ lines).
- [ ] Task: Manually verify smooth scrolling.
- [ ] Task: Conductor - User Manual Verification 'Studio UI Fix' (Protocol in workflow.md)
