# Museq GUI Prototype (Dear ImGui)

This is a boilerplate prototype for the upcoming Museq Studio interface. It is designed to work with **Dear ImGui** and **GLFW**.

## Prerequisites

1.  **Install GLFW:**
    *   **Linux:** `sudo apt install libglfw3-dev`
    *   **macOS:** `brew install glfw`
    *   **Windows:** `vcpkg install glfw3:x64-windows`

2.  **Download Dear ImGui:**
    *   Download the latest release from [ocornut/imgui](https://github.com/ocornut/imgui).
    *   Place the following files into `third_party/imgui/`:
        *   `imgui.cpp`, `imgui.h`, `imgui_draw.cpp`, etc.
        *   `backends/imgui_impl_glfw.cpp` / `.h`
        *   `backends/imgui_impl_opengl3.cpp` / `.h`

## Proposed Features
*   **Live Editor:** Real-time syntax highlighting and editing of `.museq` scripts.
*   **Oscilloscope:** Visual feedback of the current audio buffer using `ImGui::PlotLines`.
*   **Instrument Inspector:** Quick view of all imported templates and their parameters.

## Building (Conceptual)
```bash
mkdir build_gui
cd build_gui
cmake ../muqomposer
make
./museq-gui
```
