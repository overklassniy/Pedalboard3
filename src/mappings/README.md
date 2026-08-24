# src/mappings/

MIDI and OSC mapping system and mapping UI components for Pedalboard3. This
folder holds the managers that route incoming MIDI CC and OSC messages to
plugin parameters and application commands, the dialogs used to view and edit
those mappings, and the supporting UI widgets.

## Contents

- `MidiMappingManager.h` / `MidiMappingManager.cpp` – the MIDI mapping core.
  Declares `MidiMapping` (a MIDI CC to plugin parameter mapping with latch,
  channel, and lower/upper bounds), `MidiAppMapping` (a MIDI CC to
  application command mapping), `MidiMappingManager` (dispatches incoming
  MIDI CC messages to registered mappings, supports a one-shot MIDI learn
  callback, and tracks tap tempo), and `MidiInterceptor` (an
  `AudioPluginInstance` with no audio buses that intercepts MIDI messages in
  the graph and forwards them to the manager).
- `MappingsDialog.h` / `MappingsDialog.cpp` – `MappingsDialog`, a dialog
  listing all MIDI and OSC mappings for a single plugin, with add/remove
  buttons, MIDI channel combo, OSC-over-MIDI address editor, and per-row
  MIDI learn.
- `MappingEntryMidi.h` / `MappingEntryMidi.cpp` – `MappingEntryMidi`, a single
  row component representing a MIDI CC mapping (CC combo, latch toggle,
  parameter combo, and a `MappingSlider` for the bound range).
- `MappingEntryOsc.h` / `MappingEntryOsc.cpp` – `MappingEntryOsc`, a single
  row component representing an OSC mapping (address combo, parameter combo,
  and an OSC parameter index slider).
- `MappingSlider.h` / `MappingSlider.cpp` – `MappingSlider`, a custom
  two-tick slider component (ported from the JUCE `Slider`) that supports
  inverted values and multiple slider styles, used by `MappingEntryMidi` for
  range selection.
- `MidiCcAlertWindow.h` / `MidiCcAlertWindow.cpp` – `MidiCcAlertWindow`, an
  `AlertWindow` override with a MIDI CC combo box and learn entry that
  registers a `MidiLearnCallback` to capture the next received CC.
- `ApplicationMappingsEditor.h` / `ApplicationMappingsEditor.cpp` –
  `ApplicationMappingsEditor`, a component with a tree view of application
  command categories and a reset button, used to edit application-level key,
  MIDI, and OSC mappings.
- `TapTempoBox.h` / `TapTempoBox.cpp` – `TapTempoBox`, a clickable component
  that calculates the tempo from repeated taps and sends the result to a
  `PluginField`.

## Integration

`MidiMappingManager` and `OscMappingManager` are owned by `canvas/PluginField`
(declared as members in `PluginField.h`); `app/MainPanel` accesses them
through `PluginField::getMidiManager`/`getOscManager` rather than owning them
directly. `MappingsDialog` is opened from `canvas/PluginComponent` (via
`PluginComponent::openMappingsWindow`) and edits the mappings for a single
graph node, using `MappingEntryMidi` and `MappingEntryOsc` for individual
rows. `ApplicationMappingsEditor` and `TapTempoBox` are instantiated from
`app/MainPanel`. `MappingSlider` is the custom slider used inside
`MappingEntryMidi`. `MidiCcAlertWindow` is used by `ApplicationMappingsEditor`
for MIDI CC learn. `ApplicationMappingsEditor` includes `util/Vectors.h` for
its button graphics.
