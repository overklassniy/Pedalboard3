# Architecture

How Pedalboard3 is structured and how the subsystems fit together. This
document summarises the module breakdown from `src/README.md` and the
per-module READMEs; read those for file-level detail.

## Project layout

```
Pedalboard3/
  CMakeLists.txt        # build definition for the app and scanner
  CMakePresets.json     # Windows and Linux CMake presets
  assets/               # icons and UI vector graphics (embedded at build time)
  cmake/                # CPM.cmake dependency manager
  docs/                 # this documentation
  JUCE/                 # JUCE 8.0.15 (submodule)
  src/                  # application source, organised by feature
  tests/                # Catch2 unit tests
```

The `src/` tree is split into feature subdirectories, each with its own
`README.md`:

| Subdirectory | Purpose |
| --- | --- |
| `app/` | Application shell: JUCE app entry point, main window, main panel, about dialog, log display, system tray |
| `audio/` | Audio engine core: `FilterGraph`, singletons, internal I/O format, bypassable plugin instance, transport, MIDI FIFO |
| `canvas/` | Plugin canvas: `PluginComponent` (node UI) and `PluginField` (canvas with nodes and connections) |
| `lookandfeel/` | Custom LookAndFeel (`BranchesLAF`) and the colour scheme system and editor |
| `mappings/` | MIDI mapping system, mapping dialogs, mapping entries, tap tempo box |
| `osc/` | OSC mapping: base `Mapping` class, `OscMappingManager`, tap tempo helper |
| `preset/` | Patch and preset management: patch organiser, preset manager, preset bar, preferences |
| `processors/` | Built-in processors and editors/controls, plus DSP safety modules |
| `scanner/` | Out-of-process plugin scanner IPC, safe scanner wrapper, plugin pool manager |
| `stability/` | Crash protection, plugin blacklist, JSON settings, undo/redo actions |
| `util/` | Properties singleton, log file, JUCE helpers, embedded SVG/PNG data |

## Audio engine

The audio engine is built around JUCE's `AudioProcessorGraph`. The core
types live in `src/audio/`:

- `FilterGraph` – a `FileBasedDocument` wrapping
  `juce::AudioProcessorGraph`. It manages plugin nodes, connections, and
  XML serialization, and defines the `.filtergraph` file extension. It
  is owned by `MainPanel` as the `signalPath` member and played through a
  `juce::AudioProcessorPlayer`.
- `AudioSingletons` – singleton wrappers for `AudioPluginFormatManager`,
  `AudioFormatManager`, `AudioThumbnailCache`, and `KnownPluginList`.
- `InternalPluginFormat` – an `AudioPluginFormat` that registers the
  built-in I/O nodes (audio input, audio output, MIDI input, OSC input)
  with the plugin format manager so they appear in the plugin list.
- `BypassableInstance` – an `AudioPluginInstance` wrapper that adds a
  smooth bypass ramp, per-plugin MIDI channel filtering, and
  OSC-injected MIDI messages. Bypass state is `std::atomic` for
  cross-thread safety. Its `createEditor()` calls JUCE 8's
  `createEditorAndMakeActive()`.
- `MainTransport` – a `ChangeBroadcaster` singleton that coordinates
  play/stop/return-to-zero state across registered transports.
- `MidiAppFifo` – a lock-free FIFO (`juce::AbstractFifo` with a
  `SpinLock` on the producer side) that passes command IDs, tempo
  changes, patch changes, and deferred parameter changes from the audio
  thread to the message thread.

## Built-in processors

The built-in processors live in `src/processors/` and extend
`PedalboardProcessor` (an `AudioPluginInstance` subclass). Each provides
an on-canvas control component and, where applicable, a full editor
window:

- `LevelProcessor` – volume control
- `FilePlayerProcessor` – audio file playback (wraps
  `AudioTransportSource`)
- `OutputToggleProcessor` – toggles between two output destinations
- `VuMeterProcessor` – drives a VU meter from the passing audio
- `RecorderProcessor` – writes incoming audio to disk
- `MetronomeProcessor` – generates click samples in sync with the
  playhead tempo
- `LooperProcessor` – records input to disk and memory and plays it
  back, using fixed-size buffers to avoid audio-thread allocations

Three DSP safety modules in the same folder extend `juce::AudioProcessor`
directly rather than `PedalboardProcessor`:

- `SafetyLimiterProcessor` – final output safety limiter that
  soft-limits peaks above -0.5 dBFS, auto-mutes on sustained dangerous
  levels, DC offset, or ultrasonic content, and exposes thread-safe
  peak/VU accessors.
- `CrossfadeMixerProcessor` – applies an atomic fade-out/fade-in gain
  ramp for glitch-free patch switching.
- `VuMeterDsp` – header-only VU meter DSP implementing a critically
  damped 2-pole lowpass at ~3.5 Hz for the 300 ms IEC 60268-17 VU
  ballistics.

The per-class processor filter types are declared but not yet registered
with `InternalPluginFormat`; the DSP safety modules are not yet wired
into `FilterGraph`. See `src/processors/README.md` for the current
integration status.

## MIDI and OSC mapping

Mapping is split across `src/mappings/` (MIDI) and `src/osc/` (OSC):

- `MidiMappingManager` dispatches incoming MIDI CC messages to
  registered `MidiMapping` (CC to plugin parameter) and `MidiAppMapping`
  (CC to application command) entries, supports a one-shot MIDI learn
  callback, and tracks tap tempo. `MidiInterceptor` is a no-audio
  `AudioPluginInstance` that intercepts MIDI in the graph and forwards
  it to the manager.
- `OscMappingManager` receives `juce::OSCMessage` objects and dispatches
  float and MIDI arguments to `OscMapping` (OSC address to plugin
  parameter) and `OscAppMapping` (OSC address to application command)
  entries. `OscInput` is a dummy `AudioPluginInstance` that serves as a
  visual placeholder on the canvas. The actual `juce::OSCReceiver` is
  owned by `MainPanel`, which forwards messages to
  `OscMappingManager::messageReceived()`.
- `TapTempoHelper` averages the intervals between the last four taps and
  returns BPM, resetting below 30 BPM. It is shared between the OSC and
  MIDI mapping code.

Both managers are owned by `canvas/PluginField` and accessed via
`PluginField::getMidiManager()` / `getOscManager()`. The mapping UI
(`MappingsDialog`, `ApplicationMappingsEditor`, `TapTempoBox`) is
instantiated from `MainPanel` and `PluginComponent`.

## Patch and preset management

Patch and preset UI lives in `src/preset/`:

- `PatchOrganiser` manages the patch list inside a `.pdl` document (add,
  copy, remove, reorder, import from other `.pdl` files).
- `PresetManager` imports and saves per-plugin `.fxp` presets under
  `<user data>/Pedalboard3/presets/<plugin name>/`.
- `PresetBar` is embedded above each plugin editor and shows factory and
  user presets.
- `UserPresetWindow` is the full preset management dialog (copy, remove,
  import, export, rename).
- `PreferencesDialog` configures OSC port and multicast address, visible
  I/O nodes, MIDI options, and window/tray options.

`MainPanel` is a `FileBasedDocument` using `.pdl` as the patch document
extension; individual graph saves use `.filtergraph`.

## Stability infrastructure

`src/stability/` keeps the host alive when plugins misbehave:

- `CrashProtection` – `executeWithProtection()` (SEH-wrapped on Windows),
  `executeWithTimeout()` (worker thread with a deadline), and
  `executeWithProtectionAndTimeout()` (both). Includes an auto-save
  callback hook, a watchdog thread for UI hangs, crash-context logging,
  and the `ScopedOperationContext` RAII helper / `PROTECTED_OPERATION`
  macro.
- `PluginBlacklist` – user-configurable blacklisting by path and
  identifier, with path normalisation for case-insensitive comparison on
  Windows. Persists through `SettingsManager`.
- `SettingsManager` – thread-safe JSON settings persistence
  (`nlohmann::json` guarded by a mutex, auto-save after each write).
- `UndoActions` – `juce::UndoableAction` subclasses for filter graph
  editing (`AddPluginAction`, `RemovePluginAction`,
  `AddConnectionAction`, `RemoveConnectionAction`).
- `BlacklistWindow` – UI for managing the blacklist.

`CrashProtection` is used by `SafePluginScanner`; `PluginBlacklist` is
used by `PluginScannerClient` (auto-blacklist on scanner crash) and
`SafePluginScanner` (skip blacklisted files). The undo actions are
compiled in but not yet wired into `MainPanel`.

## Out-of-process plugin scanner

`src/scanner/` provides an isolated scanner process so a crashing plugin
cannot take down the host:

- `PluginScannerMain.cpp` builds the separate `Pedalboard3Scanner`
  console app. It registers plugin formats, connects to the host over a
  Windows named pipe (`\\.\pipe\Pedalboard3PluginScanner`), receives
  `ScanPlugin` requests, and sends back `ScanResult`, `ScanError`, or
  `ScanCrash` responses.
- `PluginScannerIPC.h` defines the protocol: message types, result
  codes, `MessageHeader`, `ScanRequest`, `ScanResponse`, and their JSON
  serialization.
- `PluginScannerClient` is the host-side IPC client. It launches the
  scanner process, sends requests, waits with a timeout, and restarts
  the scanner after a crash.
- `SafePluginScanner` uses `PluginScannerClient` for out-of-process
  scanning and falls back to in-process
  `juce::PluginDirectoryScanner` with timeout protection. It also
  defines `SafePluginListComponent`, a drop-in replacement for
  `juce::PluginListComponent`.
- `PluginPoolManager` is a sliding-window plugin pool that preloads
  plugin instances for the current patch plus N patches ahead or behind
  for near-instant patch switching.

The IPC protocol uses Windows named pipes and is currently
Windows-specific. `SafePluginListComponent` and `PluginPoolManager` are
compiled in but not yet wired into `MainPanel`.

## LookAndFeel and colour scheme

`src/lookandfeel/` holds the visual style:

- `BranchesLAF` is the custom `juce::LookAndFeel` that draws buttons,
  scrollbars, menus, combo boxes, progress bars, labels, toggle buttons,
  tick boxes, text editors, and callout boxes. Its constructor
  configures widget colours from `ColourScheme`.
- `ColourScheme` is a singleton struct holding a
  `std::map<String, Colour>` of named colours plus the current preset
  name, with methods to list, load, save, and compare `.colourscheme`
  presets.
- `ColourSchemeEditor` is the dialog for choosing, saving, and deleting
  colour scheme presets.

The default colour scheme (from `ColourScheme.cpp`) is a warm light
beige palette: window background `#EEECE1`, plugin background `#FFFFFF`,
plugin border and audio connections `#B0B0FF` (with the audio connection
cable colour being `#B0B0FF` darkened by 0.25), parameter connections
`#FFD3B3` (darkened by 0.25), and warm taupe sliders `#9A9181`. The
README hero uses this same palette.
