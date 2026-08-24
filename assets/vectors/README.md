# assets/vectors/

Source SVG vector graphics for Pedalboard3 UI elements.

## Contents

- `playButton.svg`, `pauseButton.svg`, `stopButton.svg` — transport buttons
- `rtzButton.svg` — return-to-zero button
- `recordButton.svg` — record button
- `bypassButtonOn.svg`, `bypassButtonOff.svg` — plugin bypass button states
- `closeFilterButton.svg`, `closeFilterButtonDown.svg`,
  `closeFilterButtonOver.svg` — plugin close button states
- `addMappingButton.svg`, `addMappingButtonOver.svg` — add mapping button
- `deleteButton.svg` — delete button
- `newButton.svg`, `openButton.svg`, `saveButton.svg` — file operation buttons
- `outputToggle1.svg`, `outputToggle2.svg` — output toggle states

## Integration

These SVGs are the source for the embedded vector data in
`src/util/Vectors.cpp`. The SVG data is loaded at runtime via
`juce::Drawable::createFromSVG` and rendered by the custom `BranchesLAF`
LookAndFeel and UI components.
