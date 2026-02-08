# Initial Concept
Museq is a text-based music sequencer and synthesizer written in C++17. It allows for hierarchical composition and high-quality audio output via a custom scripting language.

# Product Definition - Museq & Muqomposer

## Vision
Museq is a cross-platform, text-based music sequencer and synthesizer designed for musicians, composers, and sound designers who prefer a "code-first" workflow. It bridges the gap between programmatic music generation and traditional synthesis, providing a transparent, human-readable scripting language that eliminates the "blackbox" effect often associated with automated music design.

## Target Audience
- **Code-First Composers:** Musicians who find creative freedom in text-based structures and hierarchical composition.
- **Sound Designers:** Professionals seeking a lightweight, programmable synthesizer for building custom subtractive and sample-based instruments.
- **AI-Augmented Artists:** Users interested in exploring AI-guided music design where the logic remains visible and editable in script form.

## Core Features
- **Hierarchical & Theoretical Composition:** A powerful model using `parallel`, `sequential`, and `auto-looping` blocks, now enhanced with first-class support for `scale` definitions and built-in `chord` name expansion.
- **Hybrid Synthesis Engine:** Supports subtractive synthesis (oscillators, ADSR, filters, LFOs) alongside high-quality sampled instruments (SoundFonts and WAV/MP3/OGG samples).
- **Studio-Quality Output:** Direct-to-disk rendering for WAV, MP3, and OGG formats with an advanced export workflow, normalization, and high-fidelity stereo processing.
- **Dual-Interface Workflow:** A feature-compatible ecosystem where the `museq` CLI provides automation and speed, while the `muqomposer` Studio offers real-time visual feedback, asset management, and interactive playback.
- **Studio Ergonomics:** Global asset search, keyboard shortcuts (Ctrl+P, Ctrl+S, etc.), customizable font sizes, and a professional code editor with syntax highlighting and autocomplete.

## Development Goals
- **Musical Intelligence:** Enhancing the scripting language with scale-aware and chord-aware functions to simplify complex music theory logic.
- **Interactive Studio:** Improving Muqomposer with an advanced code editor featuring syntax highlighting, autocomplete, real-time error reporting, and playback tracking.
- **Engine Expansion:** Developing a more sophisticated synthesis engine with additional oscillator types and a comprehensive modulation matrix.
- **Transparent AI Integration:** Creating hooks for AI-guided design that output readable Museq scripts, ensuring the user always retains full control and understanding of the generated music.
