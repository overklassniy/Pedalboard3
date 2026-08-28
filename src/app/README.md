# src/app/

Application shell for Pedalboard3. Contains the JUCE application entry
point, the main top-level UI component that coordinates the audio engine
and all sub-systems, and auxiliary UI components for the about dialog,
log display, and system tray icon.

## Contents

- `App.cpp` / `App.h` – `Pedalboard3App`, the JUCE `JUCEApplication` subclass that bootstraps the main window and tears down audio singletons at shutdown
- `MainPanel.cpp` / `MainPanel.h` – `MainPanel`, the central UI component and `FileBasedDocument` that owns the `FilterGraph`, `AudioDeviceManager`, `KnownPluginList`, `OSCReceiver`, patch list, transport controls, menu bar, and command dispatch; also defines `PluginListWindow`
- `AboutPage.cpp` / `AboutPage.h` – `AboutPage`, a dialog component showing application version, credits, hyperlinks, the current local IP address, and a live update-status label that checks GitHub for newer releases
- `LogDisplay.cpp` / `LogDisplay.h` – `LogDisplay`, a component with a read-only text editor, start/stop logging button, and event-type filter toggles that displays events from the `LogFile` singleton
- `TrayIcon.cpp` / `TrayIcon.h` – `TrayIcon`, a `SystemTrayIconComponent` with a right-click popup menu and double-click window toggle (compiled only when `JUCE_MAC` is not defined)

## Integration

`Pedalboard3App` (in `App.h`) creates the `DocumentWindow` and optional
`TrayIcon` at startup. The main window's content component is `MainPanel`,
which is the central coordinator for the entire application.

`MainPanel` directly includes headers from `audio/` (`AudioSingletons.h`,
`FilterGraph.h`, `MainTransport.h`, `MidiAppFifo.h`), `canvas/`
(`PluginField.h`), `lookandfeel/` (`ColourScheme.h`, `ColourSchemeEditor.h`),
`mappings/` (`ApplicationMappingsEditor.h`, `TapTempoBox.h`), `preset/`
(`PatchOrganiser.h`, `PreferencesDialog.h`, `UserPresetWindow.h`),
`processors/` (`PedalboardProcessors.h`), and `util/` (`Images.h`,
`JuceHelperStuff.h`, `LogFile.h`, `PropertiesSingleton.h`, `Vectors.h`). It
also includes the generated `HelpData.h` header produced by
`juce_add_binary_data` from `assets/help/help.html`.

`MainPanel` accesses the `MidiMappingManager` and `OscMappingManager`
through `PluginField` (in `canvas/`) rather than including their headers
directly. It does not directly include headers from `osc/` or `stability/`,
but interacts with those sub-systems transitively through the command
system and `PluginField`.

All files in this directory are compiled into the `Pedalboard3` GUI
application target.

## Constraints

- `TrayIcon.h` and `TrayIcon.cpp` are guarded by `#ifndef JUCE_MAC` — the system tray icon is not built on macOS.
- `MainPanel` inherits `juce::FileBasedDocument` and uses `.pdl` as the patch document extension; individual graph saves use `.filtergraph` (defined in `audio/FilterGraph.h`).
- The `HelpDocumentation` command (F1, Help -> Documentation) writes the embedded help HTML (`HelpData::help_html`, embedded via `juce_add_binary_data` from `assets/help/help.html`) to the application data folder and launches it in the default browser. No on-disk `documentation/` folder next to the executable is required; the help works regardless of where the `.exe` is located, on all platforms.
- `AboutPage` performs an automatic version-actuality check when opened. A detached background thread queries the GitHub releases API (`https://api.github.com/repos/overklassniy/Pedalboard3/releases/latest`) with a 5-second timeout, parses the `tag_name` field from the JSON response, and compares it numerically against the running version returned by `getApplicationVersion()`. The result is posted back to the message thread via `MessageManager::callAsync` using a `Component::SafePointer` so the callback is a no-op if the dialog was already closed. The status is shown in a label at the same position as the initial "Checking for updates..." text. Possible statuses: "Up to date (vX.Y.Z)" (green), "Update available: vX.Y.Z (click to download)" (orange, shown as a clickable hyperlink to the GitHub releases page), "Newer than latest release (vX.Y.Z)" (orange), "No releases published yet" (grey, HTTP 404), or "Update check failed" (grey, network error). The hyperlink is only shown when an update is available; in all other cases the status is a plain label.
