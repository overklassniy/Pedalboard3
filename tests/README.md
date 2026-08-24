# tests/

Unit tests for the Pedalboard3 project, built with Catch2 v3. This folder
exists to keep test sources separate from the application sources while still
participating in the same CMake configure and CTest workflow.

## Contents

- `smoke_test.cpp` – basic smoke tests tagged `[smoke]`. Verifies that the
  project compiles, that `fmt` and `spdlog` are linked and usable, and that
  C++17 language features (structured bindings, `std::optional`,
  `if constexpr`) work. TEST_CASEs: "Build verification", "fmt library
  integration", "spdlog library integration", "C++17 features".
- `filtergraph_test.cpp` – unit tests for the FilterGraph / IFilterGraph
  interface, tagged `[filtergraph]`. Uses mock types that mirror the
  `IFilterGraph` interface so logic can be verified without JUCE audio
  initialization. TEST_CASEs: "FilterGraph Node Management" (`[nodes]`),
  "FilterGraph Connection Management" (`[connections]`), "FilterGraph Position
  Management" (`[position]`), "FilterGraph Infrastructure Detection"
  (`[infrastructure]`), "FilterGraph Mutation Testing" (`[mutation]`).
- `protection_test.cpp` – tests for plugin protection features
  (`PluginBlacklist`, `CrashProtection`), tagged `[protection]`. Uses mock
  implementations to avoid actual crashes and `SettingsManager` dependencies.
  TEST_CASEs: "PluginBlacklist - Path Management", "PluginBlacklist - Path
  Normalization", "PluginBlacklist - ID Management", "PluginBlacklist -
  Retrieval", "CrashProtection - Successful Operations", "CrashProtection -
  Exception Handling", "CrashProtection - Auto-Save Callback", "CrashProtection
  - Operation Context", "PluginBlacklist - Mutation Tests", "CrashProtection -
  Mutation Tests", "PluginBlacklist - Thread Safety", "CrashProtection -
  Thread Safety".
- `CMakeLists.txt` – build definition for the `Pedalboard3_Tests` executable.
  Adds the three test source files, links `Catch2::Catch2WithMain`, `fmt::fmt`,
  and `spdlog::spdlog`, defines `PEDALBOARD3_TESTS`, and registers every
  `TEST_CASE` with CTest via `catch_discover_tests(Pedalboard3_Tests)`.

## Integration

The `tests/` subdirectory is added by the root `CMakeLists.txt` inside the
`Pedalboard3_BUILD_TESTS` guard (lines 409-411):

```
if(Pedalboard3_BUILD_TESTS)
    add_subdirectory(tests)
endif()
```

The `Pedalboard3_BUILD_TESTS` option is declared on line 49 of the root
`CMakeLists.txt` and defaults to `ON`. When enabled, the root file also fetches
Catch2 v3.15.3 via CPM and appends its `extras` directory to
`CMAKE_MODULE_PATH` so that `include(Catch)` resolves in
`tests/CMakeLists.txt`.

The tests link against the same `fmt::fmt` and `spdlog::spdlog` targets as the
main application, confirming those dependencies are usable from an external
consumer. The test sources do not link JUCE directly; `filtergraph_test.cpp`
and `protection_test.cpp` exercise logic through mock types rather than the
real `src/audio/FilterGraph.h` and `src/stability/PluginBlacklist.h`
implementations.

## Constraints

Tests are only built when `Pedalboard3_BUILD_TESTS` is `ON` (the default);
disable it with `-DPedalboard3_BUILD_TESTS=OFF` to skip the subdirectory
entirely. Test discovery is handled by `catch_discover_tests`, so each
`TEST_CASE` is registered with CTest at build time. After building, run the
executable directly or via CTest:

```
.\build\tests\Release\Pedalboard3_Tests.exe
ctest --test-dir build --build-config Release
```

To run a single tag, for example the smoke tests:

```
.\build\tests\Release\Pedalboard3_Tests.exe [smoke]
```
