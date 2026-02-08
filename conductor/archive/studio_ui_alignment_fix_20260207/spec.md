# Specification - Studio UI Alignment and Scrolling Fix

## Overview
The Code Interface in Muqomposer has two primary UI issues:
1.  **Line Number Drift:** As the number of lines in the script increases, the line numbers in the left column gradually drift out of vertical alignment with the text in the multi-line input field.
2.  **Scrolling Desync:** The line number column does not scroll in sync with the code editor, or it lags by one frame.

## Functional Requirements
- **Synchronized Scrolling:** Ensure the `LineNumbers` column and the `EditorColumn` always share the same vertical scroll position.
- **Accurate Alignment:** Fix the vertical positioning logic for line numbers so they remain perfectly aligned with the corresponding lines in the `InputTextMultiline` widget, regardless of the total line count.
- **Robustness:** The solution should handle window resizing and font scaling if applicable (though currently fixed).

## Acceptance Criteria
- [ ] Line numbers and code lines are perfectly aligned at line 1, line 100, and line 500+.
- [ ] Scrolling the code editor using the mouse wheel or scrollbar immediately updates the line number column position.
- [ ] No visual "jitter" or one-frame lag in the line number column during scrolling.
