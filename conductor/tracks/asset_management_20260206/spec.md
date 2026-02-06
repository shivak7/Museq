# Specification - Advanced Asset Management

## Overview
This track focuses on transforming the Muqomposer Left Panel into a robust asset manager. It will support dynamic folder loading, tree-based navigation, real-time filtering, and auditioning capabilities.

## User Stories
- As a sound designer, I want to load my own directories of SoundFonts and samples so I can easily use my library.
- As a composer, I want to search for specific instruments by name across all loaded folders.
- As a user, I want to hear a 'demo' of a SoundFont preset or sample before I commit to adding it to my script.
- As a user, I want to drag an asset from the sidebar into the editor to automatically generate the required instrument code at the cursor location.

## Functional Requirements
- **Folder Loader:** Buttons to trigger a directory selection (using our internal file browser) to add to the 'Available' list.
- **Categorization:** Logic to sort discovered files into 'SoundFonts' and 'Samples' while maintaining a tree structure.
- **Search:** A real-time text filter at the top of the sidebar.
- **Audio Preview:** Integrated button/icon next to assets that triggers a brief playback using the engine's `AudioPlayer`.
- **Drag & Drop:** Implementation of ImGui drag-and-drop payloads for asset paths.
- **Favorites:** A 'Star' system to pin assets to a top-level 'Favorites' category.
