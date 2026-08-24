# icon/

Application icons for Pedalboard3.

## Contents

- `icon-16.png`, `icon-32.png`, `icon-48.png`, `icon-256.png`,
  `icon-512.png` — PNG icons at various resolutions
- `icon.ico` — Windows icon file
- `vectorIcon.svg` — source SVG for the icon
- `oldIcon/` — previous icon set (kept for reference)

## Integration

The icons are referenced by `CMakeLists.txt` in the `juce_add_gui_app` call
(`ICON_BIG` and `ICON_SMALL`). The `.ico` file is used for the Windows
executable resource.
