# src/lookandfeel/

Custom LookAndFeel and colour scheme system for Pedalboard3. This folder
holds the application's widget drawing style, the named-colour palette that
feeds it, and the editor dialog that lets users customise and save colour
schemes.

## Contents

- `BranchesLAF.h` / `BranchesLAF.cpp` – `BranchesLAF`, a `juce::LookAndFeel`
  subclass (originally written for the Branches story editor, adapted for
  Pedalboard3) that draws buttons, scrollbar buttons/thumbs, menu bars, popup
  menus, combo boxes, progress bars, keymap buttons, labels, toggle buttons,
  tick boxes, text editor backgrounds, and callout boxes. Its constructor
  configures widget colours from the current `ColourScheme`.
- `ColourScheme.h` / `ColourScheme.cpp` – `ColourScheme`, a singleton struct
  holding a `std::map<String, Colour>` of named colours plus the current
  preset name, with methods to list, load, save, and compare colour scheme
  presets.
- `ColourSchemeEditor.h` / `ColourSchemeEditor.cpp` – `ColourSchemeEditor`, a
  `Component`/`ListBoxModel`/`ChangeBroadcaster` dialog with a colour
  selector, a colour list, and a combo box plus buttons for choosing, saving,
  and deleting colour scheme presets.

## Integration

`BranchesLAF` is the application's custom LookAndFeel; it is included by
`app/App.cpp` and configures its widget colours from `ColourScheme`. It draws
the file-chooser folder icon and magnifying glass from embedded graphics in
`util/LookAndFeelImages` (it includes `LookAndFeelImages.h` directly).
`ColourScheme` is the singleton colour palette queried by `BranchesLAF` and
other components. `ColourSchemeEditor` uses embedded SVG button graphics from
`util/Vectors` (it includes `Vectors.h`) and is instantiated from the Options
menu in `app/MainPanel` (see `MainPanel.cpp`), which listens for its change
broadcasts to refresh the UI.
