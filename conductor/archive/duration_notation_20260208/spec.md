# Specification - Musical Denominator Duration Notation

## Overview
This track introduces a new, theory-friendly notation for specifying note, chord, and rest durations based on musical denominators (e.g., `_4` for a quarter note). This notation serves as an elegant alternative to explicit millisecond values, automatically synchronizing with the song's current `tempo`.

## Functional Requirements

### 1. Suffix Syntax
- **Symbol:** Underscore (`_`) followed by an integer.
- **Placement:** Immediately following a note name, chord name, or rest keyword.
- **Examples:** `C4_4`, `Gmin7_8`, `R_16`, `36_4` (MIDI).

### 2. Supported Denominators
The parser will support a predefined set of musical denominators:
- **Standard:** `1` (Whole), `2` (Half), `4` (Quarter), `8` (Eighth), `16` (Sixteenth), `32` (Thirty-second).
- **Tuplets/Advanced:** `3` (Half-note triplet), `6` (Quarter-note triplet), `12` (Eighth-note triplet), `24` (Sixteenth-note triplet).

### 3. Duration Calculation
Durations are calculated relative to the current `tempo` (BPM):
- **Formula:** `Duration(ms) = (4.0 / Denominator) * (60000.0 / BPM)`
- **Tempo Sync:** If the `tempo` changes mid-script, subsequent denominator-based notes will use the new beat duration.

### 4. Integration
- **Compact Syntax:** The notation must work within the compact notes list (e.g., `Piano { notes C4_4, E4_8, G4_8 }`).
- **Compatibility:** This notation can coexist with the existing millisecond syntax `C4(500)`. If both are provided (unlikely but possible), the latter will override.

## Non-Functional Requirements
- **Parsing Robustness:** The parser should handle the underscore without conflicting with instrument or function names.
- **Precision:** Use floating-point math for the internal calculation to prevent rounding errors in triplets.

## Acceptance Criteria
- [ ] `C4_4` results in a 1-beat duration at any tempo.
- [ ] `R_8` results in a 0.5-beat rest.
- [ ] `Cmaj7_1` results in a 4-beat duration (whole note).
- [ ] Tuplets like `_6` (Quarter-note triplet) result in exactly 2/3 of a beat.
- [ ] Mixing `C4_4` and `D4(250)` in the same sequence works correctly.
