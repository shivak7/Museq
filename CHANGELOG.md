# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **Modular Import System:** Added `import` keyword to load instrument templates from external `.museq` files.
- **Instrument Renaming:** Implemented automatic renaming (e.g., `Piano_1`) and warnings for instrument name conflicts during imports.
- **Polyphony (Chords):** Added support for simultaneous notes using the `+` operator (e.g., `C4+E4+G4`).
- **Global Variables:** Introduced `var` keyword for defining reusable constants (e.g., `var BPM 128`) with `$var` substitution.
- **Time Offsets:** Added `offset` (milliseconds) and `phase` (fraction of a beat) keywords to shift blocks in time.
- **Octave Context:** Added `octave` keyword to set a default octave for subsequent notes within a block or globally.
- **Instrument Gain:** Added `gain` property to instruments for easier master volume control per track.
- **Note Repetition:** Added `*` operator for repeating notes in compact syntax (e.g., `C4*4`).
- **Synthesizer Library:** Created `musynths/` directory with baseline templates for Leads, Pads, Drums, and SFX.
- **Comprehensive Documentation:** Updated `README.md` with detailed effect parameters, portamento, and MIDI support.

### Changed
- **Upfront Declarations:** Restricted instrument definitions to the top-level of scripts to improve structural clarity.
- **Note Parser Enhancement:** Updated `NoteParser` to support default octave parameters and handle invalid brace characters gracefully.
- **Audio Engine:** Updated `AudioRenderer` to support non-advancing notes for true polyphony and structural offsets.

### Fixed
- **Parsing Bug:** Fixed a critical bug where braces attached to keywords (e.g., `Mandolin {`) caused parsing failures.
- **Panning Inconsistency:** Fixed an issue where compact note syntax ignored the instrument's default panning value.
