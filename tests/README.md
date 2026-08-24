# tests/

Unit tests for Pedalboard3, using Catch2 v3.

## Contents

- `CMakeLists.txt` — test target build configuration
- `main.cpp` — Catch2 test runner entry point
- `graph_test.cpp` — FilterGraph node and connection tests
- `midi_mapping_test.cpp` — MidiAppFifo and MIDI mapping tests
- `cable_test.cpp` — connection wiring tests
- `settings_test.cpp` — SettingsManager JSON persistence tests
- `scanner_test.cpp` — plugin blacklist and scanner tests

## Running tests

```bash
# Windows
.\build\tests\Release\Pedalboard3_Tests.exe

# Linux
./build/tests/Release/Pedalboard3_Tests
```

Run a specific tag:

```bash
.\build\tests\Release\Pedalboard3_Tests.exe [graph]
```
