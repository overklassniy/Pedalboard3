# tests

Unit tests for the Pedalboard3 project, built with Catch2 v3.

## Contents

- `smoke_test.cpp` - Basic smoke tests verifying that the project builds
  correctly and that core libraries (fmt, spdlog) are linked properly.
  Also validates C++17 language features.
- `filtergraph_test.cpp` - Unit tests for FilterGraph / IFilterGraph
  interface operations, covering node management, connection management,
  position management, infrastructure node detection, and mutation
  testing. Uses mock objects to verify logic without JUCE audio
  initialization.
- `protection_test.cpp` - Tests for plugin protection features
  (PluginBlacklist, CrashProtection). Verifies path and ID blacklisting,
  path normalization, crash protection callback invocation, context
  tracking, exception handling, mutation testing, and thread safety.
  Uses mock implementations to avoid actual crashes.
- `CMakeLists.txt` - CMake build definition for the test executable
  `Pedalboard3_Tests`.

## Build

Tests are built when the CMake option `Pedalboard3_BUILD_TESTS` is `ON`
(the default). The test executable is linked against Catch2, fmt, and
spdlog.

## Running

After building, run the test executable directly:

```
.\build\tests\Release\Pedalboard3_Tests.exe
```

To run a specific tag (e.g. smoke tests only):

```
.\build\tests\Release\Pedalboard3_Tests.exe [smoke]
```

## Integration

The `tests/` subdirectory is added by the root `CMakeLists.txt` inside
the `Pedalboard3_BUILD_TESTS` guard. Catch2 test discovery is handled
via `catch_discover_tests`, which registers each `TEST_CASE` with CTest.