# User guide

How to use Pedalboard3 as a performer. This guide describes the running
application: patches, presets, the built-in processors, MIDI and OSC
mapping, tap tempo, the colour scheme editor, preferences, the system
tray, and the log display.

## Patches and graph saves

A Pedalboard3 document is a multi-patch `.pdl` file. Each patch is a
complete plugin signal chain (a `FilterGraph`). Individual graph saves
use the `.filtergraph` extension.

- Use the patch bar to switch between patches in the current document.
- The **Patch Organiser** (Options menu) lists the patches in the
  current document with buttons to add, copy, remove, reorder, and
  import patches from other `.pdl` files. Patch names are editable by
  double-clicking.
- Patch switching is designed to be near-instant via the
  `PluginPoolManager`, which preloads plugin instances for the current
  patch plus N patches ahead or behind.

## The canvas

The main canvas (`PluginField`) shows the plugin nodes and the audio
cables connecting them. Each plugin is a `PluginComponent` with a
bypass button, a preset bar, and an editor window. Audio connections
are drawn in the audio connection colour; parameter connections are
drawn in the parameter connection colour.

## Built-in processors

Pedalboard3 ships with built-in processors (in addition to hosted VST3
and LADSPA plugins):

- **Level** – volume control.
- **File Player** – plays an audio file from disk, with a waveform
  display and transport controls.
- **Output Toggle** – toggles between two output destinations.
- **VU Meter** – shows the level of the passing audio.
- **Audio Recorder** – writes the incoming audio to disk.
- **Metronome** – generates click samples in sync with the playhead
  tempo.
- **Looper** – records input to disk and memory and plays it back.

Each processor has an on-canvas control component and, where applicable,
a full editor window opened from the plugin node.

## Presets

Per-plugin presets use the `.fxp` format:

- The **Preset Bar** above each plugin editor shows factory and
  user-saved presets with import and save buttons.
- The **User Preset Window** (Options menu) is the full preset manager:
  a tree view of plugin directories and their `.fxp` presets with
  buttons to copy, remove, import, export, and rename. User presets are
  stored under `<user data>/Pedalboard3/presets/<plugin name>/`.

## MIDI mapping

MIDI CC messages can drive plugin parameters and application commands:

- Open the **Mappings Dialog** from a plugin node to view and edit the
  MIDI and OSC mappings for that single plugin. Each MIDI row has a CC
  combo, a latch toggle, a parameter combo, and a `MappingSlider` for
  the bound range.
- Use **MIDI Learn** to capture the next received CC for a mapping.
- The **Application Mappings Editor** (Options menu) edits
  application-level key, MIDI, and OSC mappings via a tree view of
  command categories.
- `MidiInterceptor` is a no-audio plugin that intercepts MIDI messages
  in the graph and forwards them to the `MidiMappingManager`.

## OSC mapping

OSC messages can drive plugin parameters and application commands over
the network:

- Configure the OSC port and multicast address in the **Preferences**
  dialog.
- `OscInput` appears on the canvas as a visual placeholder so you can
  see which plugins have OSC mappings; actual reception is handled by a
  `juce::OSCReceiver` in the main panel.
- OSC mappings bind an OSC address to a plugin parameter
  (`OscMapping`) or to an application command (`OscAppMapping`). Float
  and MIDI arguments are dispatched to the registered mappings.

## Tap tempo

Tap tempo is shared between the MIDI and OSC mapping systems. The
**Tap Tempo Box** calculates the tempo from repeated taps and sends the
result to the canvas. `TapTempoHelper` averages the intervals between
the last four taps and returns BPM, resetting below 30 BPM.

## Colour scheme

The application uses a custom LookAndFeel (`BranchesLAF`) fed by a
named-colour palette (`ColourScheme`). The default scheme is a warm
light beige palette.

- Open the **Colour Scheme Editor** (Options menu) to choose, save, and
  delete `.colourscheme` presets. The editor has a colour selector, a
  colour list, and a combo box plus buttons for managing presets.
- Colour scheme presets are stored as `.colourscheme` files in the
  application data folder and loaded at startup.

## Audio settings

The **Audio Settings** dialog (Options menu) configures the audio
interface used by Pedalboard3:

- **Audio device type** – selects the audio backend. On Windows the
  available types are WASAPI (shared), WASAPI (exclusive), WASAPI
  (shared low latency), DirectSound, and ASIO. On Linux the available
  types are ALSA and JACK. On macOS the available type is CoreAudio.
- **Output / Input device** – selects the physical audio interface.
  The **Test** button plays a short test tone through the selected
  output device.
- **Sample rate / Buffer size** – advanced settings shown when the
  "Show advanced settings..." button is toggled.
- **Control Panel** – opens the selected device's own control panel.
  This button only appears for devices that provide a control panel
  (e.g. ASIO devices). It is hidden for WASAPI and DirectSound.
- **Reset Device** – restarts the audio device, sometimes needed after
  changing properties in the device's custom control panel.

Audio settings are saved to the application properties and restored on
the next launch. When the "save audio settings in `.pdl` files"
preference is enabled, the audio device state is also embedded in the
`.pdl` document.

## Preferences

The **Preferences** dialog (Options menu) configures:

- OSC port and multicast address
- Visible I/O nodes (audio, MIDI, OSC)
- MIDI options (program change, MMC transport)
- Window options (auto mappings window, loop patches, windows on top,
  ignore pin names, tray icon, start in tray, fixed-size windows)
- Whether to save audio settings in `.pdl` files

Preferences are read and written through the `PropertiesSingleton`
(JUCE `PropertiesFile`).

## System tray

On Windows and Linux, `TrayIcon` provides a system tray icon with a
right-click popup menu and double-click window toggle. The tray icon is
not built on macOS (`TrayIcon.h` and `TrayIcon.cpp` are guarded by
`#ifndef JUCE_MAC`).

## Dialog windows

Alert and confirmation dialogs (including the "Closing document..." save
prompt shown when quitting or closing the main window) use the operating
system's native message box API rather than JUCE's custom `AlertWindow`.

This is a deliberate choice for responsiveness: JUCE 8's `AlertWindow`
constructor calls `WindowUtils::areThereAnyAlwaysOnTopWindows()`, which
on Windows invokes `EnumWindows()` — a system-wide enumeration of every
top-level window. On a system with many open applications this adds a
perceptible delay before the dialog appears. Native message boxes
(`TaskDialog` on Windows, `NSAlert` on macOS) bypass this enumeration
entirely and appear near-instantly, matching the behaviour of the
original Pedalboard2 project which was built against JUCE 2.x where the
same check used a fast internal loop over JUCE desktop components.

The MIDI CC learn dialog (`MidiCcAlertWindow`) is an exception: it
subclasses `AlertWindow` directly to embed a combo box and MIDI learn
callback, so it continues to use the JUCE `AlertWindow` appearance.

## Log display

The **Log Display** is a component with a read-only text editor,
start/stop logging button, and event-type filter toggles. It displays
events from the `LogFile` singleton, which is useful for diagnosing
audio, MIDI, OSC, and plugin issues during a session.

## Stability features

Several stability features protect the host during live use:

- **Crash protection** wraps risky operations (notably plugin scanning)
  with SEH on Windows and a worker-thread timeout, with an auto-save
  callback hook and a watchdog thread for UI hangs.
- **Plugin blacklist** skips known-bad plugins during scanning and is
  auto-populated when the scanner crashes on a plugin. Manage it through
  the **Blacklist Window**.
- **Undo/redo** actions for filter graph editing (add/remove plugin,
  add/remove connection) are defined and designed for a
  `juce::UndoManager`.

See [architecture.md](architecture.md) for which of these are fully
wired into the UI today.
