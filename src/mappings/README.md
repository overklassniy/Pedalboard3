# src/mappings/

MIDI and OSC mapping system and mapping UI components for Pedalboard3.

## Contents

- `MidiMappingManager.cpp` / `MidiMappingManager.h` — MIDI CC to parameter mapping manager (learn mode, latch/toggle, MIDI interception)
- `MappingsDialog.cpp` / `MappingsDialog.h` — mappings dialog window (lists all mappings, add/remove)
- `MappingEntryMidi.cpp` / `MappingEntryMidi.h` — single MIDI mapping entry row in the mappings dialog
- `MappingEntryOsc.cpp` / `MappingEntryOsc.h` — single OSC mapping entry row in the mappings dialog
- `MappingSlider.cpp` / `MappingSlider.h` — custom two-tick slider for mapping range selection (ported from JUCE Slider)
- `MidiCcAlertWindow.cpp` / `MidiCcAlertWindow.h` — alert window shown during MIDI CC learn
- `ApplicationMappingsEditor.cpp` / `ApplicationMappingsEditor.h` — application-level mappings editor (tree view of categories and mappings)
- `TapTempoBox.cpp` / `TapTempoBox.h` — tap tempo component (click to set BPM)

## Integration

`MidiMappingManager` is owned by `app/MainPanel` and routes incoming MIDI CC
messages to plugin parameters. `MappingsDialog` provides the UI for viewing
and editing all mappings, using `MappingEntryMidi` and `MappingEntryOsc` for
individual rows. `MappingSlider` is the custom slider used in mapping entries
for range selection. `ApplicationMappingsEditor` provides a tree-based view
for managing application-level mappings.
