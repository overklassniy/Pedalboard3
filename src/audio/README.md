# src/audio/

Audio engine core for Pedalboard3.

## Contents

- `AudioSingletons.cpp` / `AudioSingletons.h` — singletons for AudioPluginFormatManager, AudioFormatManager, AudioThumbnailCache, and KnownPluginList
- `FilterGraph.cpp` / `FilterGraph.h` — AudioProcessorGraph wrapper (the audio engine that manages plugin nodes and connections)
- `InternalFilters.cpp` / `InternalFilters.h` — InternalPluginFormat (registers built-in processors with the plugin format manager)
- `BypassableInstance.cpp` / `BypassableInstance.h` — wrapper around AudioPluginInstance that adds bypass functionality
- `MainTransport.cpp` / `MainTransport.h` — main transport controller (play/stop/record state)
- `MidiAppFifo.cpp` / `MidiAppFifo.h` — MIDI message FIFO for thread-safe MIDI passing

## Integration

`FilterGraph` is the core audio engine. It is owned by `MainPanel` (in
`app/`) and uses `BypassableInstance` to wrap hosted plugins. `InternalFilters`
registers the built-in processors from `processors/` so they appear in the
plugin list. `AudioSingletons` provides shared format managers used across
the application.
