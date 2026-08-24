# scanner/

Out-of-process plugin scanner for Pedalboard3.

## Contents

- `PluginScannerMain.cpp` — entry point for the `Pedalboard3Scanner` console
  application. Runs in a separate process so that a crashing plugin during
  scanning cannot take down the main application.

## Integration

The scanner communicates with the main application via IPC
(`PluginScannerIPC.h` in the parent `src/` directory). The main app launches
the scanner as a child process, sends scan requests, and receives plugin
descriptions or crash notifications back.

The scanner binary is copied next to `Pedalboard3.exe` at build time via a
CMake post-build command.
