# Technology Stack - Museq & Muqomposer

## Core Language & Build
- **Language:** C++17 (Modern C++ features for hierarchical data structures).
- **Build System:** CMake 3.10+ with optional `BUILD_TESTS` flag for developer environments.

## Audio Engine
- **Synthesis:** Custom subtractive synthesis implementation.
- **SoundFonts:** [TinySoundFont (TSF)](https://github.com/schellingb/TinySoundFont) for high-quality MIDI instrument rendering.
- **Real-time Playback:** [miniaudio](https://github.com/mackron/miniaudio) for cross-platform low-latency audio output.
- **File Export:**
    - `libsndfile` for WAV and general audio I/O.
    - `libvorbis` & `libogg` for OGG encoding.
    - `LAME` for MP3 encoding.

## Studio GUI (Muqomposer)
- **Framework:** [Dear ImGui](https://github.com/ocornut/imgui) for immediate-mode GUI.
- **Editor Component:** [ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit) for the advanced code editor.
- **Image Loading:** [stb_image](https://github.com/nothings/stb) for loading branding assets and icons.
- **Windowing & Input:** [GLFW](https://www.glfw.org/) for window management and event handling.
- **Graphics API:** OpenGL (via `OpenGL::GL` or system-specific drivers).

## Data & Utilities
- **Serialization:** [nlohmann/json](https://github.com/nlohmann/json) for dumping song structures and configuration.
- **Script Parsing:** Custom hand-written parser for the Museq scripting language.
- **Filesystem:** `std::filesystem` (C++17) for asset discovery and file dialogs.
