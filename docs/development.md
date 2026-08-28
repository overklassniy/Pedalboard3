# Development

How to work on the Pedalboard3 codebase: project structure, coding
conventions, JUCE 8 API notes, adding a new processor, and running
tests. For build instructions, see [build.md](build.md).

## Project structure

The source is in `src/`, organised into feature subdirectories. Each
subdirectory has its own `README.md` describing its files, integration
points, and constraints. See [architecture.md](architecture.md) for the
module breakdown and the per-module READMEs for file-level detail.

Flat includes (`#include "Foo.h"`) work across all subdirectories via
`target_include_directories` in the root `CMakeLists.txt`, which lists
every `src/` subdirectory in the include path.

Two CMake targets are defined:

- `Pedalboard3` – the GUI application (`juce_add_gui_app`).
- `Pedalboard3Scanner` – the out-of-process scanner console app
  (`juce_add_console_app`), built from `src/scanner/PluginScannerMain.cpp`
  only. The remaining scanner files are compiled into the main target.

## Coding conventions

- C++20, JUCE 8.0.15 APIs.
- All comments in English.
- `///` Doxygen-style line comments for declarations; `//` for
  implementation comments inside function bodies. See the project
  documentation rules for the full `@param` / `@return` conventions.
- No emojis, no decorative dividers, no double hyphens in comments; use
  the em dash where a dash is needed.
- Code style is defined by `.clang-format` at the repo root.

## JUCE 8 API notes

The codebase targets JUCE 8.0.15 and relies on its breaking API
changes. Do not update the JUCE submodule to `master` or 9.x. Notes
from `src/README.md`:

- `AudioProcessor::createEditor()` is private – use
  `createEditorAndMakeActive()`.
- `Font::getStringWidth` removed – use
  `juce::GlyphArrangement::getStringWidthInt`.
- `Font(15.0f)` becomes `Font(FontOptions().withHeight(15.0f))`.
- `Drawable::createFromSVG(XmlElement)` is correct for 8.0.15 (not
  `createFromSVG_string`).
- `ScopedPointer<T>` becomes `std::unique_ptr<T>`.
- `node->nodeID` (uint32) becomes `node->nodeID.uid`.
- `getNumConnections()` / `getConnection(i)` become `getConnections()`
  (returns `std::vector`).
- `Graphics`: call `setColour` before `setFont` before `drawText`.

## Adding a new built-in processor

Built-in processors live in `src/processors/` and extend
`PedalboardProcessor` (defined in `PedalboardProcessor.h`). The
existing per-class files are the source of truth; the umbrella headers
`PedalboardProcessors.h` / `PedalboardProcessorEditors.h` include all
per-class headers for backward compatibility.

To add a new processor:

1. Create `MyProcessor.h` and `MyProcessor.cpp` in `src/processors/`,
   extending `PedalboardProcessor`. Implement the pure virtual
   `getControls()` and `getSize()` used to build the on-canvas control
   component.
2. If the processor has a full editor window, create
   `MyEditor.h` / `MyEditor.cpp` extending `AudioProcessorEditor`, and a
   control component (`MyControl.h` / `MyControl.cpp`) if it has an
   on-canvas control.
3. Add the new source files to `target_sources(Pedalboard3 PRIVATE ...)`
   in the root `CMakeLists.txt`.
4. Add the new header to the `PedalboardProcessors.h` (and, if it has
   an editor, `PedalboardProcessorEditors.h`) umbrella header so it is
   visible to the existing include surface.
5. Register the processor with `InternalPluginFormat` in
   `src/audio/InternalFilters.cpp` so it appears in the plugin list.
   Note: the per-class processor filter types are declared but not yet
   enabled there; follow the pattern of the existing I/O filters.
6. If the control component uses SVG button graphics, embed them through
   `src/util/Vectors.cpp` / `Vectors.h` and load them via
   `JuceHelperStuff::loadSVGFromMemory`.
7. Update `src/processors/README.md` and the relevant docs in this
   folder.

## Tests

Tests live in `tests/` and use Catch2 v3.15.3. They are built when
`Pedalboard3_BUILD_TESTS=ON` (the default). The test sources do not
link JUCE directly; `filtergraph_test.cpp` and `protection_test.cpp`
exercise logic through mock types rather than the real `FilterGraph` and
`PluginBlacklist` implementations.

Available test tags (from `tests/README.md`):

| Tag | Coverage |
| --- | --- |
| `[smoke]` | Build verification, fmt and spdlog integration, C++17 features |
| `[filtergraph]` | Node, connection, position, infrastructure, and mutation tests |
| `[nodes]` / `[connections]` / `[position]` / `[infrastructure]` / `[mutation]` | Sub-tags of `[filtergraph]` |
| `[protection]` | `PluginBlacklist` and `CrashProtection` tests, including thread safety |

Run the tests after building:

```bash
ctest --test-dir build --build-config Release
```

Or run the test executable directly and filter by tag:

```bash
./build/tests/Release/Pedalboard3_Tests.exe [smoke]
```

To disable tests entirely:

```bash
cmake --preset windows-default -DPedalboard3_BUILD_TESTS=OFF
```
