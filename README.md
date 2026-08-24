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

#### Prerequisites

| Platform | Requirements |
| --- | --- |
| Windows | Visual Studio 2022 (BuildTools or full IDE) with C++ workload, CMake 3.24+, Git |
| Linux | GCC 11+ or Clang 12+, CMake 3.24+, Git, ALSA dev headers, JACK dev headers (optional) |

VS 2022 bundles CMake 3.31 at:

```
C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
```

Add this path to your `PATH` environment variable, or use the full path in
commands.

#### JUCE submodule

JUCE is pinned to tag `8.0.15`. Do NOT update to JUCE 9.x — it has breaking
API changes (notably `Typeface::createSystemTypefaceFor` crashes on Windows
with DirectWrite, and `createFromSVG_string` is JUCE 9 only).

```powershell
git clone --recursive <repo-url>
cd Pedalboard3
git submodule update --init --recursive
```

If the submodule is missing, clone it manually:

```powershell
git clone --depth 1 --branch 8.0.15 https://github.com/juce-framework/JUCE.git JUCE
```

#### Configure and build (Windows)

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -- /m
```

#### Configure and build (Linux)

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

#### Build artifacts

- `build/Pedalboard3_artefacts/Release/Pedalboard3.exe` (Windows)
- `build/Pedalboard3_artefacts/Release/Pedalboard3` (Linux)
- `build/Pedalboard3Scanner_artefacts/Release/Pedalboard3Scanner.exe` (Windows)

#### Dependencies (fetched via CPM.cmake)

| Dependency | Version | Purpose |
| --- | --- | --- |
| JUCE | 8.0.15 | Audio framework (submodule) |
| fmt | 12.2.0 | String formatting |
| spdlog | v1.17.0 | Logging |
| nlohmann_json | v3.12.0 | JSON settings |
| Catch2 | v3.15.3 | Unit tests (optional) |

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

## Architecture

- UI is ported from the original Pedalboard2 (Niall Moody, 2011-2013),
  preserving the original `BranchesLAF` LookAndFeel and layout.
- VST3 hosting via JUCE's built-in `JUCE_PLUGINHOST_VST3`.
- OSC via JUCE's `juce_osc` module (replaces the original's custom
  `NiallsOSCLib`/`NiallsSocketLib`).
- Stability infrastructure (out-of-process scanner, crash protection,
  blacklist, safety limiter, crossfade patch switching, JSON settings,
  undo/redo) ported from the pedalboard3-VST3 fork.

### Reference codebases

- `pedalboard2-OLD/` – original Pedalboard2 source (UI source of truth). Read-only reference.
- `pedalboard3-VST3/` – VST3 fork (JUCE 8 migration patterns + stability infra source). Read-only reference.

Both are in `.gitignore` and are not part of the Pedalboard3 source tree.

### Directory structure

| Directory | Purpose |
| --- | --- |
| `src/app/` | Application shell, main window, main panel, about, log display, tray icon |
| `src/audio/` | Audio engine core: graph, singletons, transport, internal plugin format |
| `src/osc/` | OSC mapping: base Mapping class, OSC manager, tap tempo helper |
| `src/util/` | Utilities and embedded resource data (vectors, images, properties, log) |
| `src/lookandfeel/` | Custom BranchesLAF LookAndFeel and colour scheme system |
| `src/processors/` | Built-in processors, editors, controls, DSP safety (limiter, crossfade, VU) |
| `src/canvas/` | Plugin canvas: node components and the field/canvas |
| `src/mappings/` | MIDI mapping system and mapping UI components |
| `src/preset/` | Patch and preset management, preferences dialog |
| `src/stability/` | Stability infrastructure: blacklist, crash protection, settings, undo/redo |
| `src/scanner/` | Out-of-process plugin scanner (IPC, client, safe wrapper, pool, main entry) |
| `JUCE/` | JUCE 8.0.15 git submodule (audio framework) |
| `cmake/` | CPM.cmake dependency manager script |
| `tests/` | Unit tests (Catch2) |
| `assets/` | Visual assets: `icons/` (app icons, PNG/ICO/SVG) and `vectors/` (UI button SVGs) |
| `pedalboard2-OLD/` | Original Pedalboard2 source (reference only) |
| `pedalboard3-VST3/` | VST3 fork source (reference only) |

### JUCE 8 API notes

- `AudioProcessor::createEditor()` is private — use `createEditorAndMakeActive()`.
- `Font::getStringWidth` removed — use `juce::GlyphArrangement::getStringWidthInt`.
- `Font(15.0f)` → `Font(FontOptions().withHeight(15.0f))`.
- `Drawable::createFromSVG(XmlElement)` is correct for 8.0.15 (NOT `createFromSVG_string`).
- `ScopedPointer<T>` → `std::unique_ptr<T>`.
- `node->nodeID` (uint32) → `node->nodeID.uid`.
- `getNumConnections()`/`getConnection(i)` → `getConnections()` (returns `std::vector`).
- `Graphics`: call `setColour` before `setFont` before `drawText`.

---

## Repository Workflow

This is a single-developer repository. Do not create pull requests by default.

1. Work on the requested branch or create a feature branch when useful.
2. Commit focused changes locally following Conventional Commits.
3. Push the branch or `main` directly when the user asks to back up or publish work.

---

## Credits

- Original Author: [Niall Moody](http://www.niallmoody.com) (2011)
- Modernization: Pedalboard3 Project (2026)
- Framework: [JUCE](https://juce.com)

---

## License

GPL v3. See `license.txt`.
