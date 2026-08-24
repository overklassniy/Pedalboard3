# src/app/

Application shell for Pedalboard3.

## Contents

- `App.cpp` / `App.h` — JUCE application entry point and main window
- `MainPanel.cpp` / `MainPanel.h` — main UI container (menu bar, patch bar, transport, canvas viewport, file-based document handling, command manager)
- `AboutPage.cpp` / `AboutPage.h` — about dialog component
- `LogDisplay.cpp` / `LogDisplay.h` — log message display component
- `TrayIcon.cpp` / `TrayIcon.h` — system tray icon with popup menu (Windows/Linux only)

## Integration

`MainPanel` is the central coordinator that owns the `FilterGraph`, mapping
managers, patch organiser, and all command handling. It is created by `App`
and fills the main window. `MainPanel` includes headers from `audio/`,
`canvas/`, `mappings/`, `osc/`, `preset/`, `stability/`, `processors/`,
`util/`, and `lookandfeel/`.
