# src/canvas/

Plugin canvas for Pedalboard3 — the visual graph editor where users place
and connect plugins.

## Contents

- `PluginComponent.cpp` / `PluginComponent.h` — individual plugin node UI (title bar, pins, preset bar, editor window, bypass/close buttons)
- `PluginField.cpp` / `PluginField.h` — canvas component for plugin nodes and connections (drag-to-connect, double-click to add, context menus, file drag-and-drop)

## Integration

`PluginField` is the canvas that displays all plugin nodes as
`PluginComponent` instances and renders connections between them. It is
owned by `app/MainPanel` (via a Viewport) and interacts with `audio/FilterGraph`
for graph mutations. `PluginComponent` uses `preset/PresetBar` for per-plugin
preset management and `processors/` editors for the built-in processors.
