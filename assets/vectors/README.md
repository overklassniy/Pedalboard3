# assets/vectors/

Source SVG vector graphics for Pedalboard3 UI elements. Each SVG depicts a
single button or control glyph at a fixed pixel size; multi-state controls
are represented by separate files for each visual state.

## Contents

- `playButton.svg` – 24x24 right-pointing triangle (play transport button)
- `pauseButton.svg` – 24x24 two vertical rounded bars (pause transport
  button)
- `stopButton.svg` – 24x24 rounded square (stop transport button)
- `rtzButton.svg` – 24x24 right-pointing triangle with a vertical bar on the
  right (return-to-zero transport button)
- `recordButton.svg` – 24x24 filled circle (record transport button)
- `bypassButtonOff.svg` – 14x14 lowercase letter "b" in regular Verdana
  (plugin bypass off state)
- `bypassButtonOn.svg` – 14x14 lowercase letter "b" in bold Verdana (plugin
  bypass on state)
- `closeFilterButton.svg` – 12x12 X-shaped cross (plugin close button, normal
  state)
- `closeFilterButtonOver.svg` – 12x12 X-shaped cross (plugin close button,
  hover state)
- `closeFilterButtonDown.svg` – 12x12 X-shaped cross (plugin close button,
  pressed state)
- `addMappingButton.svg` – 32x32 plus sign (add mapping button, normal state)
- `addMappingButtonOver.svg` – 32x32 plus sign at lighter opacity (add
  mapping button, hover state)
- `deleteButton.svg` – 24x24 X-shaped cross (delete button)
- `newButton.svg` – 24x24 document outline with a folded corner (new file
  button)
- `openButton.svg` – 24x24 open folder shape (open file button)
- `saveButton.svg` – 24x24 floppy disk shape with a slot (save file button)
- `outputToggle1.svg` – 48x48 curved line ending in a filled circle (output
  toggle, state 1)
- `outputToggle2.svg` – 48x48 curved line ending in a filled circle, mirrored
  vertically (output toggle, state 2)

## Integration

These SVGs are the source for the embedded vector data in
`src/util/Vectors.cpp`. Each file is stored as a byte array and exposed
through the `Vectors` namespace declared in `src/util/Vectors.h`. At runtime
the data is loaded via `JuceHelperStuff::loadSVGFromMemory` (defined in
`src/util/JuceHelperStuff.cpp`), which wraps
`juce::Drawable::createFromSVG` to produce drawable objects.

The embedded vectors are consumed by the following source files:

- `src/processors/FilePlayerControl.cpp` – `rtzButton`, `playButton`,
  `pauseButton`
- `src/processors/LooperControl.cpp` – `rtzButton`, `playButton`,
  `pauseButton`, `recordButton`, `stopButton`
- `src/processors/LooperEditor.cpp` – `rtzButton`, `playButton`,
  `pauseButton`, `recordButton`, `stopButton`
- `src/processors/MetronomeControl.cpp` – `playButton`, `pauseButton`
- `src/processors/AudioRecorderControl.cpp` – `recordButton`, `stopButton`
- `src/processors/OutputToggleEditor.cpp` – `outputToggle1`,
  `outputToggle2`
- `src/preset/PresetBar.cpp` – `saveButton`, `openButton`
- `src/mappings/ApplicationMappingsEditor.h` – `addMappingButton`,
  `addMappingButtonOver`
- `src/lookandfeel/ColourSchemeEditor.cpp` – `newButton`, `deleteButton`
- `src/canvas/PluginComponent.cpp` – `closeFilterButton`,
  `closeFilterButtonOver`, `closeFilterButtonDown`,
  `bypassButtonOff`, `bypassButtonOn`

## Constraints

The SVGs are the build-time source for the embedded data in
`src/util/Vectors.cpp`; they are not read from disk at runtime. Modifying an
SVG requires regenerating the corresponding byte array in `Vectors.cpp` and
the declarations in `Vectors.h`.
