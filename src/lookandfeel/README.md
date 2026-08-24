# src/lookandfeel/

Custom LookAndFeel and colour scheme system for Pedalboard3.

## Contents

- `BranchesLAF.cpp` / `BranchesLAF.h` — custom LookAndFeel ported from the original Pedalboard2 (draws buttons, sliders, combo boxes, headers, etc. using SVG vector graphics)
- `ColourScheme.cpp` / `ColourScheme.h` — theme colour system (manages named colours, loads/saves schemes)
- `ColourSchemeEditor.cpp` / `ColourSchemeEditor.h` — colour scheme editor dialog (list, selector, preset save/load/delete)

## Integration

`BranchesLAF` is applied to the entire application UI and uses embedded SVG
data from `util/Vectors` and `util/LookAndFeelImages`. `ColourScheme`
provides the colour palette that `BranchesLAF` and other components query.
`ColourSchemeEditor` is opened from the Options menu in `app/MainPanel`.
