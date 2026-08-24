# src/

Source code for Pedalboard3, organized into feature subdirectories.

## Directory structure

| Subdirectory | Purpose |
| --- | --- |
| `app/` | Application shell: JUCE app entry point, main window, main panel (menu bar, patch bar, transport, canvas viewport), about dialog, log display, system tray icon |
| `audio/` | Audio engine core: AudioProcessorGraph wrapper (FilterGraph), singletons for format managers and plugin lists, internal plugin format, bypassable plugin instance, main transport, MIDI app FIFO |
| `osc/` | OSC mapping: base Mapping class, OSC mapping manager (uses juce_osc), tap tempo helper |
| `util/` | Utilities and embedded resources: properties singleton, log file, JUCE helper functions, embedded SVG vector data, embedded PNG images, LookAndFeel images |
| `lookandfeel/` | Custom LookAndFeel (BranchesLAF, ported from Pedalboard2), colour scheme system and editor |
| `processors/` | Built-in processors and their editors/controls: level, file player, output toggle, VU meter, audio recorder, metronome, looper. Also DSP safety: safety limiter, crossfade mixer, VU meter DSP |
| `canvas/` | Plugin canvas: PluginComponent (individual plugin node UI), PluginField (canvas component for nodes and connections) |
| `mappings/` | MIDI mapping system: MIDI mapping manager, mappings dialog, mapping entries (MIDI/OSC), mapping slider, MIDI CC alert window, application mappings editor, tap tempo box |
| `preset/` | Patch and preset management: patch organiser, preset manager, preset bar, user preset window, preferences dialog |
| `stability/` | Stability infrastructure: plugin blacklist, crash protection, settings manager (JSON), undo/redo actions, blacklist window |
| `scanner/` | Out-of-process plugin scanner: IPC protocol, scanner client, safe plugin scanner wrapper, plugin pool manager, scanner main entry point |

## Integration

All subdirectories are compiled into the `Pedalboard3` GUI application target,
except `scanner/PluginScannerMain.cpp` which builds the separate
`Pedalboard3Scanner` console application.

Flat includes (`#include "Foo.h"`) work across all subdirectories via
`target_include_directories` in the root `CMakeLists.txt`.

The `processors/` directory contains per-class files split from the original
`PedalboardProcessors.h/.cpp` and `PedalboardProcessorEditors.h/.cpp`. The
original files remain as umbrella headers for backward compatibility.

## JUCE 8 API notes

- `AudioProcessor::createEditor()` is private — use `createEditorAndMakeActive()`.
- `Font::getStringWidth` removed — use `juce::GlyphArrangement::getStringWidthInt`.
- `Font(15.0f)` → `Font(FontOptions().withHeight(15.0f))`.
- `Drawable::createFromSVG(XmlElement)` is correct for 8.0.15 (NOT `createFromSVG_string`).
- `ScopedPointer<T>` → `std::unique_ptr<T>`.
- `node->nodeID` (uint32) → `node->nodeID.uid`.
- `getNumConnections()`/`getConnection(i)` → `getConnections()` (returns `std::vector`).
- `Graphics`: call `setColour` before `setFont` before `drawText`.

## Conventions

- C++20, JUCE 8.0.15 APIs
- All comments in English
- `///` Doxygen-style comments for declarations, `//` for implementation comments
- No decorative dividers, no emojis, no double hyphens in comments
- Code style defined by `.clang-format` at the repo root
