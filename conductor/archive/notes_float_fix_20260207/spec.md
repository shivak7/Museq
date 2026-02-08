# Specification - Notes Float Bug Fix

## Overview
Users reported that the program crashes when floating point values are provided to the `notes` (compact notation) or `note` keywords where integers are expected (duration, velocity). The system should instead detect these invalid values, print a descriptive warning to `std::cerr`, and skip the invalid sequence or note.

## Functional Requirements
- **Validation:** Add validation logic to `ScriptParser` to detect floating point values in `duration` and `velocity` parameters for `note` and `notes` keywords.
- **Graceful Failure:** When invalid values are detected:
    - Print a warning message to `std::cerr`.
    - Skip the affected note or the entire `notes` sequence.
- **Robustness:** Ensure all intermediate parsing variables (like `p1`, `p2` in `parse_compact_notes`) are properly initialized to avoid using garbage values on parsing failure.

## Acceptance Criteria
- [ ] Scripts containing `notes C(500.5)` no longer crash.
- [ ] A warning is printed to the console when an invalid float is detected.
- [ ] The invalid note/sequence is ignored, and the rest of the script continues to parse and render.
- [ ] Unit test verifies that the program handles various invalid float inputs without crashing.
