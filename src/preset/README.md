# src/preset/

Patch and preset management UI for Pedalboard3. This folder contains the components for organising patches within a document, importing and exporting per-plugin preset files, and configuring application preferences. It exists to give the user control over multi-patch documents (`.pdl` files) and individual plugin presets (`.fxp` files) as well as global settings.

## Contents

- `PatchOrganiser.cpp` / `PatchOrganiser.h` – `PatchOrganiser` component for managing the patch list inside a document. Displays a `ListBox` of patches with buttons to add, copy, remove, reorder, and import patches from other `.pdl` files. Patch names are editable by double-clicking.
- `PresetManager.cpp` / `PresetManager.h` – `PresetManager` class for user-saved plugin presets. Imports presets from `.fxp` files into a plugin processor and saves plugin state to `.fxp` files under the user data directory (`<user data>/Pedalboard3/presets/<plugin name>/`).
- `PresetBar.cpp` / `PresetBar.h` – `PresetBar` component shown on each plugin. Displays a combo box with factory and user-saved presets plus import and save buttons; loads and applies presets via `PresetManager`.
- `UserPresetWindow.cpp` / `UserPresetWindow.h` – `UserPresetWindow` component for full user-preset management. Presents a `TreeView` of plugin directories and their `.fxp` presets with buttons to copy, remove, import, export, and rename presets.
- `PreferencesDialog.cpp` / `PreferencesDialog.h` – `PreferencesDialog` component for application preferences. Configures OSC port and multicast address, visible I/O nodes (audio, MIDI, OSC), MIDI options (program change, MMC transport), and other options (auto mappings window, loop patches, windows on top, ignore pin names, tray icon, start in tray, fixed-size windows, save audio settings in `.pdl` files).

## Integration

`PatchOrganiser` is instantiated by `app/MainPanel` in its `EditOrganisePatches` command handler and shown modally; it operates on the shared `Array<XmlElement*>` patch array owned by `MainPanel`. `UserPresetWindow` is likewise instantiated by `MainPanel` in its `EditUserPresetManagement` command handler and shown modally; it receives the `KnownPluginList` from `MainPanel` to populate the import plugin selector. `PresetBar` is held by the `EditorWrapper` inner class of `canvas/PluginComponent` and is embedded above each plugin editor. `PresetManager` is used by `PresetBar` for `.fxp` import and save operations. `PreferencesDialog` is instantiated by `MainPanel` in its `OptionsPreferences` command handler; it reads and writes settings through `util/PropertiesSingleton` (JUCE `PropertiesFile`) and calls `MainPanel` methods to apply changes live. All components in this folder use `lookandfeel/ColourScheme` for colours.
