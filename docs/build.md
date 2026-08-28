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

The project ships six CMake presets (defined in `CMakePresets.json`):

| Preset | Platform | Generator | Build type | Binary dir |
| --- | --- | --- | --- | --- |
| `windows-default` | Windows x64 | Visual Studio 17 2022 x64 | Release | `build/` |
| `windows-debug` | Windows x64 | Visual Studio 17 2022 x64 | Debug | `build-debug/` |
| `windows-arm64` | Windows arm64 | Visual Studio 17 2022 ARM64 | Release | `build-arm64/` |
| `linux-default` | Linux x64 | Ninja + gcc/g++ | Release | `build/` |
| `linux-debug` | Linux x64 | Ninja + gcc/g++ | Debug | `build-debug/` |
| `linux-arm64` | Linux arm64 | Ninja + aarch64-linux-gnu cross | Release | `build-arm64/` |

The `windows-arm64` preset requires the MSVC v143 ARM64 build tools
(`Microsoft.VisualStudio.Component.VC.Tools.ARM64`), included in the Visual
Studio 2022 "Desktop development with C++" workload. The `linux-arm64`
preset cross-compiles with `aarch64-linux-gnu-gcc` / `g++` and requires the
matching cross-compiler package plus the `:arm64` multiarch development
libraries listed in [Linux dependencies](#dependencies).

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
over a Windows named pipe (the IPC protocol is Windows-specific; on Linux
the scanner is built but is a stub). `PluginScannerClient` looks for the
scanner binary in the same directory as the main executable at runtime.
The root `CMakeLists.txt` includes a post-build command that copies
`Pedalboard3Scanner` next to the `Pedalboard3` executable, so both
binaries end up in `<build_dir>/Pedalboard3_artefacts/<config>/`. When
distributing a build, ship both binaries from that folder.

## Application version

The application version reported by `JUCEApplication::getApplicationVersion()`
(and shown in the About dialog's update check) is taken from the CMake
project version, which defaults to `3.0.0`. To build a specific release
version, pass the `Pedalboard3_VERSION_OVERRIDE` cache variable:

```bash
cmake --preset windows-default -DPedalboard3_VERSION_OVERRIDE=3.1.0
```

This is what the release workflow uses to stamp tagged builds with the tag
version.

## Release workflow

The [release workflow](../.github/workflows/release.yml) builds Pedalboard3
for Windows x64, Windows arm64, Linux x64, and Linux arm64, then attaches
the artifacts to a GitHub Release. It runs on tag pushes matching `v*` and
on manual dispatch. See
[`.github/workflows/README.md`](../.github/workflows/README.md) for the
full trigger, matrix, and dependency details.
