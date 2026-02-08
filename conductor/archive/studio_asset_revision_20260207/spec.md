# Specification: Studio Asset Management Revision

## Overview
The current Left Dock in Muqomposer Studio relies on hardcoded data for the "Synths" tab and has inconsistent logic for the "Add Folder" functionality. This track aims to replace the hardcoded synth list with a dynamic, filesystem-driven system that parses `.museq` files for instrument templates. Additionally, the "Add Folder" logic will be unified to recursively scan and categorize all supported asset types (`.sf2`, `.wav`, `.mp3`, `.ogg`, `.museq`) into their respective UI sections.

## Functional Requirements

### 1. Dynamic Synths Tab
- **Remove Hardcoding:** Eliminate the hardcoded `synth_templates` vector (`"SawLead"`, etc.) from `main.cpp`.
- **Parsing:** The `AssetManager` must scan watched folders for `.museq` files.
- **Instrument Extraction:** For each `.museq` file found, the system must parse the file to identify `instrument` blocks.
- **Display:** The "Synths" tab must display a tree structure:
    - Top Level: Filename (e.g., `drums.museq`)
    - Children: Instrument names defined in that file (e.g., `SynthKick`, `SynthSnare`).
- **Interaction:** Clicking an instrument name should insert the appropriate code into the editor:
    - If the file is not imported: Insert `import "path/to/file.museq"` at the top.
    - Insert the usage code (e.g., `SynthKick` in a sequence or as a new definition if it's a template copy). **Defaulting to: Copying the instrument definition code** (matching current behavior but with real data).

### 2. Unified Folder Scanning ("Add Folder")
- **Recursive Scan:** When a folder is added via "Add Folder", `AssetManager` must recursively scan it.
- **Automatic Categorization:** Files found must be distributed to the correct internal lists based on extension:
    - `.sf2` -> SoundFonts
    - `.wav`, `.mp3`, `.ogg` -> Samples
    - `.museq` -> Synths
- **Persistence:** Watched folders must be saved/loaded from config (already partially implemented, ensure it covers all types).

### 3. Left Dock Logic Fixes
- **Populate Samples:** Ensure the "Samples" tab correctly populates from `m_samples` list gathered during the unified scan.
- **Populate Synths:** Ensure the "Synths" tab populates from the new `m_synths` (or equivalent) list.
- **UI Structure:** Maintain the current `CollapsingHeader` structure ("Active", "SoundFonts", "Samples", "Synths") but ensure they are backed by real data.

## Non-Functional Requirements
- **Performance:** Parsing `.museq` files should be reasonably fast or cached. Don't re-parse every frame. Parse on scan/refresh.
- **Robustness:** Handle malformed `.museq` files gracefully (skip or log error, don't crash).

## Acceptance Criteria
- [ ] The "Synths" tab no longer shows "SawLead", "SoftPad", etc.
- [ ] The "Synths" tab lists `drums.museq`, `leads.museq` etc. from the `musynths/` folder (if watched).
- [ ] Expanding `drums.museq` in the UI shows `SynthKick`, `SynthSnare`, `SynthHat`.
- [ ] Clicking a synth instrument inserts its code into the editor.
- [ ] Adding a folder containing mixed assets correctly populates all three tabs (SoundFonts, Samples, Synths).
