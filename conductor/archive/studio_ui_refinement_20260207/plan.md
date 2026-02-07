# Implementation Plan - Studio UI and UX Refinement

## Phase 1: Editor Refinements
- [x] Task: Synchronize Line Numbers
    - [x] Update `main.cpp` to use `ImGui::GetStyle().FramePadding.y` for vertical alignment of line numbers.
    - [x] Ensure font is consistent (use default monospace if available).
- [x] Task: Implement Callback-based Auto-Indent
    - [x] Create `editor_callback` function that detects `Enter` key.
    - [x] Read current line indentation and insert into buffer via `data->InsertChars`.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Editor' (Protocol in workflow.md)

## Phase 2: Search and Asset Filtering
- [x] Task: Filter SF2 Presets in Tree
    - [x] Update `AssetManager::get_soundfont_tree` to perform deeper filtering.
    - [x] Modify `SF2Info` struct or create a temporary view structure that only holds matching presets.
- [x] Task: Verify Search UI
    - [x] Ensure `render_asset_tree_node` correctly iterates over the filtered list.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Search' (Protocol in workflow.md)

## Phase 3: Export and Shortcuts [checkpoint: 9b0dc19]
- [x] Task: Update Export Modal [9b0dc19]
    - [x] Add "Quality / Bitrate" slider.
    - [x] Add logic to hide/show slider based on format (MP3/OGG).
    - [x] Pass quality value to `export_logic`.
- [x] Task: Enhance Shortcuts and Popups [9b0dc19]
    - [x] Implement `close_all_popups` helper.
    - [x] Update shortcuts (Ctrl+S, Ctrl+L, Ctrl+E) to call helper before opening new popup.
    - [x] Add `Ctrl+P` toggle logic (`is_playing ? stop : play`).
    - [x] Add `Escape` key listener to close active popups.
- [x] Task: Conductor - User Manual Verification 'Phase 3: Final Polish' (Protocol in workflow.md) 9b0dc19
