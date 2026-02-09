# Specification - Multi-Tab Editor Interface

## Overview
This track implements a multi-tabbed editor interface for Muqomposer Studio. It allows users to work on multiple `.museq` files simultaneously, switching between them via a tab bar, while maintaining independent edit histories and "dirty" states for each file.

## Functional Requirements

### 1. Tab Bar UI
- **Location:** At the top of the Code Editor area.
- **Controls:**
    - Individual tabs for each open file showing the filename.
    - A "dirty" indicator (e.g., `*` next to the name) for unsaved changes.
    - A close button (`x`) on each tab.
- **Selection:** Clicking a tab makes it active and displays its content in the editor.

### 2. Tab Management (Smart Reuse)
- **Opening:** When loading a file (via Sidebar or Load Dialog):
    - If the file is already open in a tab, switch to that tab immediately.
    - If not, create a new tab and load the file content.
- **Closing:** 
    - If a tab has no unsaved changes, close it immediately.
    - If a tab is "dirty", show a "Save Changes?" confirmation popup with "Save", "Discard", and "Cancel" options.

### 3. Playback and Context
- **Active Context:** The "Play", "Stop", and "Export" controls always apply to the script in the **currently selected tab**.
- **Switching:** Switching tabs while music is playing does *not* stop the audio. However, pressing "Play" in a new tab will stop the current playback and start rendering the new tab's code.

### 4. Integration
- **Independent State:** Each tab maintains its own `TextEditor` state, including undo/redo history and cursor position.
- **Limit:** Support up to 10 simultaneous tabs (for UI stability).

## Non-Functional Requirements
- **Performance:** Switching tabs should be instantaneous.
- **UI Consistency:** Match the existing Dracula/Dark studio theme.

## Acceptance Criteria
- [ ] Multiple files can be open at once, visible in a tab bar.
- [ ] Loading an already-open file from the sidebar focuses its existing tab.
- [ ] Closing a modified tab triggers a "Save Changes?" warning.
- [ ] Pressing Play in different tabs correctly targets the active tab's script.
- [ ] Tabs maintain their own undo history after switching back and forth.
