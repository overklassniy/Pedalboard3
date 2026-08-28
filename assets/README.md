# assets/

Visual asset sources for Pedalboard3. This folder holds the original raster
icons and vector graphics that are compiled into the application as embedded
binary data; the files are not read from disk at runtime.

## Contents

- `icons/` – application icons: PNG rasters at multiple resolutions, a Windows
  `.ico` file, and the source `vectorIcon.svg`
- `vectors/` – SVG vector graphics for UI buttons and controls
- `help/` – single self-contained HTML file for the in-application F1 help,
  embedded via `juce_add_binary_data` (see `help/README.md`)
- `readme/` – visual assets for the root `README.md` (not embedded into the
  application; see `readme/README.md`)

## Integration

The asset files in this directory are the source-of-truth for the binary
resource data embedded into the application at build time:

- `icons/` is referenced by `CMakeLists.txt` (`ICON_BIG` uses
  `assets/icons/icon-512.png`, `ICON_SMALL` uses
  `assets/icons/icon-48.png`). The 16x16 and 512x512 PNGs are also embedded
  as binary data in `src/util/Images.cpp` and exposed through the `Images`
  namespace declared in `src/util/Images.h`.
- `vectors/` SVGs are embedded as binary data in `src/util/Vectors.cpp` and
  exposed through the `Vectors` namespace declared in `src/util/Vectors.h`.
  At runtime they are loaded via
  `JuceHelperStuff::loadSVGFromMemory` (defined in
  `src/util/JuceHelperStuff.cpp`), which wraps
  `juce::Drawable::createFromSVG`.
- `help/help.html` is embedded via `juce_add_binary_data` (namespace
  `HelpData`, header `HelpData.h`) in `CMakeLists.txt`. At runtime the
  `HelpDocumentation` command in `src/app/MainPanel.cpp` writes the
  embedded HTML to the application data folder and launches it in the
  default browser on F1.
- `readme/` is not embedded into the application and is not referenced by
  the build system. It holds visual assets for the root `README.md` only.

## Constraints

Because the binary data is compiled into the executable, the files in
`assets/` are not read from disk at runtime; they are only needed at build
time to regenerate the embedded data in `src/util/Images.cpp` and
`src/util/Vectors.cpp`.
