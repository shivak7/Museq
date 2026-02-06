# Specification - Music Theory Features

## Overview
This track aims to enhance the Museq scripting language and engine with first-class support for musical theory concepts, specifically scales and chords. This will allow composers to write more expressive and harmonically consistent music with less boilerplate.

## User Stories
- As a composer, I want to define a global or local scale (e.g., C Major, A Minor) so that I can use relative note names (e.g., 1, 3, 5) instead of absolute MIDI names.
- As a composer, I want to use chord names (e.g., Cmaj, Dm7) in my scripts so that I don't have to manually define each note in a chord using the '+' syntax.
- As a composer, I want the synthesis engine to be aware of these structures to potentially support algorithmic harmonizing.

## Functional Requirements
- **Scale Definition:** New keyword `scale` to define a key and a mode.
- **Scale-Relative Pitch Parsing:** Update `NoteParser` to handle relative indices (1-7) when a scale is active.
- **Chord Library:** A built-in library of common chord formulas (Major, Minor, 7th, etc.).
- **Chord Parsing:** Update `ScriptParser` to recognize chord names and expand them into polyphonic note groups.

## Non-Functional Requirements
- **Performance:** Pitch resolution must remain efficient during rendering.
- **Backward Compatibility:** Existing scripts using absolute note names must continue to function perfectly.
