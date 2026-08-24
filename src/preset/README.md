# src/preset/

Patch and preset management for Pedalboard3.

## Contents

- `PatchOrganiser.cpp` / `PatchOrganiser.h` — patch management and navigation (list, add, copy, remove, reorder, import patches within a document)
- `PresetManager.cpp` / `PresetManager.h` — user-saved plugin preset import/export (.fxp files)
- `PresetBar.cpp` / `PresetBar.h` — preset bar component shown on each plugin (combo box, import, save)
- `UserPresetWindow.cpp` / `UserPresetWindow.h` — user preset management window (tree view of plugins and presets)
- `PreferencesDialog.cpp` / `PreferencesDialog.h` — preferences dialog (OSC port, I/O nodes, MIDI, tray icon, window options)

## Integration

`PatchOrganiser` is owned by `app/MainPanel` and manages the patch list
within a document (a .pdl file containing multiple patches). `PresetManager`
handles per-plugin preset files (.fxp). `PresetBar` is embedded in each
`canvas/PluginComponent` for quick preset switching. `UserPresetWindow`
provides a full preset management UI. `PreferencesDialog` is opened from the
Options menu and reads/writes settings via `stability/SettingsManager`.
