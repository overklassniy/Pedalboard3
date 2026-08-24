# Pedalboard3 Build Instructions

This document provides detailed build instructions for Pedalboard3 on
Windows and Linux, including prerequisites, step-by-step procedures, CMake
options, presets, build artifact locations, and troubleshooting.

## Prerequisites

### Windows

| Requirement | Details |
| --- | --- |
| Visual Studio 2022 | BuildTools or full IDE with the C++ desktop development workload |
| CMake 3.24+ | VS 2022 bundles CMake 3.31 at a known path (see below) |
| Git | For cloning the repository and submodules |

The VS 2022 bundled CMake is located at:

```
C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
```

Add this path to your `PATH` environment variable, or use the full path in
commands.

### Linux

| Requirement | Details |
| --- | --- |
| GCC 11+ or Clang 12+ | C++20 compiler support required |
| CMake 3.24+ | Build system |
| Ninja | Recommended build generator (fast incremental builds) |
| Git | For cloning the repository and submodules |
| ALSA dev headers | `libasound2-dev` (Debian/Ubuntu) or equivalent |
| JACK dev headers | `libjack-dev` (Debian/Ubuntu) or equivalent; optional but recommended |

Install Linux prerequisites (Debian/Ubuntu):

```bash
sudo apt-get install build-essential cmake ninja-build git \
    libasound2-dev libjack-dev
```

## JUCE Submodule Setup

JUCE is pinned to tag 8.0.15 as a git submodule. It must be present before
configuring the build.

After cloning the repository:

```bash
git submodule update --init --recursive
```

If the submodule is missing or corrupted, clone it manually:

```bash
git clone --depth 1 --branch 8.0.15 https://github.com/juce-framework/JUCE.git JUCE
```

Do NOT update JUCE to 9.x. JUCE 9 has breaking API changes that are
incompatible with this codebase.

## CPM Dependency Fetching

Third-party dependencies (fmt, spdlog, nlohmann_json, Catch2) are fetched
automatically by CPM.cmake during the CMake configure step. No manual
download is required. An internet connection is needed on first configure.

Fetched dependencies:

| Dependency | Version | Purpose |
| --- | --- | --- |
| fmt | 12.2.0 | String formatting |
| spdlog | v1.17.0 | Logging |
| nlohmann_json | v3.12.0 | JSON settings persistence |
| Catch2 | v3.15.3 | Unit tests (optional, controlled by Pedalboard3_BUILD_TESTS) |

Dependencies are cached in the build directory under `_deps/` and
`CPM_modules/` after the first configure.

## Build Instructions

### Windows (Visual Studio 2022)

#### Using CMake Presets

```powershell
git clone --recursive <repo-url>
cd Pedalboard3

# Configure (Release)
cmake --preset windows-default

# Build
cmake --build build --config Release -- /m
```

For a debug build:

```powershell
cmake --preset windows-debug
cmake --build build-debug --config Debug
```

#### Using Raw CMake Commands

If you prefer not to use presets:

```powershell
# Add VS 2022 bundled CMake to PATH (if needed)
$env:Path = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;" + $env:Path

# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# Build (parallel)
cmake --build build --config Release -- /m
```

### Linux (Ninja)

#### Using CMake Presets

```bash
git clone --recursive <repo-url>
cd Pedalboard3

# Configure (Release)
cmake --preset linux-default

# Build
cmake --build build
```

For a debug build:

```bash
cmake --preset linux-debug
cmake --build build-debug
```

#### Using Raw CMake Commands

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## CMake Options

| Option | Default | Description |
| --- | --- | --- |
| `Pedalboard3_BUILD_TESTS` | `ON` | Build the Catch2 unit tests |

To disable tests:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DPedalboard3_BUILD_TESTS=OFF
```

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DPedalboard3_BUILD_TESTS=OFF
```

## CMake Presets

Presets are defined in `CMakePresets.json`.

### Configure Presets

| Preset | Display Name | Generator | Binary Dir | Build Type |
| --- | --- | --- | --- | --- |
| `windows-default` | Windows (Visual Studio 2022 x64) | Visual Studio 17 2022 | `build` | Release |
| `windows-debug` | Windows (Visual Studio 2022 x64 Debug) | Visual Studio 17 2022 | `build-debug` | Debug |
| `linux-default` | Linux (Ninja Release) | Ninja | `build` | Release |
| `linux-debug` | Linux (Ninja Debug) | Ninja | `build-debug` | Debug |

Linux presets set `CMAKE_C_COMPILER` to `gcc` and `CMAKE_CXX_COMPILER` to
`g++`.

### Build Presets

| Preset | Configure Preset | Configuration |
| --- | --- | --- |
| `windows-default` | `windows-default` | Release |
| `windows-debug` | `windows-debug` | Debug |
| `linux-default` | `linux-default` | (uses CMAKE_BUILD_TYPE) |
| `linux-debug` | `linux-debug` | (uses CMAKE_BUILD_TYPE) |

Usage:

```powershell
cmake --preset windows-default
cmake --build --preset windows-default
```

```bash
cmake --preset linux-default
cmake --build --preset linux-default
```

## Build Artifacts

### Windows

| Artifact | Path |
| --- | --- |
| Main application | `build/Pedalboard3_artefacts/Release/Pedalboard3.exe` |
| Plugin scanner | `build/Pedalboard3Scanner_artefacts/Release/Pedalboard3Scanner.exe` |
| Debug application | `build-debug/Pedalboard3_artefacts/Debug/Pedalboard3.exe` |
| Debug scanner | `build-debug/Pedalboard3Scanner_artefacts/Debug/Pedalboard3Scanner.exe` |

### Linux

| Artifact | Path |
| --- | --- |
| Main application | `build/Pedalboard3_artefacts/Release/Pedalboard3` |
| Plugin scanner | `build/Pedalboard3Scanner_artefacts/Release/Pedalboard3Scanner` |
| Debug application | `build-debug/Pedalboard3_artefacts/Debug/Pedalboard3` |
| Debug scanner | `build-debug/Pedalboard3Scanner_artefacts/Debug/Pedalboard3Scanner` |

The scanner executable must be in the same directory as the main
application (or a discoverable path) for the out-of-process scanning to
work. The build system places both in the artefacts directory.

## Running Tests

If `Pedalboard3_BUILD_TESTS` is enabled, tests are built with the main
project. Run them with CTest from the build directory:

```powershell
cd build
ctest --output-on-failure
```

```bash
cd build
ctest --output-on-failure
```

Test source files are in the `tests/` directory and use the Catch2
framework.

## Troubleshooting

### JUCE Submodule Not Found

**Error**: `JUCE submodule not found. Run: git submodule update --init --recursive`

**Solution**:

```bash
git submodule update --init --recursive
```

If that fails, clone manually:

```bash
git clone --depth 1 --branch 8.0.15 https://github.com/juce-framework/JUCE.git JUCE
```

### CPM Dependencies Fail to Download

**Cause**: No internet connection during configure, or CPM cache corruption.

**Solution**:

1. Ensure internet access is available during the first configure.
2. If the cache is corrupted, delete the build directory and reconfigure:
   ```powershell
   Remove-Item -Recurse -Force build
   cmake --preset windows-default
   ```
   ```bash
   rm -rf build
   cmake --preset linux-default
   ```

### CMake Version Too Old

**Error**: `cmake_minimum_required ... VERSION 3.24`

**Solution**: Install CMake 3.24 or later. On Windows, use the VS 2022
bundled CMake (3.31). On Linux, download from cmake.org or use a PPA:

```bash
sudo apt-get install cmake
# Or for a newer version:
pip install cmake
```

### Compiler Does Not Support C++20

**Error**: Compilation errors related to C++20 features (concepts,
ranges, designated initializers, etc.)

**Solution**:

- Windows: Ensure VS 2022 with the latest C++ toolset is installed.
- Linux: Ensure GCC 11+ or Clang 12+ is the default compiler. Check with:
  ```bash
  gcc --version
  g++ --version
  ```
  If an older version is default, specify the compiler explicitly:
  ```bash
  cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11
  ```

### Missing ALSA or JACK Headers (Linux)

**Error**: `ALSA not found` or `JACK not found` during configure.

**Solution**:

```bash
sudo apt-get install libasound2-dev libjack-dev
```

### Build Fails with Linker Errors on Windows

**Cause**: Stale build cache or mismatched configurations.

**Solution**: Clean the build directory and rebuild:

```powershell
Remove-Item -Recurse -Force build
cmake --preset windows-default
cmake --build build --config Release -- /m
```

### Scanner Executable Not Found at Runtime

**Cause**: The scanner executable is not in the expected location relative
to the main application.

**Solution**: Ensure both `Pedalboard3.exe` and `Pedalboard3Scanner.exe`
are in the same directory. The build system places both in the artefacts
directory by default. If you move the main executable, move the scanner
as well.

### Warning Treated as Error (MSVC)

The project uses `/W4 /WX-` (high warnings, but warnings are not treated
as errors). Common JUCE-related warnings (C4100, C4244, C4267) are
suppressed with `/wd` flags. If you encounter new warnings treated as
errors, check that you are using the project's CMake configuration and
not overriding the warning flags.

### Slow Builds

**Solutions**:

- Use the `/m` (multiprocessor) flag on Windows for parallel compilation.
- Use Ninja on Linux for fast incremental builds.
- Ensure you are building in Release mode (Debug builds include additional
  checks and no optimizations).
- Consider disabling tests if not needed: `-DPedalboard3_BUILD_TESTS=OFF`.