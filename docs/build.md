# Build guide

How to build Pedalboard3 from source on Windows and Linux. The build is
driven by CMake with presets defined in `CMakePresets.json`, and depends
on the JUCE 8.0.15 submodule.

## Prerequisites

Common to all platforms:

- CMake 3.24 or newer
- Git (with submodule support)
- A C++20 compiler

### Windows

- Visual Studio 2022 with the "Desktop development with C++" workload
- The CMake presets use the "Visual Studio 17 2022" generator with the
  x64 architecture

### Linux

- GCC (the presets pin `gcc` / `g++`)
- Ninja
- ALSA and Jack development headers (the build defines `JUCE_ALSA=1` and
  `JUCE_JACK_AUDIO_DEVICES=1` on Linux)

## Get the source

Clone with the JUCE submodule. JUCE must be at tag 8.0.15; the root
`CMakeLists.txt` aborts configuration with a clear error if the
submodule is missing.

```bash
git clone --recursive https://github.com/overklassniy/Pedalboard3.git
cd Pedalboard3
```

If you already cloned without `--recursive`:

```bash
git submodule update --init --recursive
```

The JUCE submodule is pinned to tag 8.0.15. Do not update it to `master`
or 9.x; the JUCE 8 API has breaking changes that the codebase relies on
(see [development.md](development.md#juce-8-api-notes)).

## Configure and build

The project ships four CMake presets (defined in `CMakePresets.json`):

| Preset | Platform | Generator | Build type | Binary dir |
| --- | --- | --- | --- | --- |
| `windows-default` | Windows | Visual Studio 17 2022 x64 | Release | `build/` |
| `windows-debug` | Windows | Visual Studio 17 2022 x64 | Debug | `build-debug/` |
| `linux-default` | Linux | Ninja + gcc/g++ | Release | `build/` |
| `linux-debug` | Linux | Ninja + gcc/g++ | Debug | `build-debug/` |

### Windows (Release)

```bash
cmake --preset windows-default
cmake --build build --config Release
```

The application executable is produced under
`build/Pedalboard3_artefacts/Release/`. The scanner console app is
produced under `build/Pedalboard3Scanner_artefacts/Release/`.

### Windows (Debug)

```bash
cmake --preset windows-debug
cmake --build build-debug --config Debug
```

### Linux (Release)

```bash
cmake --preset linux-default
cmake --build build
```

### Linux (Debug)

```bash
cmake --preset linux-debug
cmake --build build-debug
```

## Tests

Tests are on by default (`Pedalboard3_BUILD_TESTS=ON`) and use Catch2
v3.15.3, fetched via CPM. To disable them:

```bash
cmake --preset windows-default -DPedalboard3_BUILD_TESTS=OFF
```

After building, run the tests:

```bash
ctest --test-dir build --build-config Release
```

Or run the test executable directly and filter by tag:

```bash
./build/tests/Release/Pedalboard3_Tests.exe [smoke]
```

See [development.md](development.md#tests) for the available test tags.

## Dependencies

The following libraries are fetched by CPM during configure (defined in
the root `CMakeLists.txt`):

| Library | Version | Purpose |
| --- | --- | --- |
| fmt | 12.2.0 | String formatting |
| spdlog | v1.17.0 | Logging (uses external fmt) |
| nlohmann_json | v3.12.0 | JSON settings persistence |
| Catch2 | v3.15.3 | Unit tests (only when `Pedalboard3_BUILD_TESTS=ON`) |
| JUCE | 8.0.15 | Audio framework (submodule, not CPM) |

No system installation of fmt, spdlog, nlohmann_json, or Catch2 is
required; CPM downloads them into the build tree.

## Scanner binary placement

The `Pedalboard3Scanner` console application communicates with the host
over a Windows named pipe. `PluginScannerClient::getScannerExecutable()`
looks for the scanner binary in the same directory as the main
executable at runtime. The root `CMakeLists.txt` does not include a
post-build copy command, so when distributing a build, place the scanner
binary alongside the host executable manually or through your packaging
step.
