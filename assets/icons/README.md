# assets/icons/

Application icons for Pedalboard3. This folder holds the raster icon PNGs at
several resolutions, a Windows `.ico` bundle, and the source SVG from which
the raster icons were derived.

## Contents

- `icon-16.png` – 16x16 PNG application icon
- `icon-32.png` – 32x32 PNG application icon
- `icon-48.png` – 48x48 PNG application icon
- `icon-256.png` – 256x256 PNG application icon
- `icon-512.png` – 512x512 PNG application icon
- `icon.ico` – Windows multi-resolution icon bundle
- `vectorIcon.svg` – 512x512 source SVG for the application icon; contains an
  embedded PNG raster plus vector overlay shapes (rounded panels in light
  blue/purple with white inner areas and curved connecting paths) depicting
  the pedalboard application artwork

## Integration

The icons are referenced by `CMakeLists.txt` in the `juce_add_gui_app` call:
`ICON_BIG` is set to `assets/icons/icon-512.png` and `ICON_SMALL` is set to
`assets/icons/icon-48.png`. JUCE uses these PNGs to generate the platform
icon resources at build time.

The 16x16 and 512x512 PNGs are also embedded as binary data in
`src/util/Images.cpp` (`Images::icon16_png` and `Images::icon512_png`) and
accessed at runtime through the `Images` namespace declared in
`src/util/Images.h`.

The `icon.ico` file is not directly referenced by `CMakeLists.txt`; JUCE
generates its own icon resources from the `ICON_BIG` and `ICON_SMALL` PNGs.
It is retained as a pre-built Windows icon bundle.

## Constraints

The PNGs are the build-time source for both the JUCE-generated platform
icons and the embedded `Images` data. The `vectorIcon.svg` is the original
artwork source; it is not consumed by the build system directly.
