# src/util/

Utilities and embedded resource data for Pedalboard3. Contains singleton
wrappers for application properties and logging, helper functions for
common JUCE tasks, and auto-generated binary data files providing
embedded SVG vector graphics, PNG images, and look-and-feel images used
throughout the UI.

## Contents

- `PropertiesSingleton.cpp` / `PropertiesSingleton.h` – `PropertiesSingleton`, a singleton wrapper for JUCE's `ApplicationProperties` providing a single global instance accessible from anywhere in the codebase
- `LogFile.cpp` / `LogFile.h` – `LogFile`, a singleton `ChangeBroadcaster` that logs MIDI, OSC, and Pedalboard events to a date-stamped file in the application data directory using `juce::FileOutputStream`; broadcasts change notifications so UI components can update in real time
- `JuceHelperStuff.cpp` / `JuceHelperStuff.h` – `JuceHelperStuff` namespace with helper functions: `loadSVGFromMemory`, `showModalDialog`, `showNonModalDialog`, and `getAppDataFolder`
- `Vectors.cpp` / `Vectors.h` – `Vectors` namespace providing embedded SVG vector graphics data (button icons: add mapping, bypass, close filter, delete, new, open, save, output toggle, play, pause, stop, record, return-to-zero) as `const char*` arrays with size constants; auto-generated binary data
- `Images.cpp` / `Images.h` – `Images` namespace providing embedded PNG image data (16x16 and 512x512 application icons) as `const char*` arrays with size constants; auto-generated binary data
- `LookAndFeelImages.cpp` / `LookAndFeelImages.h` – `LookAndFeelImages` namespace providing embedded image data used by the custom look and feel (32x32 folder icon PNG, magnifying glass SVG) as `const char*` arrays with size constants; auto-generated binary data

## Integration

These utilities are used across the entire application. `PropertiesSingleton`
is accessed from `app/MainPanel`, `app/PluginListWindow`, `preset/PreferencesDialog`,
and many other components for persistent user settings. `LogFile` is accessed
by `app/LogDisplay` for the log viewer and by various components that log
MIDI, OSC, and Pedalboard events.

`JuceHelperStuff` provides `getAppDataFolder` used by `stability/SettingsManager`
and other persistence code, `loadSVGFromMemory` used by `lookandfeel/BranchesLAF`
and other components to load embedded SVG data, and dialog helpers used
throughout the UI.

`Vectors`, `Images`, and `LookAndFeelImages` provide embedded graphics data
consumed by `lookandfeel/BranchesLAF`, `canvas/PluginComponent`, `app/MainPanel`,
and other UI components. The application icon (`Images::icon512_png`) is also
used by `app/PluginListWindow` and `app/App` for window icons.

All files in this directory are compiled into the `Pedalboard3` GUI
application target.

## Constraints

- `Vectors.cpp`, `Images.cpp`, and `LookAndFeelImages.cpp` are auto-generated binary data files. Do not edit them by hand; regenerate them from the source SVG/PNG assets when the graphics change.
- `LogFile` uses `juce::FileOutputStream` for file output, not spdlog or any third-party logging library.
