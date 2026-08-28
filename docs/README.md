# docs/

Project documentation for Pedalboard3. This folder holds the detailed
guides that the root `README.md` links to, keeping the homepage lean
while making the full build, architecture, usage, and development
information available one click away.

## Contents

- [build.md](build.md) – prerequisites, CMake presets, and step-by-step
  build instructions for Windows and Linux.
- [architecture.md](architecture.md) – module breakdown, audio engine,
  scanner IPC, stability infrastructure, and the LookAndFeel / colour
  scheme system.
- [user-guide.md](user-guide.md) – using the running application:
  patches, presets, built-in processors, MIDI and OSC mapping, tap
  tempo, colour scheme editor, preferences, system tray, and log
  display.
- [development.md](development.md) – project structure, coding
  conventions, JUCE 8 API notes, adding a new processor, and running
  tests.

## Integration

The documentation is plain Markdown and is not consumed by the build
system. It is referenced from the root `README.md` via relative links.
Each document is written in English and follows the project's
documentation rules (no emojis, no decorative characters, en dash as the
separator in prose and tables).

The in-application F1 help is a single self-contained HTML file
(`assets/help/help.html`) derived from `user-guide.md` and embedded in
the binary via `juce_add_binary_data`. The Markdown docs here remain the
source of truth: when user-facing behaviour changes, update both the
relevant Markdown document and `assets/help/help.html` so the two stay
consistent.

## Constraints

The documentation describes the current state of the source tree. When a
module, file, or behaviour changes, the corresponding section here and
the relevant `src/<module>/README.md` should be updated together so the
two stay consistent.
