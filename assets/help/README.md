# assets/help/

Source for the in-application help shown when the user presses F1 or
selects Help -> Documentation.

## Contents

- `help.html` — a single self-contained HTML document with inline CSS,
  a table of contents, and the full user-facing help text for
  Pedalboard3. No external resources, no images, no linked stylesheet.

## Integration

`help.html` is embedded into the `Pedalboard3` executable at build time
via `juce_add_binary_data` (see `CMakeLists.txt`). The generated accessor
namespace is `HelpData`, exposed through the generated header
`HelpData.h`:

- `HelpData::help_html` — pointer to the embedded HTML data.
- `HelpData::help_htmlSize` — size of the embedded HTML data in bytes.

At runtime, the `HelpDocumentation` command in `MainPanel`
(`src/app/MainPanel.cpp`) writes the embedded HTML to the application
data folder (`<user data>/Pedalboard3/help.html`) and launches it in the
default browser. No on-disk `documentation/` folder next to the
executable is required.

## Constraints

- `help.html` must remain self-contained: all CSS must be inline, and it
  must not reference any external files (no linked stylesheets, no
  images, no scripts). This is what makes the embedded-resource approach
  work — a single file is written to disk and opened directly.
- The content is derived from `docs/user-guide.md`. When user-facing
  behaviour changes, update both `docs/user-guide.md` and
  `help.html` so the two stay consistent. The Markdown docs in `docs/`
  remain the source of truth.
- All text must be in English, with no emojis or decorative characters,
  per the project documentation rules.
