# src/osc/

OSC (Open Sound Control) mapping for Pedalboard3.

## Contents

- `Mapping.cpp` / `Mapping.h` — base Mapping class (represents a single parameter mapping with source, target, and range)
- `OscMappingManager.cpp` / `OscMappingManager.h` — OSC to parameter mapping manager (uses juce_osc module, manages OscInput and mapping state)
- `TapTempoHelper.cpp` / `TapTempoHelper.h` — tap tempo calculation helper (used by OSC and MIDI tempo mapping)

## Integration

`OscMappingManager` receives OSC messages via JUCE's `juce_osc` module and
routes them to plugin parameters through the `Mapping` abstraction. It is
owned by `MainPanel` (in `app/`) and used by the mapping UI in `mappings/`.
`TapTempoHelper` is shared between OSC and MIDI tempo mapping code.
