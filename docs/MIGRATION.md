# Pedalboard3 Migration Notes: JUCE 1.x to 8.0

This document records the API changes encountered when porting Pedalboard3
from JUCE 1.x (used by the original Pedalboard2) to JUCE 8.0.15. Each
section describes the old API, the new API, and how the codebase was
adapted.

## Summary of API Changes

The migration from JUCE 1.x to JUCE 8.0.15 involved numerous breaking
changes across the framework. The most significant categories are:

- Memory ownership: `ScopedPointer` replaced by `std::unique_ptr`
- Font API: direct float constructor replaced by `FontOptions`
- Audio graph API: `Node::Ptr` and connection enumeration changes
- PlayHead: `CurrentPositionInfo` replaced by `Optional<PositionInfo>`
- Plugin parameter access: deprecated `getParameter()`/`setParameter()`
  replaced by `AudioProcessorParameter` array access
- Editor creation: `createEditor()` made private, replaced by
  `createEditorAndMakeActive()`
- OSC: custom library replaced by `juce_osc` module
- Various signature changes in list models, command info, and system tray

## ScopedPointer to unique_ptr

JUCE 8 removed the `ScopedPointer` template class. All uses of
`ScopedPointer<T>` have been replaced with `std::unique_ptr<T>`.

### Before (JUCE 1.x)

```cpp
ScopedPointer<AudioPluginInstance> plugin;
ScopedPointer<Component> controls;
```

### After (JUCE 8)

```cpp
std::unique_ptr<AudioPluginInstance> plugin;
std::unique_ptr<Component> controls;
```

This change affects nearly every source file. The `JUCE_STRICT_REFCOUNTEDPOINTER`
compile definition is enabled to catch any remaining `ReferenceCountedObjectPtr`
misuse at compile time.

## Font API

JUCE 8 replaced the direct `Font(float height)` constructor with
`Font(FontOptions)`. The `Font::getStringWidth` method was also removed.

### Font Construction

#### Before (JUCE 1.x)

```cpp
Font smallFont(15.0f);
Font bigFont(48.0f, Font::bold);
```

#### After (JUCE 8)

```cpp
Font smallFont(FontOptions().withHeight(15.0f));
Font bigFont(FontOptions(48.0f, Font::bold));
```

### getStringWidth

`Font::getStringWidth` was removed in JUCE 8. Use
`juce::GlyphArrangement::getStringWidthInt` instead.

#### Before (JUCE 1.x)

```cpp
int width = font.getStringWidth(text);
```

#### After (JUCE 8)

```cpp
GlyphArrangement ga;
ga.addLineOfText(font, text, 0, 0);
int width = ga.getStringWidthInt();
```

The codebase uses `juce::GlyphArrangement` for text measurement in
`PluginComponent` (for sizing plugin name labels and channel names).

## Graphics Drawing Order

In JUCE 8, the order of `setColour`, `setFont`, and `drawText` calls
matters. `setColour` must be called before `setFont`, which must be called
before `drawText`.

### Correct Order (JUCE 8)

```cpp
g.setColour(textColour);
g.setFont(font);
g.drawText(text, bounds, justification);
```

This pattern is followed throughout the codebase (e.g., in
`MainPanel::ProgramChangeWarning::paint`).

## OSC Library Replacement

The original Pedalboard2 used a custom OSC library (`NiallsOSCLib`/
`NiallsSocketLib`). Pedalboard3 replaces this with JUCE's built-in
`juce_osc` module.

### Before (JUCE 1.x)

- Custom `OscInput` class handling raw UDP socket reception
- Custom `OSC::Message` struct with manual parsing
- `NiallsSocketLib` for cross-platform socket abstraction

### After (JUCE 8)

- `juce::OSCReceiver` for receiving OSC messages (with
  `MessageLoopCallback` for message-thread delivery)
- `juce::OSCMessage` and `juce::OSCBundle` for message representation
- `juce::OSCAddress` and `juce::OSCArgument` for address matching and
  value extraction

The `OscMappingManager::messageReceived()` method now takes a
`const juce::OSCMessage&` instead of the custom `OSC::Message*`. The
manager extracts float, int, and MIDI arguments from `juce::OSCArgument`
objects.

The `OscInput` class remains as a dummy `AudioPluginInstance` for visual
representation on the canvas, but performs no actual OSC reception.

The `juce_osc` module is linked in CMakeLists.txt alongside other JUCE
modules. On Windows, `ws2_32` is linked for Winsock support.

## AudioProcessorGraph API Changes

### Node::Ptr

`AudioProcessorGraph::Node::Ptr` is a `ReferenceCountedObjectPtr<Node>`.
In JUCE 8, node identification uses `AudioProcessorGraph::NodeID` (a
struct containing a `uid` field) instead of a plain `uint32`.

#### Before (JUCE 1.x)

```cpp
uint32 nodeId = node->nodeID;
```

#### After (JUCE 8)

```cpp
AudioProcessorGraph::NodeID nodeId = node->nodeID;
// Access the uint32 via:
uint32 uid = node->nodeID.uid;
```

The `FilterGraph` API uses `AudioProcessorGraph::NodeID` throughout for
node identification in `addFilter`, `removeFilter`, `disconnectFilter`,
`setNodePosition`, `getNodePosition`, `connectionExists`, `canConnect`,
`addConnection`, and `removeConnection`.

### getConnections

JUCE 8 replaced the indexed connection enumeration with a single method
returning a `std::vector`.

#### Before (JUCE 1.x)

```cpp
int numConnections = graph.getNumConnections();
for (int i = 0; i < numConnections; ++i)
{
    auto* conn = graph.getConnection(i);
    // ...
}
```

#### After (JUCE 8)

```cpp
std::vector<AudioProcessorGraph::Connection> connections = graph.getConnections();
for (const auto& conn : connections)
{
    // ...
}
```

`FilterGraph::getConnections()` wraps this, returning
`std::vector<AudioProcessorGraph::Connection>`.

## AudioPlayHead PositionInfo API

JUCE 8 replaced the `AudioPlayHead::CurrentPositionInfo` struct with
`Optional<AudioPlayHead::PositionInfo>`.

### Before (JUCE 1.x)

```cpp
bool getCurrentPosition(CurrentPositionInfo& result) override
{
    result.bpm = tempo;
    result.isPlaying = playing;
    return true;
}
```

### After (JUCE 8)

```cpp
Optional<AudioPlayHead::PositionInfo> getPosition() const override
{
    PositionInfo info;
    info.setBpm(tempo);
    info.setIsPlaying(playing);
    return info;
}
```

`PluginField` implements `AudioPlayHead::getPosition()` returning
`juce::Optional<juce::AudioPlayHead::PositionInfo>`, providing tempo and
transport state to plugins in the graph.

## Drawable::createFromSVG Return Type Change

In JUCE 8.0.15, `Drawable::createFromSVG` takes an `XmlElement` reference
and returns `std::unique_ptr<Drawable>`. The `createFromSVG_string`
method is JUCE 9 only and must not be used.

### Correct Usage (JUCE 8.0.15)

```cpp
auto svgXml = XmlDocument::parse(svgText);
auto drawable = Drawable::createFromSVG(*svgXml);
```

The codebase loads SVG vector graphics from the `vectors/` directory using
this pattern. The `Vectors` and `LookAndFeelImages` classes handle SVG
loading.

## SystemTrayIconComponent API Change

In JUCE 8, `SystemTrayIconComponent` methods for setting the icon image
changed. The `setImage` method signature was updated.

The `TrayIcon` class (src/TrayIcon.h) inherits from
`SystemTrayIconComponent` and overrides `mouseDown` and `mouseDoubleClick`
for right-click popup menu and double-click window toggle respectively.

The class is conditionally compiled with `#ifndef JUCE_MAC` since the
system tray is not supported on macOS in this codebase.

## KnownPluginList::sort Signature Change

JUCE 8 changed the `KnownPluginList::sort` method signature. The sorting
parameters were updated to use an enum type.

The `SafePluginListComponent` handles plugin list sorting via
`sortOrderChanged()` which calls the updated `KnownPluginList::sort` with
the new parameter types.

## ApplicationCommandInfo::addDefaultKeypress Signature Change

JUCE 8 changed the `ApplicationCommandInfo::addDefaultKeypress` signature.
The key modifier parameter type was updated.

### Before (JUCE 1.x)

```cpp
commandInfo.addDefaultKeypress('p', ModifierKeys::commandModifier);
```

### After (JUCE 8)

The modifier parameter uses `juce::ModifierKeys` with updated flag
definitions. The codebase in `MainPanel::getCommandInfo()` uses the
updated signature for registering keyboard shortcuts.

## ListBoxModel::backgroundClicked Signature Change

JUCE 8 changed `ListBoxModel::backgroundClicked` to take a
`const MouseEvent&` parameter.

### Before (JUCE 1.x)

```cpp
void backgroundClicked(const MouseEvent& e) override
```

### After (JUCE 8)

```cpp
void backgroundClicked(const MouseEvent& e) override
```

The signature appears similar but the `MouseEvent` class itself was
updated in JUCE 8. The `PatchOrganiser` class implements this method for
deselecting rows when clicking empty space in the patch list.

## PluginDescription uid to uniqueId

JUCE 8 renamed the `PluginDescription::uid` field to `uniqueId` and
changed its type. The old `uid` field was a `int32`; the new `uniqueId`
uses a different representation.

### Before (JUCE 1.x)

```cpp
int32 uid = desc.uid;
```

### After (JUCE 8)

```cpp
// Use desc.uniqueId or desc.createIdentifierString()
String identifier = desc.createIdentifierString();
```

The `PluginPoolManager` uses `createPluginIdentifier()` to generate a
stable identifier string from `PluginDescription` fields, avoiding
direct dependency on the renamed field.

## File::nonexistent Deprecation

JUCE 8 deprecated `File::nonexistent` in favor of `File()` (default
constructor) or `File::getNonexistentSibling()`.

### Before (JUCE 1.x)

```cpp
File f = File::nonexistent;
if (f == File::nonexistent) { ... }
```

### After (JUCE 8)

```cpp
File f;  // Default-constructed File is "nonexistent"
if (!f.exists()) { ... }
```

## ComponentPeer::setBounds Signature Change

JUCE 8 changed `ComponentPeer::setBounds` to use `Rectangle<int>` instead
of separate x, y, width, height parameters.

### Before (JUCE 1.x)

```cpp
peer->setBounds(x, y, w, h);
```

### After (JUCE 8)

```cpp
peer->setBounds(Rectangle<int>(x, y, w, h));
```

## AudioProcessor::createEditor Made Private

In JUCE 8, `AudioProcessor::createEditor()` was made private. Use
`createEditorAndMakeActive()` instead, which creates the editor and makes
it active in one call.

### Before (JUCE 1.x)

```cpp
AudioProcessorEditor* editor = plugin->createEditor();
```

### After (JUCE 8)

```cpp
AudioProcessorEditor* editor = plugin->createEditorAndMakeActive();
```

`BypassableInstance::createEditor()` delegates to
`plugin->createEditorAndMakeActive()`.

## Plugin Parameter Access

JUCE 8 deprecated the `getParameter(int)`, `setParameter(int, float)`,
`getParameterText(int)`, and `getParameterName(int)` methods on
`AudioProcessor`. These are replaced by the `AudioProcessorParameter`
array accessed via `getParameters()`.

### Before (JUCE 1.x)

```cpp
int numParams = plugin->getNumParameters();
float value = plugin->getParameter(index);
plugin->setParameter(index, newValue);
String name = plugin->getParameterName(index);
String text = plugin->getParameterText(index);
```

### After (JUCE 8)

```cpp
auto& params = plugin->getParameters();
int numParams = params.size();
float value = params[index]->getValue();
params[index]->setValue(newValue);
String name = params[index]->getName(128);
String text = params[index]->getCurrentValueAsText();
```

`BypassableInstance` provides wrapper methods (`getNumPluginParameters`,
`getPluginParameter`, `getPluginParameterName`, `getPluginParameterValue`,
`getPluginParameterText`, `setPluginParameterValue`,
`isPluginParameterAutomatable`, `isPluginMetaParameter`) that use the
`AudioProcessorParameter` array.

The built-in processors (`LevelProcessor`, `FilePlayerProcessor`, etc.)
keep deprecated `getParameter()`/`setParameter()` as regular non-override
methods for internal use by their control components, since these
processors do not expose parameters through the JUCE parameter system.

## JUCE 9 Compatibility Warning

The codebase is pinned to JUCE 8.0.15 and is NOT compatible with JUCE 9.x.
Key incompatibilities:

- `Typeface::createSystemTypefaceFor` crashes on Windows with DirectWrite
  in JUCE 9.
- `Drawable::createFromSVG_string` is JUCE 9 only; the codebase uses
  `Drawable::createFromSVG(XmlElement)` which is the JUCE 8 API.
- Additional breaking API changes in JUCE 9 have not been addressed.

Do not update the JUCE submodule beyond tag 8.0.15.