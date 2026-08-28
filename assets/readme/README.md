# assets/readme/

Visual assets used by the root `README.md`. These assets are not consumed
by the build system and are not embedded into the application; they exist
only to illustrate the repository homepage.

## Contents

- `hero.svg` - the README hero. A 1200x280 static SVG with a centred
  title-only composition on the application's beige background. All
  colours are taken from the application's real default colour scheme
  in `src/lookandfeel/ColourScheme.cpp`.

## Integration

The hero is referenced from the root `README.md` via a relative path:

```markdown
<img src="assets/readme/hero.svg" alt="..." />
```

It is not referenced by `CMakeLists.txt` or any source file.

## Constraints

- Static SVG only. No animation, no external fonts, no `foreignObject`,
  no remote references - GitHub strips those.
- Colours must stay in sync with the default preset in
  `src/lookandfeel/ColourScheme.cpp`. If the application's default
  palette changes, this hero should be updated to match.
- Keep the SVG editable; do not rasterize it.
