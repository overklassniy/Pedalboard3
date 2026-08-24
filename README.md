# Pedalboard3

A free, open-source VST3 plugin host for live performance, preserving the
original Pedalboard2 user interface while running on a modernized foundation.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![JUCE](https://img.shields.io/badge/JUCE-8.0.15-orange.svg)](https://juce.com)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)

---

## What It Is

Pedalboard3 is a standalone application that lets you load VST3 plugins, wire
them together on a visual canvas, and switch between patches for live
performance. It is a modernized rebuild of [Niall Moody's
Pedalboard2](http://www.niallmoody.com) (2011-2013), keeping the original's
user interface and workflow while updating the audio engine, plugin format
support, and build system to current standards.

The original Pedalboard2 was a VST2/AU/LADSPA host built on JUCE 1.x, abandoned
at version 2.13. Pedalboard3 preserves its UI, ports the codebase to JUCE
8.0.15 and C++20, adds VST3 hosting, and incorporates stability infrastructure
from a modern VST3 fork.

---

## Features

- Original Pedalboard2 user interface (custom BranchesLAF, colour schemes,
  SVG vector graphics, patch bar, transport, plugin canvas)
- VST3 plugin hosting (Windows and Linux)
- LADSPA plugin hosting (Linux)
- MIDI CC mapping with learn mode, custom ranges, latch/toggle
- OSC control via JUCE's juce_osc module
- Out-of-process plugin scanner with crash protection and blacklist
- Safety limiter and glitch-free crossfade patch switching
- JSON-based settings persistence
- Undo/redo for graph operations
- Built-in processors: audio I/O, MIDI input, OSC input, level, file player,
  output toggle, VU meter, audio recorder, metronome, looper

---

## Installation

### Windows (64-bit)

1. Download the latest release from the GitHub Releases page.
2. Extract `Pedalboard3.exe` to your preferred location.
3. Run the app and scan plugins from **Options > Plugin List**.

### Build From Source

Requirements:

- Visual Studio 2022 (Windows) or GCC 11+/Clang 12+ (Linux)
- CMake 3.24+
- Git

```bash
git clone --recursive <repo-url>
cd Pedalboard3
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Output: `build/Pedalboard3_artefacts/Release/Pedalboard3.exe` (Windows) or
`build/Pedalboard3_artefacts/Release/Pedalboard3` (Linux).

See [docs/BUILD.md](docs/BUILD.md) for detailed build instructions.

---

## Quick Start

1. Double-click the canvas to add a plugin or processor.
2. Drag from an output pin to an input pin to create a connection.
3. Use the patch bar at the bottom to save, load, and switch patches.
4. Configure MIDI and OSC mappings from the Options menu.

---

## Settings Location

| OS | Path |
| --- | --- |
| Windows 10/11 | `%APPDATA%\Pedalboard3` |
| Linux | `~/.config/Pedalboard3` |

---

## Documentation

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — how it is built
- [docs/BUILD.md](docs/BUILD.md) — detailed build instructions
- [docs/USER_GUIDE.md](docs/USER_GUIDE.md) — end-user feature documentation
- [docs/MIGRATION.md](docs/MIGRATION.md) — JUCE 1.x to 8.0 porting notes
- [AGENTS.md](AGENTS.md) — build setup and JUCE 8 API notes for developers

---

## Credits

- Original Author: [Niall Moody](http://www.niallmoody.com) (2011)
- Modernization: Pedalboard3 Project (2026)
- Framework: [JUCE](https://juce.com)

---

## License

GPL v3. See `license.txt`.
