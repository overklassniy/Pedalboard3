# assets/icons/

Application icons for Pedalboard3.

## Contents

- `icon-16.png`, `icon-32.png`, `icon-48.png`, `icon-256.png`,
  `icon-512.png` — PNG icons at various resolutions
- `icon.ico` — Windows icon file
- `vectorIcon.svg` — source SVG for the icon

## Integration

The icons are referenced by `CMakeLists.txt` in the `juce_add_gui_app` call
(`ICON_BIG` uses `icon-512.png`, `ICON_SMALL` uses `icon-48.png`). The
`icon.ico` file is used for the Windows executable resource.

The 16x16 and 512x512 PNGs are also embedded as binary data in
`src/util/Images.cpp` and accessed at runtime through the `Images` namespace
declared in `src/util/Images.h`.
