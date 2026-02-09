# Implementation Plan - Musical Denominator Duration Notation

This plan outlines the updates to the `ScriptParser` to support the `_Denominator` suffix for note, rest, and chord durations.

## Phase 1: Core Parsing Logic
- [x] Task: Update `ScriptParser::parse_compact_notes` to detect the `_` delimiter during tokenization. [47ed671]
- [x] Task: Implement a helper function `calculate_denominator_duration(int denominator)` that uses the current global BPM. [47ed671]
- [x] Task: Implement validation for the predefined set of denominators (1, 2, 3, 4, 6, 8, 12, 16, 24, 32). [47ed671]
- [ ] Task: Conductor - User Manual Verification 'Core Duration Logic' (Protocol in workflow.md)

## Phase 2: Notation Integration
- [ ] Task: Extend the Note/Chord/Rest parsing paths in `ScriptParser.cpp` to handle the `_` suffix across all element types.
- [ ] Task: Ensure that `36_4` (MIDI pitch) is correctly parsed without conflicting with the underscore.
- [ ] Task: Add comprehensive unit tests in a new `testing/test_duration_notation.cpp` file covering standard and tuplet values at various tempos.
- [ ] Task: Conductor - User Manual Verification 'Notation Integration' (Protocol in workflow.md)

## Phase 3: Documentation and Cleanup
- [ ] Task: Update the `README.md` "Scripting Language Guide" to include the new `_Denominator` syntax with examples.
- [ ] Task: Remove any temporary debug logs used during implementation.
- [ ] Task: Conductor - User Manual Verification 'Final Feature Check' (Protocol in workflow.md)
