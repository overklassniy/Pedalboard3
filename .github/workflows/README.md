# .github/workflows

GitHub Actions workflow definitions for Pedalboard3.

## Files

- `release.yml` – Release workflow that builds Pedalboard3 for all supported
  Windows and Linux architectures and publishes distributable artifacts.

## release.yml

Builds the application for four targets and attaches the artifacts to a
GitHub Release.

### Triggers

- `push` on tags matching `v*` (for example `v3.0.0`). A GitHub Release is
  created automatically with all artifacts attached.
- `workflow_dispatch` with two inputs:
  - `version` – the version string without the `v` prefix
    (for example `3.0.0`). Passed to CMake as
    `Pedalboard3_VERSION_OVERRIDE` so the About dialog's update check
    reports the release version.
  - `create_release` – when `true`, a GitHub Release is created for the
    manual build. When `false`, artifacts are only uploaded as workflow
    artifacts.

### Build matrix

| Job | Runner | Target | Toolchain | Preset | Artifact |
| --- | --- | --- | --- | --- | --- |
| `build-windows` | `windows-latest` | Windows x64 | Visual Studio 2022 x64 | `windows-default` | `Pedalboard3-<version>-windows-x64.zip` |
| `build-windows` | `windows-latest` | Windows arm64 | Visual Studio 2022 ARM64 | `windows-arm64` | `Pedalboard3-<version>-windows-arm64.zip` |
| `build-linux` | `ubuntu-latest` | Linux x64 | gcc + Ninja | `linux-default` | `Pedalboard3-<version>-linux-x64.tar.gz` |
| `build-linux` | `ubuntu-latest` | Linux arm64 | `aarch64-linux-gnu` cross + Ninja | `linux-arm64` | `Pedalboard3-<version>-linux-arm64.tar.gz` |

Each Windows zip contains `Pedalboard3.exe` and `Pedalboard3Scanner.exe`.
Each Linux tarball contains the `Pedalboard3` application and the
`Pedalboard3Scanner` helper. The scanner IPC is Windows-specific, so on
Linux the helper is a stub; it is still packaged for completeness.

### Release job

The `release` job runs only on tag pushes or when `create_release` is set
on manual dispatch. It downloads all build artifacts and creates a GitHub
Release using `softprops/action-gh-release@v2` with auto-generated release
notes.

### Dependencies

The JUCE 8.0.15 submodule is checked out recursively. CPM fetches fmt,
spdlog, nlohmann_json, and Catch2 during configure, so no system copies of
those libraries are required. JUCE 8 bundles the ASIO headers, so Windows
ASIO support builds without an external Steinberg ASIO SDK.

Linux system dependencies (installed by the workflow) follow the official
JUCE `docs/Linux Dependencies.md` list for the modules Pedalboard3 uses:
ALSA, JACK, LADSPA, freetype, fontconfig, X11 (with the composite, cursor,
ext, inerama, randr, render extensions), and Mesa OpenGL. `JUCE_USE_CURL=0`
and `JUCE_WEB_BROWSER=0`, so libcurl and WebKit are not required.

The arm64 Linux build adds the `arm64` architecture to apt, fetches
package indices from `ports.ubuntu.com`, installs the
`gcc-aarch64-linux-gnu` cross-compiler and the `:arm64` multiarch
development libraries, and points `PKG_CONFIG_PATH` /
`PKG_CONFIG_LIBDIR` at `/usr/lib/aarch64-linux-gnu/pkgconfig` so JUCE's
pkg-config lookups resolve the foreign-architecture dependencies.

## Integration points

- The CMake presets used by the workflow are defined in
  [`CMakePresets.json`](../../CMakePresets.json) at the repository root.
- The application version override is implemented in
  [`CMakeLists.txt`](../../CMakeLists.txt) via the
  `Pedalboard3_VERSION_OVERRIDE` cache variable.
- The build prerequisites and presets are documented in
  [`docs/build.md`](../../docs/build.md).
