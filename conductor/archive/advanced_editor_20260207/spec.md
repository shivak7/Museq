# Specification - Muqomposer Visuals: Advanced Code Editor

## Overview
To improve the visuals and usability of the Muqomposer Studio, we will replace the basic `ImGui::InputTextMultiline` widget with a feature-rich code editor. This track focuses on integrating `ImGuiColorTextEdit` to provide syntax highlighting, line numbers, and advanced editing capabilities. Additionally, we will implement custom visual feedback mechanisms such as real-time playback tracking and inline error reporting to enhance the user's workflow.

## Functional Requirements

### 1. Third-Party Editor Integration
- **Library:** Integrate `ImGuiColorTextEdit` (or a similar lightweight ImGui editor component) into the project.
- **Source:** Vendor the library source code into `third_party/ImGuiColorTextEdit`.
- **Replacement:** Replace the existing `InputTextMultiline` in `muqomposer/main.cpp` with the new editor widget.

### 2. Syntax Highlighting & Styling
- **Language Support:** Define a custom language definition for Museq scripts, highlighting keywords (`instrument`, `note`, `parallel`, `sequential`, etc.), numbers, strings, and comments.
- **Theme:** Apply a dark, high-contrast palette consistent with the Studio aesthetic (e.g., Dracula or Monokai-inspired).
- **Font:** Increase the default font size for better readability and ensure a monospaced font is used for the editor.

### 3. Visual Feedback
- **Playback Tracking:** Implement a mechanism to highlight the line of code currently being executed by the audio engine. This requires mapping the `AudioPlayer` state back to the source line.
- **Error Visualization:** Parse the script and highlight lines containing syntax errors with error markers (e.g., red underline or gutter icon).
- **Auto-completion:** Implement a basic auto-complete popup that suggests keywords and currently defined instrument names.

## Non-Functional Requirements
- **Performance:** The syntax highlighting and real-time feedback must not degrade the rendering framerate significantly (target 60fps).
- **Maintainability:** The editor integration should be modular, keeping the core `main.cpp` loop clean.

## Acceptance Criteria
- [ ] `ImGuiColorTextEdit` is successfully vendored and built with the project.
- [ ] The "Code Interface" panel displays the new editor with syntax highlighting for Museq keywords.
- [ ] The font size in the editor is increased for better visibility.
- [ ] Typing provides auto-completion suggestions for standard keywords.
- [ ] During playback, the current line is visually distinguished.
- [ ] Syntax errors are marked on the correct line.
