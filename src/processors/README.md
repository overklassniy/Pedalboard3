# src/processors/

Built-in processors, their editors and controls, and DSP safety modules for
Pedalboard3. This folder holds the internal audio processor types that ship
with the host, the UI components that represent them on the canvas and in
editor windows, and standalone DSP modules that protect the audio output.

## Contents

### Processor base and umbrella headers
- `PedalboardProcessor.h` – `PedalboardProcessor`, the abstract base class
  for all internal processors (extends `AudioPluginInstance`); declares the
  pure virtual `getControls()` and `getSize()` used to build the on-canvas
  control component.
- `PedalboardProcessors.h` / `PedalboardProcessors.cpp` – umbrella header that
  includes `PedalboardProcessor.h` and every per-class processor header
  (`FilePlayerProcessor`, `LevelProcessor`, `LooperProcessor`,
  `MetronomeProcessor`, `OutputToggleProcessor`, `RecorderProcessor`,
  `VuMeterProcessor`).
- `PedalboardProcessorEditors.h` / `PedalboardProcessorEditors.cpp` – umbrella
  header that includes every per-class editor header
  (`AudioRecorderEditor`, `FilePlayerEditor`, `LevelEditor`,
  `MetronomeEditor`, `OutputToggleEditor`, `VuMeterEditor`).

### Per-class processor files
- `LevelProcessor.h` / `LevelProcessor.cpp` – `LevelProcessor`, a simple
  level (volume) control processor.
- `FilePlayerProcessor.h` / `FilePlayerProcessor.cpp` – `FilePlayerProcessor`,
  an audio file playback processor (also a `ChangeBroadcaster`/
  `ChangeListener`) wrapping a `AudioTransportSource`.
- `OutputToggleProcessor.h` / `OutputToggleProcessor.cpp` –
  `OutputToggleProcessor`, a processor that toggles between two output
  destinations.
- `VuMeterProcessor.h` / `VuMeterProcessor.cpp` – `VuMeterProcessor`, a
  processor that drives a VU meter from the passing audio.
- `RecorderProcessor.h` / `RecorderProcessor.cpp` – `RecorderProcessor`, an
  audio recorder processor (also a `ChangeBroadcaster`/`ChangeListener`)
  that writes the incoming audio to disk.
- `MetronomeProcessor.h` / `MetronomeProcessor.cpp` – `MetronomeProcessor`,
  a metronome processor (also a `ChangeBroadcaster`/`ChangeListener`) that
  generates click samples in sync with the playhead tempo.
- `LooperProcessor.h` / `LooperProcessor.cpp` – `LooperProcessor`, a basic
  looper processor that records input to disk and memory and plays it back,
  using an array of fixed-size buffers to avoid audio-thread allocations.

### Per-class editor files
- `LevelEditor.h` / `LevelEditor.cpp` – `LevelControl` (the on-canvas
  component) and `LevelEditor` (the `AudioProcessorEditor` window).
- `FilePlayerEditor.h` / `FilePlayerEditor.cpp` – `FilePlayerEditor`, the
  editor window for the file player.
- `OutputToggleEditor.h` / `OutputToggleEditor.cpp` – `OutputToggleControl`
  (the on-canvas component) and `OutputToggleEditor` (the editor window).
- `VuMeterEditor.h` / `VuMeterEditor.cpp` – `VuMeterControl` (the on-canvas
  meter) and `VuMeterEditor` (the editor window).
- `AudioRecorderEditor.h` / `AudioRecorderEditor.cpp` – `AudioRecorderEditor`,
  the editor window for the recorder.
- `MetronomeEditor.h` / `MetronomeEditor.cpp` – `MetronomeEditor`, the editor
  window for the metronome.

### Control components
- `AudioRecorderControl.h` / `AudioRecorderControl.cpp` –
  `AudioRecorderControl`, the on-canvas control component for the recorder.
- `FilePlayerControl.h` / `FilePlayerControl.cpp` – `FilePlayerControl`, the
  on-canvas control component for the file player.
- `LooperControl.h` / `LooperControl.cpp` – `LooperControl`, the on-canvas
  control component for the looper.
- `LooperEditor.h` / `LooperEditor.cpp` – `LooperEditor`, the full editor
  window for the looper (not included in the editors umbrella header).
- `MetronomeControl.h` / `MetronomeControl.cpp` – `MetronomeControl`, the
  on-canvas control component for the metronome.
- `WaveformDisplay.h` / `WaveformDisplay.cpp` – `WaveformDisplay`, a
  `Component`/`ChangeListener`/`ChangeBroadcaster` that draws an audio
  thumbnail waveform, used by the file player and looper editors.

### DSP safety
- `VuMeterDsp.h` – `VuMeterDsp`, a standalone VU meter DSP class implementing
  a critically damped 2-pole lowpass at ~3.5 Hz for the 300 ms IEC 60268-17
  VU ballistics; `process()` is called from the audio thread and `read()`
  from the UI thread. Header-only, no `.cpp`.
- `SafetyLimiter.h` / `SafetyLimiter.cpp` – `SafetyLimiterProcessor`, a final
  output safety `AudioProcessor` (not a `PedalboardProcessor`) that
  soft-limits peaks above -0.5 dBFS, auto-mutes on sustained dangerous
  levels, DC offset, or ultrasonic content, and exposes thread-safe
  input/output peak and VU level accessors; uses `VuMeterDsp` internally.
- `CrossfadeMixer.h` / `CrossfadeMixer.cpp` – `CrossfadeMixerProcessor`, an
  `AudioProcessor` (not a `PedalboardProcessor`) that applies an atomic
  fade-out/fade-in gain ramp for glitch-free patch switching.

## Integration

All seven per-class processors extend `PedalboardProcessor` and provide a
controls component (shown on the canvas via `PluginComponent`) and, where
applicable, a full `AudioProcessorEditor` window. They are intended for
registration with the plugin format manager through `audio/InternalFilters`
(`InternalPluginFormat`), which currently only provides the audio input,
audio output, MIDI input, and OSC input I/O filters; the per-class processor
filter types are declared but not yet enabled there. The umbrella headers
`PedalboardProcessors.h` and `PedalboardProcessorEditors.h` are included by
`canvas/PluginField.cpp`, `canvas/PluginComponent.cpp`, `app/MainPanel.cpp`,
and several control `.cpp` files in this folder so that code referencing the
original monolithic headers still compiles. The control components use
embedded SVG button graphics from `util/Vectors` (included by
`AudioRecorderControl.cpp`, `FilePlayerControl.cpp`, `LooperControl.cpp`,
`LooperEditor.cpp`, `MetronomeControl.cpp`, and `OutputToggleEditor.cpp`).

The DSP safety modules (`SafetyLimiterProcessor`, `CrossfadeMixerProcessor`,
`VuMeterDsp`) are self-contained within this folder: `SafetyLimiterProcessor`
includes `VuMeterDsp.h` internally, and none of the three are currently
referenced outside `src/processors/` (they are not wired into
`audio/FilterGraph`).

## Constraints

- Umbrella-header backward compatibility: `PedalboardProcessors.h`/`.cpp` and
  `PedalboardProcessorEditors.h`/`.cpp` remain as umbrella headers that
  include the per-class headers, preserving the original monolithic include
  surface; the per-class files are the source of truth and the `.cpp` umbrella
  files contain only the license header.
- `SafetyLimiterProcessor` and `CrossfadeMixerProcessor` extend
  `juce::AudioProcessor` directly, not `PedalboardProcessor`, so they do not
  implement `getControls()`/`getSize()` and are not part of the
  `PedalboardProcessors.h` umbrella.
- `VuMeterDsp.h` is header-only (no `.cpp`); all of its logic is inline.
