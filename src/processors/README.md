# src/processors/

Built-in processors, editors, controls, and DSP safety for Pedalboard3.

## Contents

### Processor base and umbrella headers
- `PedalboardProcessor.h` — abstract base class for all internal processors
- `PedalboardProcessors.h` / `.cpp` — umbrella header (includes all per-processor headers)
- `PedalboardProcessorEditors.h` / `.cpp` — umbrella header (includes all per-editor headers)

### Per-class processor files (split from PedalboardProcessors)
- `LevelProcessor.h` / `.cpp` — level control processor
- `FilePlayerProcessor.h` / `.cpp` — audio file playback processor
- `OutputToggleProcessor.h` / `.cpp` — output toggle processor
- `VuMeterProcessor.h` / `.cpp` — VU meter processor
- `RecorderProcessor.h` / `.cpp` — audio recorder processor
- `MetronomeProcessor.h` / `.cpp` — metronome processor
- `LooperProcessor.h` / `.cpp` — looper processor

### Per-class editor files (split from PedalboardProcessorEditors)
- `LevelEditor.h` / `.cpp` — LevelControl and LevelEditor
- `FilePlayerEditor.h` / `.cpp` — FilePlayerEditor
- `OutputToggleEditor.h` / `.cpp` — OutputToggleControl and OutputToggleEditor
- `VuMeterEditor.h` / `.cpp` — VuMeterControl and VuMeterEditor
- `AudioRecorderEditor.h` / `.cpp` — AudioRecorderEditor
- `MetronomeEditor.h` / `.cpp` — MetronomeEditor

### Control components
- `AudioRecorderControl.cpp` / `.h` — audio recorder control UI
- `FilePlayerControl.cpp` / `.h` — file player control UI
- `LooperControl.cpp` / `.h` — looper control UI
- `LooperEditor.cpp` / `.h` — looper full editor UI
- `MetronomeControl.cpp` / `.h` — metronome control UI
- `WaveformDisplay.cpp` / `.h` — waveform display component

### DSP safety
- `VuMeterDsp.h` — VU meter DSP (2-pole lowpass, IEC 60268-17 integration)
- `SafetyLimiter.cpp` / `.h` — final output safety limiter (soft-limit, auto-mute on dangerous levels, DC offset detection, ultrasonic detection)
- `CrossfadeMixer.cpp` / `.h` — glitch-free crossfade mixer for patch switching

## Integration

All processors extend `PedalboardProcessor` (the abstract base) and are
registered with the plugin format manager via `audio/InternalFilters`. Each
processor provides a control component (shown on the canvas) and optionally
a full editor (shown in a separate window). The DSP safety components
(`SafetyLimiter`, `CrossfadeMixer`, `VuMeterDsp`) protect the audio output
and are integrated into the audio processing chain by `audio/FilterGraph`.
