# Implementation Plan - Studio UI and UX Refinement

## Phase 1: Editor Refinements
- [ ] Task: Synchronize Line Numbers
    - [ ] Update `main.cpp` to use `ImGui::GetStyle().FramePadding.y` for vertical alignment of line numbers.
    - [ ] Ensure font is consistent (use default monospace if available).
- [ ] Task: Implement Callback-based Auto-Indent
    - [ ] Create `editor_callback` function that detects `Enter` key.
    - [ ] Read current line indentation and insert into buffer via `data->InsertChars`.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Editor' (Protocol in workflow.md)

## Phase 2: Search and Asset Filtering
- [ ] Task: Filter SF2 Presets in Tree
    - [ ] Update `AssetManager::get_soundfont_tree` to perform deeper filtering.
    - [ ] Modify `SF2Info` struct or create a temporary view structure that only holds matching presets.
- [ ] Task: Verify Search UI
    - [ ] Ensure `render_asset_tree_node` correctly iterates over the filtered list.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Search' (Protocol in workflow.md)

## Phase 3: Export and Shortcuts
- [ ] Task: Update Export Modal
    - [ ] Add "Quality / Bitrate" slider.
    - [ ] Add logic to hide/show slider based on format (MP3/OGG).
    - [ ] Pass quality value to `export_logic`.
- [ ] Task: Enhance Shortcuts and Popups
    - [ ] Implement `close_all_popups` helper.
    - [ ] Update shortcuts (Ctrl+S, Ctrl+L, Ctrl+E) to call helper before opening new popup.
    - [ ] Add `Ctrl+P` toggle logic (`is_playing ? stop : play`).
    - [ ] Add `Escape` key listener to close active popups.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Final Polish' (Protocol in workflow.md)
