# assets/

Visual asset sources for Pedalboard3.

## Contents

- `icons/` — application icons (PNG at multiple resolutions, Windows `.ico`,
  and the source `vectorIcon.svg`)
- `vectors/` — SVG vector graphics for UI buttons and controls

## Integration

The asset files in this directory are the source-of-truth for the binary
resource data embedded into the application at build time:

- `icons/` is referenced by `CMakeLists.txt` (`ICON_BIG`, `ICON_SMALL`) and
  the 16x16 / 512x512 PNGs are embedded in `src/util/Images.cpp`.
- `vectors/` SVGs are embedded in `src/util/Vectors.cpp` and rendered at
  runtime via `juce::Drawable::createFromSVG`.

Because the binary data is compiled into the executable, the files in
`assets/` are not read from disk at runtime; they are only needed at build
time to regenerate the embedded data.
