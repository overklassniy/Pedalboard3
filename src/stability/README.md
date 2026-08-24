# src/stability/

Stability infrastructure for Pedalboard3 — crash protection, blacklisting,
settings persistence, and undo/redo.

## Contents

- `SettingsManager.cpp` / `SettingsManager.h` — thread-safe JSON-based settings persistence singleton (uses nlohmann_json)
- `PluginBlacklist.cpp` / `PluginBlacklist.h` — plugin blacklist manager (path and ID blacklisting, path normalization, persistence)
- `CrashProtection.cpp` / `CrashProtection.h` — defensive crash protection (SEH wrappers on Windows, auto-save, watchdog thread, timeout protection, crash context logging)
- `UndoActions.cpp` / `UndoActions.h` — undo/redo actions for graph operations (add/remove plugin, add/remove connection)
- `BlacklistWindow.cpp` / `BlacklistWindow.h` — window for managing the plugin blacklist (list, remove, clear)

## Integration

`SettingsManager` is the central settings store, used by `app/MainPanel`,
`preset/PreferencesDialog`, and other components for persistent configuration.
`PluginBlacklist` is used by `scanner/PluginScannerClient` and
`scanner/SafePluginScanner` to skip known-bad plugins. `CrashProtection`
wraps risky plugin operations (loading, scanning) to prevent crashes from
taking down the application. `UndoActions` are registered with the
`UndoManager` in `app/MainPanel` for graph edit history. `BlacklistWindow`
is opened from the Options menu.
