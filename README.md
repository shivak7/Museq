# Museq - Music Sequencer & Synthesizer

Museq is a cross-platform, text-based music sequencer and synthesizer written in C++17. It allows you to compose music using a simple, human-readable scripting language, supporting both synthesized sounds (subtractive synthesis) and sample-based instruments (SoundFonts, WAV samples).

It features a powerful hierarchical composition model (parallel/sequential tracks), automated looping workflows, and studio-quality stereo output (WAV, MP3, OGG).

## Getting Started

### Prerequisites
*   **CMake** (3.10+)
*   **C++ Compiler** (GCC, Clang, MSVC) supporting C++17
*   **Linux:** `libsndfile1-dev`, `libvorbis-dev`, `libogg-dev`, `libmp3lame-dev`
*   **Windows:** Dependencies are handled automatically via `vcpkg`.

### Build
```bash
mkdir build
cd build
cmake ..
make
```

### Run
```bash
./museq my_song.museq
```

## Command Line Usage

```bash
museq <script_file> [options]
```

| Flag | Long Flag | Description |
| :--- | :--- | :--- |
| `-o <name>` | `--out <name>` | Specify output filename base (default: "song"). Extension is appended automatically. |
| `-f <fmt>` | `--format <fmt>` | Output format: `wav`, `mp3`, `ogg` (default: `wav`). |
| `-q <hz>` | `--quality <hz>` | Sample rate in Hz (default: 44100). |
| `-p` | `--playback` | Render to a temporary file and play immediately via system audio (ignores `-o`). |
| `-d` | `--dump-json` | Dump the internal song structure to `<output_base>.json` for debugging. |

**Example:**
```bash
./museq -o mysong -f mp3 -q 48000 composition.museq
```

---

## Scripting Language Guide

Museq scripts are whitespace-insensitive but line-oriented. Comments start with `//`.

### 1. Defining Instruments

#### Synthesizers
Define a subtractive synth with an envelope, filter, and LFO.

```museq
instrument BassSyn {
    waveform sawtooth       // sine, square, triangle, sawtooth
    envelope 0.01 0.2 0.8 0.1 // Attack, Decay, Sustain, Release
    
    // Filters (Optional)
    filter lowpass 1000 2.0 // Type (lowpass/highpass/bandpass), Cutoff (Hz), Resonance (Q)
    
    // LFO Modulation (Optional)
    lfo cutoff sine 4 500   // Target (pitch/amplitude/cutoff), Waveform, Freq (Hz), Amount
                            // Amount: Hz for cutoff, Semitones for pitch, 0-1 for amplitude
    
    pan -0.5                // -1.0 (Left) to 1.0 (Right)
}
```

#### SoundFonts (SF2)
Use high-quality sampled instruments.

```museq
instrument Piano {
    soundfont "sounds/GeneralUser GS v1.471.sf2"
    bank 0
    preset 0
}
```

#### Samples (WAV)
Play a single sample.

```museq
instrument Kick {
    sample "sounds/kick.wav"
}
```

### 2. Defining Functions (Templates)
Functions allow you to reuse instrument definitions or musical patterns.

```museq
function DrumKit {
    instrument Kick { ... }
    instrument Snare { ... }
}
```

### 3. Sequencing Notes

#### Standard Syntax
Explicitly define pitch, duration (ms), and velocity (0-127).

```museq
instrument Piano {
    sequence {
        note C4 500 100
        note E4 500 100
        note G4 1000 120
    }
}
```

#### Compact Syntax
Define multiple notes on one line.
Format: `Pitch(Duration, Velocity, [Pan])`.
Defaults: Duration and Velocity use global defaults if omitted.

```museq
tempo 120
velocity 100

instrument Synth {
    notes C4, E4(250), G4(500, 120), C5(1000, 127, 0.5)
}
```

#### Rests
Use `R` or `Rest` to insert silence.

```museq
note R 1000 0
```

### 4. Composition Flow

#### Sequential & Parallel
Structure your song using blocks.

*   `sequential`: Plays children one after another.
*   `parallel`: Plays children simultaneously.

```museq
sequential {
    // Intro
    parallel {
        Piano { ... }
        Bass { ... }
    }
    // Verse
    parallel {
        Piano { ... }
        Drums { ... }
    }
}
```

#### Repeats
Repeat a block N times.

```museq
repeat 4 {
    Drums { ... }
}
```

### 5. Advanced Workflow: Auto-Looping
A powerful feature for backing tracks. You define a "Loop Leader" (foreground) and "Loop Followers" (background). The followers automatically repeat to match the duration of the leader.

```museq
// Define background patterns
function DrumBeat { ... }
function BassLine { ... }

// Start the context. Drums and Bass will loop automatically.
loop start DrumBeat, BassLine

    // This is the "Leader". The loop runs as long as this block lasts.
    instrument SoloViolin {
        sequence {
            note C5 2000 100
            note D5 2000 100
            // ...
        }
    }
    
    // Insert a break (silence) where background loops are heard alone
    Rest {
        note R 2000 0
    }

loop stop
```

---

## Built-in Instruments

*   **Rest**: A utility instrument that produces silence. Useful for timing in loops.
    ```museq
    Rest {
        note R 1000 0
    }
    ```

## License
MIT
