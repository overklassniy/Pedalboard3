# src/audio/

Audio engine core for Pedalboard3. Contains the `AudioProcessorGraph`
wrapper that manages the plugin signal chain, singleton wrappers for
shared JUCE format managers and plugin lists, the internal plugin format
for I/O nodes, a bypass-capable plugin instance wrapper, the main
transport singleton, and a lock-free MIDI FIFO for audio-to-message-thread
communication.

## Contents

- `AudioSingletons.cpp` / `AudioSingletons.h` – singleton wrappers for `AudioPluginFormatManager`, `AudioFormatManager`, `AudioThumbnailCache`, and `KnownPluginList` (the last is set by `MainPanel` on startup rather than owned)
- `FilterGraph.cpp` / `FilterGraph.h` – `FilterGraph`, a `FileBasedDocument` wrapping JUCE's `AudioProcessorGraph` that manages plugin nodes, connections, and XML serialization; also defines the `.filtergraph` file extension and wildcard
- `InternalFilters.cpp` / `InternalFilters.h` – `InternalPluginFormat`, an `AudioPluginFormat` that registers the built-in I/O nodes (audio input, audio output, MIDI input, OSC input) with the plugin format manager
- `BypassableInstance.cpp` / `BypassableInstance.h` – `BypassableInstance`, an `AudioPluginInstance` wrapper that adds a smooth bypass ramp, per-plugin MIDI channel filtering, and OSC-injected MIDI messages; bypass state is `std::atomic` for cross-thread safety
- `MainTransport.cpp` / `MainTransport.h` – `MainTransport`, a `ChangeBroadcaster` singleton that coordinates play/stop/return-to-zero state across all registered transports
- `MidiAppFifo.cpp` / `MidiAppFifo.h` – `MidiAppFifo`, a lock-free FIFO using `juce::AbstractFifo` with a `SpinLock` for multi-producer safety, passing command IDs, tempo changes, patch changes, and deferred parameter changes from the audio thread to the message thread

## Integration

`FilterGraph` is the core audio engine. It is owned by `MainPanel` (in
`app/`) as the `signalPath` member and played through a
`juce::AudioProcessorPlayer`. `BypassableInstance` wraps hosted plugins
inside the graph to provide bypass and MIDI channel filtering.

`InternalPluginFormat` registers I/O nodes (audio input, audio output,
MIDI input, and `OscInput`) so they appear in the plugin list. `OscInput`
is defined in `osc/OscMappingManager.h`, which `InternalFilters.cpp`
includes directly. The built-in processors from `processors/` are not yet
registered here — their integration is planned for a later phase.

`AudioSingletons` provides shared format managers and the plugin list
used across the application. `MainTransport` is a JUCE singleton accessed
by transport-aware processors and `MainPanel`. `MidiAppFifo` is owned by
`MainPanel` and used to defer audio-thread events to the message thread.

All files in this directory are compiled into the `Pedalboard3` GUI
application target.

## Constraints

- `BypassableInstance::createEditor()` calls `plugin->createEditorAndMakeActive()` because JUCE 8 made `AudioProcessor::createEditor()` private.
- `FilterGraph` uses `AudioProcessorGraph::NodeID` (JUCE 8 API); node lookups use `node->nodeID.uid`.
- `MidiAppFifo` uses `juce::AbstractFifo` (single-producer/single-consumer) with a `juce::SpinLock` on the producer side because multiple threads (MIDI audio thread and OSC network thread) may write concurrently.
