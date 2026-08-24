# src/

Source code for Pedalboard3.

## Contents

- `App.cpp` / `App.h` — JUCE application entry point and main window
- `MainPanel.cpp` / `MainPanel.h` — main UI container (menu bar, patch bar, transport, canvas viewport)
- `BranchesLAF.cpp` / `BranchesLAF.h` — custom LookAndFeel (ported from Pedalboard2)
- `ColourScheme.cpp` / `ColourScheme.h` — theme colour system
- `FilterGraph.cpp` / `FilterGraph.h` — AudioProcessorGraph wrapper (audio engine)
- `PluginField.cpp` / `PluginField.h` — canvas component for plugin nodes and connections
- `PluginComponent.cpp` / `PluginComponent.h` — individual plugin node UI
- `MidiMappingManager.cpp` / `MidiMappingManager.h` — MIDI CC to parameter mapping
- `OscMappingManager.cpp` / `OscMappingManager.h` — OSC to parameter mapping (uses juce_osc)
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
