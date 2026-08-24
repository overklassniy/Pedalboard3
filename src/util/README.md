# src/util/

Utilities and embedded resource data for Pedalboard3.

## Contents

- `PropertiesSingleton.cpp` / `PropertiesSingleton.h` — singleton wrapper for ApplicationProperties
- `LogFile.cpp` / `LogFile.h` — logging file writer (uses spdlog)
- `JuceHelperStuff.cpp` / `JuceHelperStuff.h` — helper functions (app data folder path)
- `Vectors.cpp` / `Vectors.h` — embedded SVG vector graphics data for UI buttons (auto-generated binary data)
- `Images.cpp` / `Images.h` — embedded PNG image data (auto-generated binary data)
- `LookAndFeelImages.cpp` / `LookAndFeelImages.h` — embedded images used by BranchesLAF (folder icon, magnifying glass SVG)

## Integration

These utilities are used across the entire application. `PropertiesSingleton`
and `LogFile` are accessed from many components. `Vectors` and `Images`
provide embedded graphics data consumed by `lookandfeel/BranchesLAF` and
various UI components. `JuceHelperStuff` provides the app data folder path
used by `stability/SettingsManager` and other persistence code.
