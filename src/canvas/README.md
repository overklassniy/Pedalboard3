# src/canvas/

Plugin canvas for Pedalboard3 — the visual graph editor where users place,
connect, and rearrange plugins. This folder holds the components that render
the signal-path graph as draggable nodes with connectable pins.

## Contents

- `PluginComponent.h` / `PluginComponent.cpp` – the per-node UI for a single
  plugin/filter in the graph. Declares four classes: `PluginComponent` (the
  node itself, with title label, edit/bypass/delete buttons, and cached
  presets), `PluginPinComponent` (an input/output/parameter pin used to drag
  connections), `PluginEditorWindow` (a `DocumentWindow` wrapping a plugin
  editor and a `PresetBar`), and `PluginConnection` (the bezier curve drawn
  between two pins, with selection and hit-testing).
- `PluginField.h` / `PluginField.cpp` – `PluginField`, the canvas `Component`
  that owns the `MidiMappingManager` and `OscMappingManager`, displays all
  plugin nodes as `PluginComponent` instances, renders `PluginConnection`s
  between them, and handles double-click-to-add, drag-to-connect, file
  drag-and-drop, context menus, and patch save/load via XML.

## Integration

`PluginField` is the canvas viewed through a `Viewport` owned by
`app/MainPanel` (see `MainPanel::getMidiMappingManager`, which delegates to
`PluginField::getMidiManager`). It holds a pointer to `audio/FilterGraph` and
mutates the graph when nodes are added, deleted, or connected. It includes
`processors/PedalboardProcessors.h` so the built-in processor types are
visible. `PluginComponent` includes `preset/PresetBar.h` for per-plugin preset
management and `processors/PedalboardProcessors.h` to handle the built-in
processors. `PluginComponent` also opens `mappings/MappingsDialog` from its
mappings button.

## Constraints

- `PluginField` implements `juce::AudioPlayHead::getPosition` using the
  JUCE 8 `Optional<PositionInfo>` return type (not the deprecated
  `CurrentPositionInfo` struct); see the JUCE 8 notes in `src/README.md`.
