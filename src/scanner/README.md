# src/scanner/

Out-of-process plugin scanner for Pedalboard3.

## Contents

- `PluginScannerMain.cpp` — entry point for the `Pedalboard3Scanner` console application. Runs in a separate process so that a crashing plugin during scanning cannot take down the main application.
- `PluginScannerIPC.h` — IPC protocol definitions (message types, scan result codes, shared data structures)
- `PluginScannerClient.cpp` / `PluginScannerClient.h` — client-side IPC wrapper used by the main application to communicate with the scanner process
- `SafePluginScanner.cpp` / `SafePluginScanner.h` — safe plugin scanner wrapper with crash recovery and plugin list component UI
- `PluginPoolManager.cpp` / `PluginPoolManager.h` — plugin instance pool manager (reuses plugin instances to avoid repeated loading)

## Integration

The scanner communicates with the main application via IPC
(`PluginScannerIPC.h`). The main app (`app/MainPanel`) launches the scanner
as a child process via `PluginScannerClient`, sends scan requests, and
receives plugin descriptions or crash notifications back. `SafePluginScanner`
provides a higher-level wrapper with automatic retry and blacklist integration
(via `stability/PluginBlacklist`). `PluginPoolManager` manages a pool of
loaded plugin instances for efficient plugin editing.

The scanner binary is copied next to `Pedalboard3.exe` at build time via a
CMake post-build command.
