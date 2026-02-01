# Museq Roadmap & TODO

This document outlines planned features, experimental ideas, and architectural improvements for future versions of Museq.

## 🎤 Vocal Synthesis & Humanization
*   **Formant Synthesis (The "Robot" Voice):**
    *   Implement parallel Bandpass filters to simulate human vocal tracts.
    *   Add a `vowel` command (e.g., `vowel "a"`, `vowel "o"`) to shape oscillator output.
    *   *Goal:* Create Daft Punk-style "talkbox" effects natively using the existing synth engine.
*   **Retro TTS (Text-to-Speech):**
    *   Investigate integrating lightweight C-based TTS engines (like SAM or libctts).
    *   Allow binding lyrics to notes (e.g., `lyrics "Hel-lo World"`).
*   **Sampler Improvements:**
    *   Explore formant-preserving pitch shifting for sample-based vocals to avoid the "chipmunk" effect.

## 🎛️ DSP & Synthesis Enhancements
*   **Global Effects Chain:**
    *   Add Reverb, Delay/Echo, and Compression.
    *   Allow effects to be applied globally or per-instrument.
*   **Oscillator Quality:**
    *   Implement Band-Limited Step (BLEP) oscillators to reduce aliasing artifacts at high frequencies.
*   **Noise Generator:**
    *   Add White/Pink noise generators (crucial for synthesized percussion like snares and hats).

## 🔊 Audio Engine Architecture
*   **Streaming Renderer:**
    *   Refactor `AudioRenderer` to generate audio in small chunks on-demand rather than rendering the entire song to RAM.
    *   *Enables:* Infinite playback, lower memory usage, and real-time interaction.
*   **Multi-Channel Support:**
    *   Expand beyond Stereo (2.0) to 5.1 or 7.1 surround sound.
*   **Spatial Audio (Long Term):**
    *   Explore Object-Based Audio (Eclipsa/IAMF) for 3D spatial positioning.

## 📱 Platform & Portability
*   **Mobile Support (Android/iOS):**
    *   Create JNI (Android) and Objective-C++ (iOS) wrappers.
    *   Requires the "Streaming Renderer" refactor above.
*   **WebAssembly (Wasm):**
    *   Compile Museq to Wasm to run directly in a web browser.

## 🛠️ Core & Scripting
*   **JSON Deserialization:**
    *   Implement `JsonSerializer::load` to allow re-generating audio from the intermediate JSON format without re-parsing the script.
*   **Advanced Scripting:**
    *   Variables (e.g., `set $base_freq 440`).
    *   Arithmetic (e.g., `note $base_freq * 1.5`).
    *   Conditional logic (`if`).
