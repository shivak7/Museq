# <img src="Museq_logo.png" width="100" align="left" /> Museq - Music Sequencer & Synthesizer

Museq is a cross-platform, text-based music sequencer and synthesizer written in C++17. It allows you to compose music using a simple, human-readable scripting language, supporting both synthesized sounds (subtractive synthesis) and sample-based instruments (SoundFonts, WAV samples).

It features a powerful hierarchical composition model (parallel/sequential tracks), automated looping workflows, and studio-quality stereo output (WAV, MP3, OGG).

---

## Contents
- [Getting Started](#getting-started)
    - [Download Binaries](#download-binaries)
    - [Prerequisites](#prerequisites)
    - [Build Instructions](#build-instructions)
    - [Run](#run)
- [Assets & Working Directory](#assets--working-directory)
- [Command Line Usage](#command-line-usage)
- [Scripting Language Guide](#scripting-language-guide)
    - [1. Importing Instruments](#1-importing-instruments)
    - [2. Defining Instruments](#2-defining-instruments)
    - [3. Defining Functions (Templates)](#3-defining-functions-templates)
    - [4. Global Variables](#4-global-variables)
    - [5. Music Theory (Scales & Chords)](#5-music-theory-scales--chords)
    - [6. Sequencing Notes](#6-sequencing-notes)
    - [7. Composition Flow](#7-composition-flow)
    - [8. Advanced Workflow: Auto-Looping](#8-advanced-workflow-auto-looping)
- [Built-in Instruments](#built-in-instruments)
- [License](#license)

---

## Getting Started

### Download Binaries
You can download the latest pre-built binaries automatically:

*   **[Download for Windows (x64)](https://nightly.link/shivak7/Museq/workflows/build.yml/master/museq-windows-x64.zip)**
*   **[Download for macOS (Intel x64)](https://nightly.link/shivak7/Museq/workflows/build.yml/master/museq-macos-x64.zip)**
*   **[Download for macOS (Apple Silicon)](https://nightly.link/shivak7/Museq/workflows/build.yml/master/museq-macos-arm64.zip)**
*   **[Download for Linux (Ubuntu x64)](https://nightly.link/shivak7/Museq/workflows/build.yml/master/museq-ubuntu-x64.zip)**

*(Powered by [nightly.link](https://nightly.link))*

### Prerequisites
*   **CMake** (3.10+)
*   **C++ Compiler** (GCC, Clang, MSVC) supporting C++17
*   **Git**

### Build Instructions

#### Linux
Install system dependencies:
```bash
sudo apt update
sudo apt install build-essential cmake libsndfile1-dev libvorbis-dev libogg-dev libmp3lame-dev libflac-dev libopus-dev libmpg123-dev
```

Build the project:
```bash
mkdir build
cd build
cmake ..
make
```

#### macOS
Install dependencies via Homebrew:
```bash
brew install cmake libsndfile libvorbis libogg lame flac opus mpg123
```

Build the project:
```bash
mkdir build
cd build
cmake ..
make
```

#### Windows
Dependencies are handled via **vcpkg**.

1.  **Install vcpkg:**
    ```powershell
    git clone https://github.com/microsoft/vcpkg
    .\vcpkg\bootstrap-vcpkg.bat
    ```

2.  **Install Libraries:**
    
    *Option A: Visual Studio (MSVC) - Recommended*
    ```powershell
    .\vcpkg\vcpkg install libsndfile libvorbis libogg mp3lame libflac opus mpg123 --triplet x64-windows
    ```

    *Option B: MinGW / MSYS2*
    ```powershell
    .\vcpkg\vcpkg install libsndfile libvorbis libogg mp3lame libflac opus mpg123 --triplet x64-mingw-static
    ```

3.  **Build with CMake:**

    **If using Visual Studio:**
    Open the "Developer Command Prompt for VS 2022" and run:
    ```cmd
    mkdir build
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
    cmake --build . --config Release
    ```

    **If using MSYS2 / MinGW:**
    If you see an error like `'nmake' ... failed`, you need to explicitly specify the generator:
    ```bash
    mkdir build
    cd build
    cmake .. -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
    cmake --build .
    ```
    *(Replace `[path/to/vcpkg]` with the actual path to your vcpkg installation).*

### Run
#### CLI
```bash
./build/bin/museq muscripts/my_song.museq
```

#### Studio (GUI)
```bash
./build/bin/muqomposer
```

## Assets & Working Directory

Museq resolves file paths (for SoundFonts and samples) relative to the **Current Working Directory** where you run the executable.

To ensure your scripts run correctly:
1.  **Run from the Project Root:** This is the easiest way. Run `./build/bin/museq muscripts/song.museq`.
2.  **Copy Assets:** Alternatively, copy the `sounds/` and `muscripts/` folders into your `build/bin/` (or `Release/`) directory and run `./museq song.museq`.

Ensure that the path specified in your `.museq` script (e.g., `soundfont "sounds/myfont.sf2"`) actually exists relative to where you are running the command.

**Additional Resources:**
*   **[Free SoundFonts & Samples (Internet Archive)](https://archive.org/details/free-soundfonts-sf2-2019-04)** - A collection of public domain and free-to-use SoundFonts and samples compatible with Museq.

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
| `-Q <sf2>` | `--query <sf2>` | List available instruments (presets) in a SoundFont file. |

**Example:**
```bash
./museq -o mysong -f mp3 -q 48000 composition.museq
```

---

## Scripting Language Guide

Museq scripts are whitespace-insensitive but line-oriented. Comments start with `//`.

### 1. Importing Instruments
You can import instrument definitions from other `.museq` files. This allows you to create reusable synthesizer libraries.

```museq
import "musynths/leads.museq"
import "musynths/drums.museq"

ClassicLead { notes C4 }
SynthKick { notes C2 }
```

*Note: The `import` keyword currently only imports instrument templates. Functions and variables defined in the imported file are ignored.*

### 2. Defining Instruments
All instruments must be declared at the top level of the script, before any execution blocks.

#### Synthesizers
Define a subtractive synth using the following properties:

| Property | Parameters | Description |
| :--- | :--- | :--- |
| `waveform` | `<type> [freq_mult]` | `sine`, `square`, `triangle`, `sawtooth`. Optional `freq_mult` defaults to 1.0. |
| `envelope` | `<a> <d> <s> <r>` | ADSR times in seconds (Attack, Decay, Release) and Sustain level (0.0-1.0). |
| `filter` | `<type> <freq> <q>` | `lowpass`, `highpass`, `bandpass`. Freq in Hz, Q resonance (e.g., 1.0). |
| `lfo` | `<target> <wave> <hz> <amt>` | Target: `pitch`, `amplitude`, `cutoff`. Wave: see `waveform`. |
| `pan` | `<value>` | Stereo position: -1.0 (Left) to 1.0 (Right). |
| `gain` | `<value>` | Master volume multiplier (e.g., 0.5 for half volume). |
| `portamento` | `<ms>` | Glide time in milliseconds between consecutive notes. |

```museq
instrument BassSyn {
    waveform sawtooth
    envelope 0.01 0.2 0.8 0.1
    filter lowpass 1000 2.0
    lfo cutoff sine 4 500
    pan -0.5
    gain 0.8
    portamento 200
}
```

#### Effects
Instruments can have multiple effects applied in a chain.

| Effect | Parameters | Description |
| :--- | :--- | :--- |
| `delay` | `<time_ms> <feedback>` | Echo effect. Feedback: 0.0 to 1.0. |
| `distortion` | `<drive>` | Hard clipping distortion. Drive: 1.0+. |
| `bitcrush` | `<bits>` | Reduces resolution. Bits: 1 to 16. |
| `fadein` | `<time_ms>` | Gradually increases volume at the start. |
| `fadeout` | `<time_ms>` | Gradually decreases volume at the end. |
| `tremolo` | `<rate_hz> <depth>` | Amplitude modulation. Depth: 0.0 to 1.0. |
| `reverb` | `<room_size> <damp>` | Spatial reverb. Room size and dampening: 0.0 to 1.0. |

```museq
instrument SpacePad {
    waveform triangle
    effect tremolo 5 0.5
    effect reverb 0.8 0.5
    effect delay 400 0.4
}
```

#### Sequence-level Effects
Effects can also be applied to entire blocks of music (sequential or parallel). This is useful for adding global reverb or distortion to a group of instruments.

```museq
sequential {
    effect reverb 0.7 0.4
    
    parallel {
        Piano { notes C4, E4 }
        Violin { notes G4, B4 }
    }
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

### 3. Defining Functions (Templates)
Functions allow you to reuse musical patterns.

```museq
function DrumKit {
    Kick { ... }
    Snare { ... }
}
```

### 4. Global Variables
Define reusable constants for BPM, volumes, or notes.

```museq
var BPM 128
var ROOT C4

tempo $BPM
instrument Synth { ... }

Synth {
    notes $ROOT, E4, G4
}
```

### 5. Music Theory (Scales & Chords)
Museq supports high-level music theory concepts to simplify composition.

#### Scale Definition
You can define a global or local scale to use relative note names.
**Syntax:** `scale <root> <mode>`
**Supported Modes:** `major`, `minor`, `dorian`, `phrygian`, `lydian`, `mixolydian`, `locrian`.

```museq
scale D minor

instrument Pad {
    waveform sine
    // Relative notes 1, 3, 5 resolve to D, F, A
    notes 1, 3, 5
}
```

#### Relative Note Notation
When a scale is active, use numbers `1` through `7` to refer to the degrees of that scale. This allows you to change the key of your entire song by modifying a single `scale` line.

#### Built-in Chords
You can use common chord names directly in your `notes` lists.
**Syntax:** `<Root><Quality>` (e.g., `Cmaj`, `Dm7`, `G7`, `Abmaj9`).

**Supported Qualities:**
- `maj`, `min`, `7`, `maj7`, `min7`, `dim`, `aug`
- `sus4`, `sus2`, `add9`, `maj9`, `min9`

```museq
instrument Piano {
    notes Cmaj, G7, Amin7, Fmaj
}
```

### 6. Sequencing Notes

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

**Repetition:** Use `*N` to repeat a note N times (e.g., `C4*4`).
**Chords:** Use `+` to play multiple notes simultaneously (e.g., `C4+E4+G4`).

```museq
tempo 120
velocity 100

instrument Synth {
    notes C4+E4+G4, F4+A4+C5*2
}
```

#### Octave Context
Use `octave` to set a default octave for subsequent notes.

```museq
octave 5
notes C, D, E  // Played as C5, D5, E5
```

#### Pan Context
Inside an instrument block, you can override the panning for subsequent notes.

```museq
Piano {
    pan -0.5
    notes C, D, E // Played at -0.5 pan
}
```

#### Rests & MIDI
Use `R` or `Rest` to insert silence. You can also use MIDI numbers (0-127) instead of note names. 

*Note: The pitch `0` is technically supported and often used as a very low-frequency pulse or silent marker depending on the instrument.*

```museq
note R 1000 0
notes 36, 38, 40 // C1, D1, E1
```

### 7. Composition Flow

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

#### Time Offset & Phase
Use `offset` to delay by milliseconds, or `phase` to delay by a fraction of a beat (relative to current tempo).

```museq
parallel {
    Piano { notes C4 }
    
    // Starts 500ms later
    offset 500 {
        Piano { notes G4 }
    }
    
    // Starts 0.5 beats later
    phase 0.5 {
        Piano { notes E4 }
    }
}
```

### 8. Advanced Workflow: Auto-Looping
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