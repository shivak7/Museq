# Product Guidelines - Museq & Muqomposer

## Scripting & Language Philosophy
- **Precision with Musicality:** The Museq language must balance technical accuracy (exact frequency/envelope control) with intuitive music theory concepts (BPM, scales, chords).
- **Minimalist Syntax:** Prioritize concise keywords and compact note syntax to enable a "live-coding" feel where complex patterns can be typed quickly.
- **Explanatory Feedback:** Both the CLI and GUI must provide verbose, line-specific error messages to guide the user in correcting script logic immediately.

## Studio (GUI) Design Principles
- **Dark Studio Aesthetic:** Utilize a high-contrast dark theme optimized for long hours of focused composition and sound design.
- **Task-Oriented Modularity:** The interface should remain resizable and collapsible, allowing the user to switch focus between the Code Interface, Asset Browser, and Visualizers.
- **Asset Transparency:** Provide a seamless experience for managing and auditioning assets. Users must be able to explore SoundFont presets and directory-based samples with the ability to "demo" sounds before importing them into a script.
- **Immediate Visual Feedback:** Maintain a tight loop between code changes and visual results. Real-time waveform visualization and active instrument tracking are core to the Studio experience.

## User Experience (UX) Goals
- **Eliminate the Black Box:** Ensure that every generated or AI-assisted element is ultimately represented by transparent, editable Museq code.
- **Cross-Platform Consistency:** The core engine behavior and Studio layout must remain identical across Linux, macOS, and Windows.
- **Asset Discovery:** Automate the scanning of SoundFonts and sample directories to reduce the friction of finding and utilizing external sound libraries.
