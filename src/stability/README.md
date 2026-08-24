# src/stability/

Stability infrastructure for Pedalboard3: crash protection, plugin blacklisting, JSON-based settings persistence, and undoable graph actions. This folder exists to keep the host alive when plugins misbehave, to persist user configuration and blacklist data across sessions, and to provide the undo/redo action types for filter graph editing.

## Contents

- `CrashProtection.cpp` / `CrashProtection.h` – `CrashProtection` singleton for defensive crash protection. Provides `executeWithProtection()` (SEH-wrapped on Windows, direct execution elsewhere), `executeWithTimeout()` (runs on a worker thread with a deadline), and `executeWithProtectionAndTimeout()` (combines both). Includes an auto-save callback hook, a watchdog thread that flags UI hangs when pings stop arriving, crash-context logging, and the `ScopedOperationContext` RAII helper and `PROTECTED_OPERATION` macro.
- `PluginBlacklist.cpp` / `PluginBlacklist.h` – `PluginBlacklist` singleton managing user-configurable plugin blacklisting by path and by identifier. Performs path normalisation for case-insensitive comparison on Windows and persists the blacklist through `SettingsManager`.
- `SettingsManager.cpp` / `SettingsManager.h` – `SettingsManager` singleton for thread-safe JSON settings persistence. Stores settings in an `nlohmann::json` object guarded by a mutex and auto-saves to disk after each write. Provides typed getters and setters for strings, booleans, integers, doubles, string arrays, and `XmlElement` values.
- `UndoActions.cpp` / `UndoActions.h` – `juce::UndoableAction` subclasses for filter graph editing. Defines `FilterGraphAction` (base, holds a `FilterGraph&` reference), `AddPluginAction`, `RemovePluginAction`, `AddConnectionAction`, and `RemoveConnectionAction`, each with `perform()` and `undo()` implementations.
- `BlacklistWindow.cpp` / `BlacklistWindow.h` – UI for managing the plugin blacklist. Defines `BlacklistListModel` (ListBoxModel), `BlacklistComponent` (Component with list, remove, clear, and close buttons), and `BlacklistWindow` (DocumentWindow shown as a singleton via `showWindow()`).

## Integration

`CrashProtection` is used by `scanner/SafePluginScanner`, which calls `executeWithProtectionAndTimeout()` to wrap in-process plugin scanning with SEH and timeout protection. `PluginBlacklist` is used by `scanner/PluginScannerClient` (auto-blacklists plugins on scanner crash) and `scanner/SafePluginScanner` (skips blacklisted files during scanning); it persists its data through `SettingsManager`. `SettingsManager` is currently used only by `PluginBlacklist` within this folder; the main application's settings (OSC port, I/O toggles, window options) are handled separately through `util/PropertiesSingleton`. `UndoActions` operate on `audio/FilterGraph` and are designed for registration with a `juce::UndoManager`; they are compiled into the main target but are not yet wired into `app/MainPanel`. `BlacklistWindow` is self-contained and shown via its static `showWindow()` method; it is not currently invoked from the application's Options menu.

## Constraints

`CrashProtection`'s SEH wrapping (`__try`/`__except` with an exception filter logging access violations, stack overflows, and other hardware exceptions) is active only on Windows (`#ifdef _WIN32`); on other platforms operations run directly without hardware-exception catching. `PluginBlacklist` normalises paths for case-insensitive comparison on Windows. `SettingsManager` requires the `nlohmann_json` library and writes to the user application data directory.
