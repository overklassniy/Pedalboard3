# Pedalboard3 Architecture

This document describes the internal architecture of Pedalboard3, a
standalone VST3 plugin host for live performance. It covers the audio engine,
threading model, UI architecture, OSC implementation, stability infrastructure,
settings persistence, plugin scanning, build system, and source organization.

## Project Overview

Pedalboard3 is a modernized rebuild of Niall Moody's Pedalboard2 (2011-2013),
preserving the original user interface and workflow while updating the audio
engine, plugin format support, and build system to current standards.

The project is built on JUCE 8.0.15 with C++20. It hosts VST3 plugins on
Windows and Linux, and LADSPA plugins on Linux. The original Pedalboard2 was
a VST2/AU/LADSPA host built on JUCE 1.x; Pedalboard3 ports the codebase to
JUCE 8 and adds stability infrastructure from a modern VST3 fork.

### Reference Codebases

- `pedalboard2-OLD/` – Original Pedalboard2 source (UI source of truth). Read-only reference, not part of the build.
- `pedalboard3-VST3/` – VST3 fork (JUCE 8 migration patterns and stability infrastructure source). Read-only reference, not part of the build.

Both directories are listed in `.gitignore` and are not compiled.

### Directory Structure

| Directory | Purpose |
| --- | --- |
| `src/` | All C++ source files for the main application and scanner |
| `JUCE/` | JUCE 8.0.15 git submodule (audio framework) |
| `cmake/` | CPM.cmake dependency manager script |
| `docs/` | Project documentation |
| `tests/` | Unit tests (Catch2) |
| `icon/` | Application icons (PNG) |
| `images/` | Bitmap images used by the UI |
| `vectors/` | SVG vector graphics for UI elements |
| `pedalboard2-OLD/` | Original Pedalboard2 source (reference only) |
| `pedalboard3-VST3/` | VST3 fork source (reference only) |

## Audio Engine Architecture

### FilterGraph

`FilterGraph` (src/FilterGraph.h, src/FilterGraph.cpp) is the central audio
engine class. It wraps JUCE's `AudioProcessorGraph` and manages the plugin
signal chain.

Key responsibilities:

- Adding and removing plugins (nodes) at canvas positions
- Creating and removing connections between plugin pins
- Serializing and deserializing the graph to/from XML
- Managing default I/O nodes (audio input, audio output, MIDI input)

`FilterGraph` inherits from `juce::FileBasedDocument`, enabling standard
save/load/new document operations. The graph uses `.filtergraph` as its file
suffix.

The underlying `AudioProcessorGraph` holds all plugin nodes and their
connections. An `AudioProcessorPlayer` drives the graph, pulling audio from
the device manager through the processing chain.

Node identification uses `AudioProcessorGraph::NodeID` (JUCE 8 API). The
`getConnections()` method returns a `std::vector<AudioProcessorGraph::Connection>`
(replacing the old `getNumConnections()`/`getConnection(i)` pattern from
JUCE 1.x).

### AudioProcessorGraph

JUCE's `AudioProcessorGraph` is the backbone of the signal routing. Each
plugin is represented as a `Node` accessed via `Node::Ptr` (a
`ReferenceCountedObjectPtr`). Connections are defined by
`AudioProcessorGraph::Connection` objects linking source and destination
node/channel pairs.

The graph supports audio connections and MIDI connections. MIDI connections
use the special channel index `FilterGraph::midiChannelNumber`.

### BypassableInstance

`BypassableInstance` (src/BypassableInstance.h, src/BypassableInstance.cpp)
wraps an `AudioPluginInstance` to provide smooth bypass ramping and per-plugin
MIDI channel filtering.

Key features:

- **Smooth bypass** – Uses a `bypassRamp` float and a `tempBuffer` to crossfade between processed and dry audio. The bypass state is stored in `std::atomic<bool>` for cross-thread safety (set from the UI thread, read from the audio thread).
- **MIDI channel filtering** – An `std::atomic<int> midiChannel` (0 = omni) filters incoming MIDI messages so only the configured channel reaches the plugin.
- **OSC MIDI injection** – A `juce::MidiMessageCollector` allows OSC-derived MIDI messages to be injected into the plugin's processing via `addMidiMessage()`.
- **Parameter access** – Provides indexed access to the wrapped plugin's `AudioProcessorParameter` array (JUCE 8 API), replacing the deprecated `getParameter()`/`setParameter()` methods.
- **Editor creation** – Uses `createEditorAndMakeActive()` (JUCE 8 requirement, since `createEditor()` is private).

### InternalPluginFormat

`InternalPluginFormat` (src/InternalFilters.h, src/InternalFilters.cpp)
provides an `AudioPluginFormat` for the app's built-in processors. It
registers these internal types so they appear alongside VST3 plugins in the
plugin selection menu.

Available internal filter types:

| Type | Description |
| --- | --- |
| `audioInputFilter` | Audio input node |
| `audioOutputFilter` | Audio output node |
| `midiInputFilter` | MIDI input node |
| `oscInputFilter` | OSC input node (visual placeholder) |

### Built-in Processors

The built-in processors are defined in src/PedalboardProcessors.h and
src/PedalboardProcessors.cpp. All inherit from `PedalboardProcessor`, which
extends `AudioPluginInstance` and adds a `getControls()` method returning a
custom UI component.

| Processor | Description |
| --- | --- |
| `LevelProcessor` | Simple level (volume) control with a slider |
| `FilePlayerProcessor` | Plays back an audio file with play/pause, return-to-zero, loop, and sync-to-transport |
| `OutputToggleProcessor` | Toggles between two audio outputs with a fade |
| `VuMeterProcessor` | Stereo VU meter display |
| `RecorderProcessor` | Records audio to a file with sync-to-transport |
| `MetronomeProcessor` | Click track with configurable time signature, accent/click sounds, and sync-to-transport |
| `LooperProcessor` | Audio looper with record, playback, seamless looping, auto-play, and configurable bar length |

### Audio Singletons

`AudioSingletons.h` provides singleton wrappers for shared JUCE audio
objects:

- `AudioPluginFormatManagerSingleton` – Manages plugin format registration (VST3, LADSPA, Internal)
- `AudioFormatManagerSingleton` – Manages audio file format readers/writers
- `AudioThumbnailCacheSingleton` – Caches audio waveform thumbnails
- `KnownPluginListSingleton` – Global accessor for the `KnownPluginList`

## Threading Model

Pedalboard3 uses four distinct threads, each with specific responsibilities
and constraints.

### Audio Thread

The real-time audio thread runs inside JUCE's audio device callback. It
processes audio blocks through the `AudioProcessorGraph`.

Rules for the audio thread:

- No memory allocation, no locks, no file I/O.
- Bypass state and MIDI channel are read from atomics set by the UI thread.
- MIDI messages from OSC are injected via `MidiMessageCollector` (lock-free).
- The `SafetyLimiterProcessor` and `CrossfadeMixerProcessor` run on this thread as final-stage processors.
- Level metering values are written to `std::atomic<float>` for the UI to read.

### Message Thread (UI Thread)

The JUCE message thread handles all UI events, component painting, timer
callbacks, and user interactions. Most application logic runs here.

Responsibilities:

- Creating and destroying plugin instances (via `AudioPluginFormatManager`)
- Modifying the graph (adding/removing nodes and connections)
- Opening and closing plugin editor windows
- Handling OSC messages (via `OSCReceiver::MessageLoopCallback`)
- Saving and loading patches
- Updating the UI from timer callbacks (CPU meter, VU meters, etc.)

### Background Scanner Thread

The plugin scanner runs as a separate process (`Pedalboard3Scanner`) to
isolate crashes during plugin discovery. Communication uses Windows named
pipes (IPC protocol defined in `PluginScannerIPC.h`).

The `PluginScannerClient` class manages the scanner process lifecycle:

- Launching the scanner executable
- Sending scan requests via named pipe
- Waiting for responses with timeout
- Detecting scanner crashes and restarting automatically
- Auto-blacklisting plugins that crash the scanner

The `SafePluginScanner` wraps `PluginScannerClient` and falls back to
in-process scanning with timeout protection if the out-of-process scanner is
unavailable.

### Plugin Pool Loader Thread

`PluginPoolManager` (src/PluginPoolManager.h) runs a background `juce::Thread`
that preloads plugins for patches ahead of and behind the current setlist
position. This enables near-instant patch switching by maintaining a sliding
window of preloaded plugin instances.

Key behaviors:

- `setCurrentPosition()` triggers background preloading of patches within the configured `preloadRange` (default: 2 patches ahead).
- `loadPatchPlugins()` runs on the background thread, creating plugin instances from `PluginDescription` objects extracted from patch XML.
- `releaseUnusedPlugins()` evicts plugins outside the current window.
- Progress is reported to listeners via `PluginPoolListener::patchLoadingProgress()` and `patchReady()`.
- A `CriticalSection` (`poolLock`) protects shared data structures.

## UI Architecture

### MainPanel

`MainPanel` (src/MainPanel.h, src/MainPanel.cpp) is the top-level UI
component. It inherits from multiple JUCE classes to handle menus, commands,
timers, file documents, drag-and-drop, and various UI events.

Key responsibilities:

- Managing the audio device manager and graph player
- Hosting the `FilterGraph` (signal path) and `KnownPluginList`
- Receiving OSC messages via `juce::OSCReceiver`
- Managing the patch list (array of `XmlElement` patch definitions)
- Providing the menu bar and application commands
- Hosting the `PluginField` (canvas) in a viewport
- Transport controls (play/pause, return-to-zero, tap tempo)
- Patch navigation (previous/next, combo box)

`MainPanel` defines application command IDs (FileNew, FileOpen, FileSave,
OptionsAudio, PatchNextPatch, TransportPlay, etc.) used by the command
manager for menu items and keyboard shortcuts.

Three timers run on the message thread:

| Timer | Purpose |
| --- | --- |
| `CpuTimer` | Updates the CPU usage slider |
| `MidiAppTimer` | Processes the `MidiAppFifo` (MIDI messages from audio thread) |
| `ProgramChangeTimer` | Handles delayed MIDI program change patch switching |

### PluginField

`PluginField` (src/PluginField.h, src/PluginField.cpp) is the visual canvas
where plugins are displayed and connected. It inherits from `Component`,
`ChangeBroadcaster`, `AudioPlayHead`, and `Timer`.

Key features:

- Displays `PluginComponent` instances at their canvas positions
- Manages `PluginConnection` objects (bezier curves between pins)
- Handles drag-to-connect interaction between plugin pins
- Implements `AudioPlayHead::getPosition()` to provide tempo and transport state to plugins (JUCE 8 `Optional<PositionInfo>` API)
- Owns `MidiMappingManager` and `OscMappingManager` for the current patch
- Serializes/deserializes the patch to/from XML
- Supports file drag-and-drop for loading `.pdl` patch files

### PluginComponent

`PluginComponent` (src/PluginComponent.h, src/PluginComponent.cpp)
represents a single plugin or processor on the canvas. Each component
displays:

- The plugin name (editable label)
- Input pins, output pins, and parameter/MIDI pins
- Edit button (opens the plugin's editor window)
- Mappings button (opens the mappings dialog)
- Bypass button (drawable toggle)
- Delete button (drawable button)

`PluginPinComponent` represents individual input/output/parameter pins.
`PluginConnection` draws bezier curves between pins and handles selection
and deletion.

`PluginEditorWindow` wraps a plugin's `AudioProcessorEditor` in a
`DocumentWindow` with an optional `PresetBar` for preset management.

### BranchesLAF

`BranchesLAF` (src/BranchesLAF.h, src/BranchesLAF.cpp) is the custom
`LookAndFeel` class inherited from the original Pedalboard2. It implements
custom drawing for:

- Button backgrounds and text
- Scrollbar buttons and tracks
- Menu bar background and items
- Popup menu background
- ComboBoxes
- Progress bars
- Key mapping change buttons
- Labels
- Toggle buttons and tick boxes
- Text editor backgrounds
- Callout box backgrounds

This class preserves the original visual identity of Pedalboard2.

### ColourScheme

`ColourScheme` (src/ColourScheme.h, src/ColourScheme.cpp) is a singleton
managing the application's colour palette. It stores colours in a
`std::map<String, Colour>` and supports named presets that can be loaded,
saved, and compared. The `ColourSchemeEditor` provides a UI for customizing
individual colours.

## OSC Implementation

Pedalboard3 replaces the original Pedalboard2's custom `NiallsOSCLib`/
`NiallsSocketLib` with JUCE's built-in `juce_osc` module.

### OSCReceiver

`MainPanel` creates a `juce::OSCReceiver` and registers itself as a listener
with `MessageLoopCallback` template parameter, ensuring OSC messages are
delivered on the message thread. The port and optional multicast address are
configured via the Preferences dialog.

### OscMappingManager

`OscMappingManager` (src/OscMappingManager.h, src/OscMappingManager.cpp)
dispatches incoming OSC messages to registered mappings. It preserves the
original mapping semantics:

- **Address learning** – Tracks unique received addresses in `uniqueAddresses` for the mapping UI.
- **Multiple OSC values** – Supports indexed parameter selection within multi-value OSC messages.
- **MIDI-over-OSC** – Registers `BypassableInstance` processors at specific OSC addresses; MIDI messages extracted from OSC are delivered via `BypassableInstance::addMidiMessage()`.
- **Plugin parameter mapping** – `OscMapping` connects an OSC address to a plugin parameter.
- **Application command mapping** – `OscAppMapping` connects an OSC address to an `ApplicationCommandTarget` command ID.

A `CriticalSection` (`containerLock`) protects all containers against
concurrent access from the OSC network thread (reads) and the message thread
(mutations).

### OscInput

`OscInput` is a dummy `AudioPluginInstance` that appears on the graph canvas
as a visual placeholder. It has no audio buses and performs no processing.
The actual OSC reception is handled by `OscMappingManager` and the
`OSCReceiver` in `MainPanel`.

## Stability Infrastructure

### SafetyLimiter

`SafetyLimiterProcessor` (src/SafetyLimiter.h, src/SafetyLimiter.cpp) is a
final-stage audio processor that protects the output from dangerous signals.

Detection and protection features:

| Condition | Threshold | Hold Time | Action |
| --- | --- | --- | --- |
| Peak limiting | -0.5 dBFS (0.944 amplitude) | Per-sample | Soft limit |
| Dangerous gain | +6 dBFS (2.0 amplitude) | 100ms | Auto-mute |
| DC offset | >0.5 amplitude | 500ms | Auto-mute |
| Ultrasonic content | >18kHz sustained | 200ms | Auto-mute |

Once muted, the limiter requires manual unmute via the Panic command. The
`checkAndClearMuteTriggered()` method allows the UI to detect when a mute
event occurred and show a notification.

The limiter also provides input and output level metering (peak with decay
and VU-ballistic levels per IEC 60268-17) via `std::atomic<float>` values
read by the UI. A static singleton instance allows `PluginComponent` to
access output levels for the Audio Output VU display.

### CrossfadeMixer

`CrossfadeMixerProcessor` (src/CrossfadeMixer.h, src/CrossfadeMixer.cpp)
provides smooth audio crossfading during patch changes. It is inserted at the
end of the audio chain before output.

Patch switching sequence:

1. `startFadeOut(durationMs)` is called from the message thread before clearing the graph.
2. Audio fades to silence over the specified duration (default: 100ms).
3. The new patch is loaded while audio is silent.
4. `startFadeIn(durationMs)` is called after loading the new patch.
5. Audio fades back in over the specified duration.

All fade state uses atomics (`fading`, `fadingOut`, `fadeGain`,
`fadeIncrement`) for audio-thread safety. The default fade duration is
configurable via `setDefaultFadeDuration()` and persisted in settings.

### CrashProtection

`CrashProtection` (src/CrashProtection.h, src/CrashProtection.cpp) is a
singleton providing defensive crash protection for risky plugin operations.

Features:

- **SEH wrappers** – `executeWithProtection()` wraps operations in Structured Exception Handling on Windows to catch hardware exceptions (access violations, etc.).
- **Timeout protection** – `executeWithTimeout()` runs operations in a separate thread with a timeout. If the operation exceeds the timeout, it returns `TimedOperationResult::Timeout` and optionally auto-blacklists the plugin.
- **Combined protection** – `executeWithProtectionAndTimeout()` combines both SEH and timeout for maximum safety.
- **Auto-save** – `setAutoSaveCallback()` registers a callback triggered before risky operations.
- **Watchdog** – `startWatchdog()` launches a background thread that monitors UI responsiveness. `pingWatchdog()` is called periodically from the message thread; if pings stop, `isHangDetected()` returns true.
- **Operation context** – `setCurrentOperation()`/`clearCurrentOperation()` track what was happening for crash log diagnostics. `ScopedOperationContext` provides RAII management.
- **PROTECTED_OPERATION macro** – Wraps risky code with SEH protection.

### PluginBlacklist

`PluginBlacklist` (src/PluginBlacklist.h, src/PluginBlacklist.cpp) is a
singleton managing a user-configurable list of blacklisted plugins. Plugins
on the blacklist are skipped during scanning and not loaded.

Features:

- Blacklist by file path or by plugin identifier
- Path normalization for case-insensitive comparison on Windows
- Persistence via `SettingsManager` (load/save to JSON settings)
- Thread-safe access via `std::mutex`
- `BlacklistWindow` (src/BlacklistWindow.h) provides a UI for managing the blacklist

### PluginPoolManager

`PluginPoolManager` (src/PluginPoolManager.h, src/PluginPoolManager.cpp)
maintains a sliding window pool of preloaded plugin instances for instant
patch switching. See the Plugin Pool Loader Thread section above for details.

## Settings Persistence

### SettingsManager

`SettingsManager` (src/SettingsManager.h, src/SettingsManager.cpp) is a
thread-safe singleton that replaces the legacy `PropertiesSingleton`/
`PropertiesFile` system. Settings are stored as human-readable JSON using
the `nlohmann_json` library.

Settings file location:

| OS | Path |
| --- | --- |
| Windows | `%APPDATA%\Pedalboard3\settings.json` |
| Linux | `~/.config/Pedalboard3/settings.json` |

The `SettingsManager` provides typed getters (`getString`, `getBool`,
`getInt`, `getDouble`, `getStringArray`, `getXmlValue`) and setters that
auto-save after each call. A `std::mutex` protects the in-memory
`nlohmann::json` cache for thread-safe access.

The `PluginBlacklist` persists its entries through `SettingsManager` via
`loadFromSettings()` and `saveToSettings()`.

## Plugin Scanning

### Out-of-Process Scanner

The scanner runs as a separate console application (`Pedalboard3Scanner`)
built via `juce_add_console_app` in CMakeLists.txt. Its source is
src/PluginScannerMain.cpp.

The scanner registers a `VST3PluginFormat` and enters a message loop,
communicating with the host via Windows named pipes.

### IPC Protocol

`PluginScannerIPC.h` defines the communication protocol:

- Pipe name: `\\.\pipe\Pedalboard3PluginScanner`
- Protocol version: 1
- Magic number: `0x50444233` ("PDB3")
- Default timeout: 30000ms

Message types (host to scanner): `Ping`, `ScanPlugin`, `Shutdown`

Message types (scanner to host): `Pong`, `ScanResult`, `ScanError`,
`ScanCrash`, `Ready`

Scan result codes: `Success`, `LoadFailed`, `InvalidFormat`, `Timeout`,
`Crashed`, `Blacklisted`

Requests and responses are serialized as JSON (via `juce::JSON`).

### PluginScannerClient

`PluginScannerClient` (src/PluginScannerClient.h,
src/PluginScannerClient.cpp) manages the scanner process lifecycle on the
host side. It launches the scanner executable, sends scan requests, waits
for responses with timeout, and handles crashes by restarting the scanner.

### SafePluginScanner

`SafePluginScanner` (src/SafePluginScanner.h, src/SafePluginScanner.cpp)
wraps `PluginScannerClient` and integrates with JUCE's `KnownPluginList`.
It falls back to in-process scanning with timeout protection if the
out-of-process scanner is unavailable.

`SafePluginListComponent` is a drop-in replacement for JUCE's
`PluginListComponent` that uses `SafePluginScanner` internally.

## Build System

### CMake

The project uses CMake 3.24+ as its build system. The main CMakeLists.txt
defines two targets:

| Target | Type | Description |
| --- | --- | --- |
| `Pedalboard3` | GUI application (`juce_add_gui_app`) | Main plugin host |
| `Pedalboard3Scanner` | Console application (`juce_add_console_app`) | Out-of-process plugin scanner |

C++20 is required (`CMAKE_CXX_STANDARD 20`, `CMAKE_CXX_STANDARD_REQUIRED ON`,
`CMAKE_CXX_EXTENSIONS OFF`).

### CPM.cmake

Third-party dependencies are fetched via CPM.cmake at configure time. The
CPM script is included from `cmake/CPM.cmake`.

| Dependency | Version | Purpose |
| --- | --- | --- |
| fmt | 12.2.0 | Modern string formatting |
| spdlog | v1.17.0 | Fast async logging (uses fmt) |
| nlohmann_json | v3.12.0 | JSON parsing for settings |
| Catch2 | v3.15.3 | Unit testing framework (optional) |

### JUCE Submodule

JUCE is pinned to tag 8.0.15 as a git submodule at `JUCE/`. The
CMakeLists.txt checks for the submodule's existence and provides a
`FATAL_ERROR` with instructions if it is missing.

JUCE must remain at 8.0.15. JUCE 9.x has breaking API changes
(notably `Typeface::createSystemTypefaceFor` crashes on Windows with
DirectWrite, and `createFromSVG_string` is JUCE 9 only).

### Compile Definitions

Key compile definitions:

| Definition | Value | Purpose |
| --- | --- | --- |
| `JUCE_PLUGINHOST_VST3` | 1 | Enable VST3 hosting |
| `JUCE_PLUGINHOST_LADSPA` | 1 | Enable LADSPA hosting (Linux) |
| `JUCE_USE_FLAC` | 1 | FLAC audio format support |
| `JUCE_USE_OGGVORBIS` | 1 | Ogg Vorbis audio format support |
| `JUCE_MODAL_LOOPS_PERMITTED` | 1 | Allow modal loops in dialogs |
| `JUCE_STRICT_REFCOUNTEDPOINTER` | 1 | Strict ref-counted pointer checks |

Platform-specific audio backends:

| Platform | Definitions |
| --- | --- |
| Windows | `JUCE_ASIO`, `JUCE_WASAPI`, `JUCE_DIRECTSOUND`, `JUCE_WIN_PER_MONITOR_DPI_AWARE` |
| Linux | `JUCE_ALSA`, `JUCE_JACK_AUDIO_DEVICES` |

### JUCE Modules

The application links against these JUCE modules:

`juce_audio_basics`, `juce_audio_devices`, `juce_audio_formats`,
`juce_audio_processors`, `juce_audio_utils`, `juce_core`,
`juce_cryptography`, `juce_data_structures`, `juce_dsp`, `juce_events`,
`juce_graphics`, `juce_gui_basics`, `juce_gui_extra`, `juce_opengl`,
`juce_osc`

Plus recommended flags: `juce_recommended_config_flags`,
`juce_recommended_warning_flags`.

On Windows, `ws2_32` is linked for Winsock support (used by juce_osc).

## Platform Support

### Windows x64

- Generator: Visual Studio 17 2022, architecture x64
- Audio backends: ASIO, WASAPI, DirectSound
- Plugin formats: VST3
- Per-monitor DPI awareness enabled
- SEH-based crash protection

### Linux

- Generator: Ninja
- Compiler: GCC 11+ (or Clang 12+)
- Audio backends: ALSA, JACK
- Plugin formats: VST3, LADSPA
- Requires ALSA dev headers and JACK dev headers

## Source File Organization

Source files in CMakeLists.txt are organized into 9 groups:

| Group | Files | Purpose |
| --- | --- | --- |
| Application | `App.cpp/.h` | JUCE application entry point |
| Audio Core | `AudioSingletons`, `FilterGraph`, `InternalFilters`, `BypassableInstance`, `MainTransport`, `MidiAppFifo` | Audio engine and graph management |
| OSC | `Mapping`, `OscMappingManager`, `TapTempoHelper` | OSC reception and mapping (juce_osc) |
| Group 1: Utilities | `PropertiesSingleton`, `LogFile`, `JuceHelperStuff`, `Vectors`, `Images`, `LookAndFeelImages` | Shared utilities and image resources |
| Group 2: LookAndFeel | `ColourScheme`, `ColourSchemeEditor`, `BranchesLAF` | Visual theme and custom widget drawing |
| Group 3: Built-in processors | `PedalboardProcessors`, `PedalboardProcessorEditors`, `AudioRecorderControl`, `FilePlayerControl`, `LooperControl`, `LooperEditor`, `MetronomeControl`, `WaveformDisplay` | Internal audio processors and their UI controls |
| Group 4: Plugin canvas | `PluginComponent`, `PluginField` | Visual canvas for plugin routing |
| Group 5: Mappings | `MidiMappingManager`, `MappingsDialog`, `MappingEntryMidi`, `MappingEntryOsc`, `MappingSlider`, `MidiCcAlertWindow`, `ApplicationMappingsEditor` | MIDI and OSC mapping UI and logic |
| Group 6: Main UI | `MainPanel`, `AboutPage`, `LogDisplay`, `TapTempoBox`, `PresetBar` | Main application window and transport |
| Group 7: Patch/preset management | `PatchOrganiser`, `PresetManager`, `UserPresetWindow`, `PreferencesDialog`, `TrayIcon` | Patch list, presets, preferences, tray |
| Group 8: Stability infrastructure | `VuMeterDsp`, `SafetyLimiter`, `CrossfadeMixer`, `SettingsManager`, `PluginBlacklist`, `CrashProtection`, `UndoActions`, `BlacklistWindow` | Safety, persistence, crash protection, undo/redo |
| Group 9: Scanner infrastructure | `PluginScannerIPC`, `PluginScannerClient`, `SafePluginScanner`, `PluginPoolManager` | Out-of-process scanning and plugin pooling |