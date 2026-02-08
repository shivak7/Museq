# Implementation Plan - Studio UI Alignment and Scrolling Fix

## Phase 1: Scrolling Synchronization
- [x] Task: Move the `scroll_y` calculation logic before the rendering of both children or use a more robust shared state mechanism. [0096fe4]
- [x] Task: Ensure `ImGui::SetScrollY` is called consistently for both `LineNumbers` and `EditorColumn` containers. [801ffe2]

## Phase 2: Alignment Drift Fix
- [x] Task: Research and verify the exact `InputTextMultiline` internal padding and line spacing. [b3511be]
- [x] Task: Adjust the `SetCursorPosY` calculation in the line number loop to match the editor's internal rendering. [b3511be]
- [x] Task: Consider using a shared `ImGuiTable` or `ImGui::Columns` if simple cursor adjustment remains brittle. (Implemented via height hack to force parent scroll). [b3511be]

## Phase 3: Verification [checkpoint: 7551cc1]
- [x] Task: Manually verify alignment with a long script (50+ lines). [b3511be]
- [x] Task: Manually verify smooth scrolling. [b3511be]
- [x] Task: Conductor - User Manual Verification 'Studio UI Fix' (Protocol in workflow.md)
