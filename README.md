<img src="assets/readme/hero.svg" alt="Pedalboard3 – a free, open-source VST3 plugin host for live performance, built on JUCE 8 and C++20." />

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)](https://isocpp.org/)
[![JUCE 8.0.15](https://img.shields.io/badge/JUCE-8.0.15-8C8CCC.svg)](https://github.com/juce-framework/JUCE)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-0078D4.svg)](docs/build.md)
[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-FCC624.svg)](docs/build.md)
[![CMake 3.24+](https://img.shields.io/badge/CMake-3.24%2B-064F8C.svg)](CMakeLists.txt)

Pedalboard3 is a free, open-source **VST3 plugin host for live
performance**. It is a modernized rebuild of Pedalboard2 that preserves
the original UI while running on JUCE 8.0.15 and C++20.

## Features

**Hosting**

- VST3 and LADSPA plugin hosting
- Windows audio backends: ASIO, WASAPI, DirectSound
- Linux audio backends: ALSA, JACK
- Out-of-process plugin scanner with crash isolation
- Plugin blacklist and crash protection
- Sliding-window plugin pool for near-instant patch switching

**Built-in processors**

- Looper, metronome, audio recorder, file player
- VU meter, output toggle, level control
- Safety limiter and crossfade mixer (DSP safety modules)

**Control**

- MIDI CC to parameter and application command mapping, with MIDI learn
- OSC to parameter and application command mapping
- Tap tempo (shared across MIDI and OSC)
- Patch (`.pdl`) and per-plugin preset (`.fxp`) management
- Undo/redo actions for graph editing

**UI**

- Custom LookAndFeel (`BranchesLAF`) ported from Pedalboard2
- Editable colour scheme with `.colourscheme` presets
- System tray icon (Windows and Linux)
- Log display with event-type filters

## Quick start

Clone with the JUCE submodule (pinned to tag 8.0.15):

```bash
git clone --recursive https://github.com/overklassniy/Pedalboard3.git
cd Pedalboard3
```

### Windows (Visual Studio 2022 x64)

```bash
cmake --preset windows-default
cmake --build build --config Release
```

### Linux (Ninja + gcc)

```bash
cmake --preset linux-default
cmake --build build
```

See the [build guide](docs/build.md) for prerequisites, debug presets,
tests, and dependency details.

## Documentation

- [Build guide](docs/build.md) – prerequisites, CMake presets, tests
- [Architecture](docs/architecture.md) – modules, audio engine, scanner,
  stability, LookAndFeel
- [User guide](docs/user-guide.md) – patches, presets, processors,
  MIDI/OSC mapping, colour scheme, preferences
- [Development](docs/development.md) – conventions, JUCE 8 API notes,
  adding a processor, running tests

## Lineage and credits

- **Pedalboard2** (2011) by Niall Moody – the original VST2/AU/LADSPA
  host. This is the base this repository is built on.
  <https://www.niallmoody.com/work/pedalboard2/>
- **Pedalboard3** (2024) by Project12x – a fork of Pedalboard2 that
  added VST3 hosting, a new UI, and many other features. Some ideas
  from this fork were referenced during this rebuild.
  <https://github.com/Project12x/Pedalboard3>
- **Pedalboard3** (2026) by overklassniy – this repository. A
  modernized rebuild based on the original Pedalboard2, with some
  features informed by the Project12x fork, running on JUCE 8.0.15
  and C++20.
  <https://github.com/overklassniy/Pedalboard3>

The full attribution chain is in the [LICENSE](LICENSE) header.

## License

Pedalboard3 is free software licensed under the
[GNU General Public License v3](LICENSE).
