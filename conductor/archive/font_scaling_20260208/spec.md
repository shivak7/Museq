# Specification - Customizable Font Sizes and Persistence

## Overview
This track introduces user-controllable font sizes for the Muqomposer Studio UI and the integrated Code Editor. Users will be able to adjust these sizes via a dedicated Settings Dialog, and their preferences will be saved to a configuration file to persist between sessions.

## Functional Requirements

### 1. Settings Dialog
- **UI:** A new modal popup titled "Settings".
- **Trigger:** A gear icon or "Settings" button added to the Transport Toolbar (Footer).
- **Controls:**
    - **UI Font Size:** Slider (Range: 10 to 30).
    - **Editor Font Size:** Slider (Range: 10 to 40).
- **Actions:** "Apply" (updates fonts immediately) and "Close".

### 2. Persistence
- **Storage:** Preferences saved to `settings.json` in the project root.
- **Loading:** On startup, the application will attempt to load `settings.json`. If missing or invalid, it will use default values (UI: 15, Editor: 18).
- **Saving:** Settings are saved whenever "Apply" is clicked in the Settings Dialog.

### 3. Font Rendering
- **Rebuilding:** Changing font sizes will trigger an ImGui font atlas rebuild to ensure text remains crisp and readable at any scale.
- **Font Selection:** Continue using the system monospaced font for the editor, but at the user-specified size.

## Non-Functional Requirements
- **Graceful Degradation:** The application must not crash if the settings file is corrupted or inaccessible.
- **Performance:** Font atlas rebuilding should be efficient to minimize UI "hiccups" when applying changes.

## Acceptance Criteria
- [ ] A Settings button is visible in the Muqomposer footer.
- [ ] The Settings Dialog correctly displays and modifies font size values.
- [ ] UI and Editor fonts update visually when "Apply" is clicked.
- [ ] Font size preferences persist after closing and restarting the application.
- [ ] `settings.json` is created/updated correctly.
