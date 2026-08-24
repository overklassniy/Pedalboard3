# AGENTS.md

## Repository Workflow

This is a single-developer repository. Do not create pull requests by default.

Default workflow:

1. Work on the requested branch or create a feature branch when useful.
2. Commit focused changes locally following Conventional Commits.
3. Push the branch or `main` directly when the user asks to back up or publish work.

## Build Setup (Windows)

The repo uses JUCE 8.0.15 as a git submodule. Third-party dependencies are
fetched via CPM.cmake at configure time.

### Prerequisites

- Visual Studio 2022 (BuildTools or full IDE) with C++ workload
- CMake 3.24+ (VS 2022 bundles CMake 3.31 at the path below)
- Git

### JUCE submodule

JUCE is pinned to tag `8.0.15`. Do NOT update to JUCE 9.x — it has breaking
API changes (notably `Typeface::createSystemTypefaceFor` crashes on Windows
with DirectWrite, and `createFromSVG_string` is JUCE 9 only).

```powershell
git submodule update --init --recursive
```

If the submodule is missing, clone it manually:

```powershell
git clone --depth 1 --branch 8.0.15 https://github.com/juce-framework/JUCE.git JUCE
```

### Configure and build

CMake is at the VS 2022 bundled path. Add it to PATH or use the full path:

```powershell
$env:Path = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;" + $env:Path
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -- /m
```

Artifacts:

- `build/Pedalboard3_artefacts/Release/Pedalboard3.exe`

### Dependencies (fetched via CPM.cmake)

| Dependency | Version | Purpose |
| --- | --- | --- |
| JUCE | 8.0.15 | Audio framework (submodule) |
| fmt | 12.2.0 | String formatting |
| spdlog | v1.17.0 | Logging |
| nlohmann_json | v3.12.0 | JSON settings |
| Catch2 | v3.15.3 | Unit tests (optional) |

## Build Setup (Linux)

```bash
git submodule update --init --recursive
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Requires GCC 11+ or Clang 12+, ALSA dev headers, JACK dev headers (optional).

## Architecture

See `docs/ARCHITECTURE.md` for the full architecture document.

Key points:

- UI is ported from the original Pedalboard2 (Niall Moody, 2011-2013),
  preserving the original `BranchesLAF` LookAndFeel and layout.
- VST3 hosting via JUCE's built-in `JUCE_PLUGINHOST_VST3`.
- OSC via JUCE's `juce_osc` module (replaces the original's custom
  `NiallsOSCLib`/`NiallsSocketLib`).
- Stability infrastructure (out-of-process scanner, crash protection,
  blacklist, safety limiter, crossfade patch switching, JSON settings,
  undo/redo) ported from the pedalboard3-VST3 fork.

## Reference codebases

- `pedalboard2-OLD/` — original Pedalboard2 source (UI source of truth). Read-only reference.
- `pedalboard3-VST3/` — VST3 fork (JUCE 8 migration patterns + stability infra source). Read-only reference.

Both are in `.gitignore` and are not part of the Pedalboard3 source tree.

## JUCE 8 API notes

- `AudioProcessor::createEditor()` is private — use `createEditorAndMakeActive()`.
- `Font::getStringWidth` removed — use `juce::GlyphArrangement::getStringWidthInt`.
- `Font(15.0f)` → `Font(FontOptions().withHeight(15.0f))`.
- `Drawable::createFromSVG(XmlElement)` is correct for 8.0.15 (NOT `createFromSVG_string`).
- `ScopedPointer<T>` → `std::unique_ptr<T>`.
- `node->nodeID` (uint32) → `node->nodeID.uid`.
- `getNumConnections()`/`getConnection(i)` → `getConnections()` (returns `std::vector`).
- `Graphics`: call `setColour` before `setFont` before `drawText`.
