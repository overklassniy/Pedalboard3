# src/osc/

OSC (Open Sound Control) mapping infrastructure for Pedalboard3. This folder contains the base parameter-mapping class, the OSC-to-parameter dispatcher built on JUCE's `juce_osc` module, and a shared tap-tempo helper. It exists so that incoming OSC messages can drive plugin parameters, application commands, and MIDI-over-OSC routing.

## Contents

- `Mapping.cpp` / `Mapping.h` – abstract base class for all parameter mappings. Holds a `FilterGraph` pointer, a plugin id, and a parameter index; subclasses override `getXml()` for state serialization and call `updateParameter()` to apply normalised values.
- `OscMappingManager.cpp` / `OscMappingManager.h` – OSC dispatch engine and related mapping classes. `OscMapping` (extends `Mapping`) binds an OSC address to a plugin parameter; `OscAppMapping` binds an OSC address to an `ApplicationCommandTarget` command id; `OscMappingManager` receives `juce::OSCMessage` objects, dispatches float and MIDI arguments to registered mappings, tracks unique received addresses for address learning, and owns a `TapTempoHelper`. Also defines `OscInput`, a dummy `AudioPluginInstance` that serves as a visual placeholder on the graph canvas so users can see which plugins have OSC mappings; actual OSC reception is handled by a `juce::OSCReceiver` created in `app/MainPanel`.
- `TapTempoHelper.cpp` / `TapTempoHelper.h` – tap-tempo calculator that averages the intervals between the last four taps and returns BPM, resetting when the result falls below 30 BPM.

## Integration

The `juce_osc` module is linked to the main `Pedalboard3` target (see the root `CMakeLists.txt`). `OscMappingManager` is owned by `canvas/PluginField` (accessed via `PluginField::getOscManager()`), which is the viewed component of `app/MainPanel`'s viewport. `MainPanel` also owns the `juce::OSCReceiver` and registers itself as a `MessageLoopCallback` listener; when an OSC message arrives, `MainPanel` forwards it to `OscMappingManager::messageReceived()`. The mapping UI in `mappings/` (notably `mappings/MappingsDialog` and `mappings/ApplicationMappingsEditor`) creates and edits `OscMapping` and `OscAppMapping` instances through the manager. `TapTempoHelper` is shared between this folder's `OscMappingManager` and the MIDI mapping code in `mappings/MidiMappingManager` and `mappings/TapTempoBox`.
