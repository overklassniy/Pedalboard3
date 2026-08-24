# src/

Source code for Pedalboard3.

## Contents

- `App.cpp` / `App.h` — JUCE application entry point and main window
- `MainPanel.cpp` / `MainPanel.h` — main UI container (menu bar, patch bar, transport, canvas viewport)
- `BranchesLAF.cpp` / `BranchesLAF.h` — custom LookAndFeel (ported from Pedalboard2)
- `ColourScheme.cpp` / `ColourScheme.h` — theme colour system
- `Vectors.cpp` / `Vectors.h` — embedded SVG vector graphics data for UI buttons
- `Images.cpp` / `Images.h` — embedded PNG image data (application icons)
- `LookAndFeelImages.cpp` / `LookAndFeelImages.h` — embedded images used by BranchesLAF (folder icon, magnifying glass SVG)
- `FilterGraph.cpp` / `FilterGraph.h` — AudioProcessorGraph wrapper (audio engine)
- `PluginField.cpp` / `PluginField.h` — canvas component for plugin nodes and connections
- `PluginComponent.cpp` / `PluginComponent.h` — individual plugin node UI
- `MidiMappingManager.cpp` / `MidiMappingManager.h` — MIDI CC to parameter mapping
- `OscMappingManager.cpp` / `OscMappingManager.h` — OSC to parameter mapping (uses juce_osc)
- `PatchOrganiser.cpp` / `PatchOrganiser.h` — patch management and navigation (list, add, copy, remove, reorder, import)
- `PresetManager.cpp` / `PresetManager.h` — user-saved plugin preset import/export (.fxp files)
- `PresetBar.cpp` / `PresetBar.h` — preset bar component shown on each plugin (combo box, import, save)
- `UserPresetWindow.cpp` / `UserPresetWindow.h` — user preset management window (tree view of plugins and presets)
- `PreferencesDialog.cpp` / `PreferencesDialog.h` — preferences dialog (OSC, I/O nodes, MIDI, tray icon, window options)
- `ColourSchemeEditor.cpp` / `ColourSchemeEditor.h` — colour scheme editor (list, selector, preset save/load/delete)
- `TapTempoBox.cpp` / `TapTempoBox.h` — tap tempo component (click to set BPM)
- `TrayIcon.cpp` / `TrayIcon.h` — system tray icon with popup menu (Windows/Linux only)
- `PropertiesSingleton.cpp` / `PropertiesSingleton.h` — singleton wrapper for ApplicationProperties
- `JuceHelperStuff.cpp` / `JuceHelperStuff.h` — helper functions (app data folder path)
- `scanner/` — out-of-process plugin scanner entry point

## Integration

The source tree is organized by feature area: application shell, audio core,
plugin components, mapping managers, UI components, built-in processors, and
stability infrastructure. All files are compiled into the `Pedalboard3` GUI
application target, except `scanner/PluginScannerMain.cpp` which builds the
separate `Pedalboard3Scanner` console application.

## Conventions

- C++20, JUCE 8.0.15 APIs
- All comments in English
- Code style defined by `.clang-format` at the repo root
