# Pedalboard3 User Guide

This document describes the features and usage of Pedalboard3, a standalone
VST3 plugin host for live performance.

## Overview

Pedalboard3 lets you load VST3 plugins, wire them together on a visual
canvas, and switch between patches during live performance. It preserves
the user interface and workflow of the original Pedalboard2 while running
on a modernized audio engine.

Key features:

- Visual plugin canvas with drag-to-connect routing
- VST3 plugin hosting (Windows and Linux), LADSPA on Linux
- Patch management with save, load, and instant switching
- MIDI CC mapping with learn mode, custom ranges, and latch/toggle
- OSC control via JUCE's juce_osc module
- Built-in processors: file player, looper, metronome, audio recorder,
  level control, output toggle, VU meter
- Colour scheme customization with presets
- Safety limiter with auto-mute and panic
- Crossfade patch switching for glitch-free transitions
- Out-of-process plugin scanner with crash protection and blacklist
- System tray icon
- JSON-based settings persistence

## Adding Plugins to the Canvas

To add a plugin or built-in processor to the canvas:

1. Double-click on an empty area of the plugin field (canvas).
2. A plugin selection menu appears, listing available VST3 plugins and
   built-in processors.
3. Select the desired plugin. It appears on the canvas at the click
   position.

Alternatively, you can drag plugin files (.vst3) from your file manager
directly onto the canvas.

To delete a plugin, click the red delete button (X icon) on the plugin
component.

To move a plugin, click and drag its body to the desired position on the
canvas.

## Creating Connections Between Plugins

Connections route audio and MIDI between plugins on the canvas.

### Audio Connections

1. Locate the output pin on the source plugin (right side, labeled with
   channel names).
2. Click and hold on the output pin.
3. Drag to the input pin on the destination plugin (left side).
4. Release the mouse button to create the connection.

A bezier curve is drawn between the connected pins.

### Connect All Outputs

Hold Shift while dragging from an output pin to connect all outputs from
the source to all inputs of the destination in sequence.

### MIDI Connections

MIDI connections use the special MIDI pin (separate from audio pins).
Drag from a MIDI output pin to a MIDI input pin to route MIDI messages
between plugins.

### Parameter Connections

Parameter pins allow mapping one plugin's parameter to another. Drag from
a parameter output pin to a parameter input pin.

### Deleting Connections

Click on a connection curve to select it, then press Delete or use the
Edit > Delete Connection menu command.

## Patch Management

A patch is a complete snapshot of the plugin graph: all plugins, their
positions, connections, parameter states, and mappings.

### Saving Patches

- Use File > Save or File > Save As to save the current set of patches to
  a `.pdl` file.
- Use File > Save As Default to save the current patches as the default
  that loads on startup.
- Use File > Reset Default to clear the default and start fresh.

### Loading Patches

- Use File > Open to load a `.pdl` patch file.
- Drag and drop a `.pdl` file onto the main window.

### Switching Patches

Use the patch bar at the bottom of the window:

- Click the Previous/Next buttons to navigate.
- Select a patch from the combo box dropdown.
- Edit the patch name by clicking in the combo box.

### Organizing Patches

Use Edit > Organise Patches to open the Patch Organiser window:

- Add – Creates a new empty patch.
- Copy – Duplicates the selected patch.
- Remove – Deletes the selected patch.
- Move Up / Move Down – Reorders patches in the list.
- Import – Imports a patch from another `.pdl` file.

Patch names can be edited by double-clicking the row label.

### MIDI Program Change Switching

When enabled in Preferences, MIDI Program Change messages switch patches.
Program Change 0 selects the first patch, 1 selects the second, and so on.
If a Program Change is received for a patch index that does not exist, a
warning notification is displayed.

## MIDI Mapping

MIDI CC (Continuous Controller) messages can be mapped to plugin
parameters and application commands.

### CC Learn

1. Right-click on a plugin's parameter pin and select "Add Mapping" (or
   use the mappings button on the plugin component).
2. In the Mappings dialog, click the Learn button.
3. Move a MIDI controller (knob, slider, wheel) to assign its CC to the
   mapping.

### Mapping Ranges

Each MIDI mapping supports custom lower and upper bounds for the parameter
range. This lets you map a portion of a knob's travel to a portion of a
parameter's range.

- Lower Bound – The parameter value when the CC is at 0.
- Upper Bound – The parameter value when the CC is at 127.

### Latch and Toggle

- Latch – When enabled, the CC value is captured and held. Useful for
  momentary buttons and footswitches.
- Toggle – When latched, the mapping toggles between its bounds on each
  CC trigger.

### MIDI Channel

Each mapping can be restricted to a specific MIDI channel (1-16) or set
to omni (0) to respond to all channels.

### Application Command Mapping

MIDI CCs can also be mapped to application commands (e.g., patch next,
transport play) via the Application Mappings editor.

## OSC Mapping

Pedalboard3 receives OSC (Open Sound Control) messages via JUCE's
juce_osc module for remote control.

### Configuration

Set the OSC port and optional multicast address in Preferences
(Options > Preferences):

- OSC Port – The UDP port to listen on.
- OSC Multicast – Optional multicast address for receiving group messages.

### Plugin Parameter Mapping

OSC addresses can be mapped to plugin parameters:

1. Right-click on a plugin's parameter pin and select "Add Mapping".
2. In the Mappings dialog, configure the OSC address and parameter index.

When an OSC message is received at the mapped address, the float value is
sent to the plugin parameter.

### Application Command Mapping

OSC addresses can be mapped to application commands via the Application
Mappings editor.

### MIDI-over-OSC

Plugins can receive MIDI messages delivered via OSC. Register a
`BypassableInstance` at a specific OSC address, and MIDI messages
extracted from OSC arguments are injected into the plugin's processing.

### Address Learning

The OSC mapping manager tracks all unique addresses received, making them
available in the mapping UI for quick selection.

## Built-in Processors

Pedalboard3 includes several built-in processors that appear alongside VST3
plugins in the selection menu.

### File Player

Plays back an audio file with transport controls:

- Play/Pause
- Return to Zero
- Loop toggle
- Read position slider
- Sync to Main Transport

Supports WAV, FLAC, and Ogg Vorbis formats.

### Looper

Records and loops audio with the following controls:

- Play/Pause
- Return to Zero
- Record
- Read position
- Sync to Main Transport
- Stop After Bar
- Auto Play (automatically plays after recording stops)
- Bar numerator/denominator (time signature)
- Input level
- Loop level

The looper uses a segmented memory buffer system to handle arbitrary loop
lengths without audio-thread allocations. A background thread pre-allocates
new buffer segments as needed.

### Metronome

Click track with configurable time signature:

- Play/Pause
- Numerator/Denominator (time signature)
- Sync to Main Transport
- Custom accent and click sound files (or built-in sine wave default)

### Audio Recorder

Records audio to a file:

- Record toggle
- Sync to Main Transport
- Waveform display

### Level Control

Simple volume control with a single slider (0.0 to 1.0).

### Output Toggle

Toggles between two audio outputs with a smooth fade.

### VU Meter

Stereo VU meter showing left and right channel levels.

## Colour Scheme Customization

Pedalboard3 supports customizable colour schemes via Options > Colour
Schemes.

- Select from preset colour schemes.
- Edit individual colours using the Colour Scheme Editor.
- Save custom presets for later use.

The colour scheme is stored in a `std::map<String, Colour>` and persisted
in the settings file.

## Preferences

The Preferences dialog (Options > Preferences) provides the following
settings:

### OSC Settings

- OSC Port – UDP port for OSC reception.
- OSC Multicast – Multicast address (optional).

### I/O Options

- Audio Input – Show/hide the audio input node.
- MIDI Input – Show/hide the MIDI input node.
- OSC Input – Show/hide the OSC input node.

### MIDI Options

- MIDI Program Change – Enable patch switching via MIDI Program Change.
- MMC Transport – Enable MIDI Machine Code transport control.

### Other Options

- Auto Mappings Window – Automatically open the mappings window when a
  parameter connection is made.
- Loop Patches – Loop back to the first patch after the last one.
- Windows On Top – Keep plugin editor windows on top of the main window.
- Ignore Pin Names – Hide channel name labels on plugin pins.
- Use Tray Icon – Show a system tray icon.
- Start in Tray – Start minimized to the system tray.
- Fixed Size – Disable window resizing.
- PDL Audio Settings – Persist audio device settings in the patch file.

## Plugin Blacklist Management

The plugin blacklist prevents problematic plugins from being loaded or
scanned, protecting application stability.

### Accessing the Blacklist

Open the blacklist management window from the Options menu or when a
plugin crash is detected.

### Managing Blacklisted Plugins

- Add – Manually add a plugin path or identifier to the blacklist.
- Remove – Remove a plugin from the blacklist.
- Clear – Remove all plugins from the blacklist.

### Automatic Blacklisting

Plugins are automatically added to the blacklist when:

- They crash the out-of-process scanner.
- They cause a timeout during loading (via CrashProtection).
- They trigger a crash during operation (via CrashProtection).

The blacklist persists across sessions in the settings file.

## Safety Limiter

The SafetyLimiterProcessor protects the audio output from dangerous
signals. It runs as the final stage in the audio chain.

### Auto-Mute Conditions

| Condition | Threshold | Hold Time | Action |
| --- | --- | --- | --- |
| Dangerous gain | +6 dBFS | 100ms | Auto-mute |
| DC offset | >0.5 amplitude | 500ms | Auto-mute |
| Ultrasonic content | >18kHz sustained | 200ms | Auto-mute |

### Peak Limiting

Signals above -0.5 dBFS are soft-limited to prevent clipping.

### Panic (Manual Unmute)

When the safety limiter mutes the output, it requires manual unmute via
the Panic command. This prevents sudden loud audio from surprising the
user after a dangerous condition has been resolved.

### Mute Notification

When a mute event occurs, a toast notification is displayed. The
`checkAndClearMuteTriggered()` method is used by the UI to detect mute
events.

## Crossfade Patch Switching

Patch changes are smoothed by the CrossfadeMixerProcessor to avoid audio
glitches.

### How It Works

1. When a patch switch is triggered, the crossfade mixer fades audio out
   over the configured duration (default: 100ms).
2. The new patch is loaded while audio is silent.
3. Audio fades back in over the same duration.

The fade duration is configurable and persisted in settings. All fade
state uses atomics for audio-thread safety.

### Plugin Pool Preloading

The PluginPoolManager preloads plugins for patches ahead of and behind the
current setlist position in a background thread. This reduces or eliminates
loading delays during patch switches.

- Default preload range: 2 patches ahead.
- Configurable memory limit (0 = unlimited).
- Progress reporting via listener callbacks.

## Tray Icon

On Windows, Pedalboard3 can show a system tray icon.

- Right-click the tray icon for a popup menu.
- Double-click the tray icon to show or hide the main window.

Enable the tray icon in Preferences (Options > Preferences > Use Tray
Icon). Enable "Start in Tray" to start the application minimized to the
tray.

## Settings File Location

Settings are stored as human-readable JSON.

| OS | Path |
| --- | --- |
| Windows 10/11 | `%APPDATA%\Pedalboard3\settings.json` |
| Linux | `~/.config/Pedalboard3/settings.json` |

The settings file contains:

- Audio device configuration
- OSC port and multicast address
- Plugin blacklist entries
- Colour scheme selection
- Window positions and sizes
- Crossfade duration
- Plugin pool configuration
- Other application preferences

Settings are auto-saved after each change. The file can be edited manually
while the application is closed, but manual edits while the application is
running will be overwritten.