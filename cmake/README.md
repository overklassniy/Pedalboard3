# cmake/

CMake helper modules for the Pedalboard3 build. This folder exists to keep
third-party CMake infrastructure out of the root source tree while remaining
trivially includable from the root `CMakeLists.txt`.

## Contents

- `CPM.cmake` – CPM.cmake v0.40.0, the CMake package manager
  (https://github.com/cpm-cmake/CPM.cmake). It provides the `CPMAddPackage`
  command used at configure time to fetch the project's dependencies. The
  version is declared on the line `set(CURRENT_CPM_VERSION 0.40.0)` and the
  module requires `cmake_minimum_required(VERSION 3.14 FATAL_ERROR)`.

## Integration

The module is pulled into the build by the root `CMakeLists.txt` via
`include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)` (line 15), which makes
`CPMAddPackage` available for the rest of the configure step. The root
`CMakeLists.txt` then uses it to fetch the following dependencies:

- `fmt` – fmtlib/fmt, `GIT_TAG 12.2.0` (line 25).
- `spdlog` – gabime/spdlog, `GIT_TAG v1.17.0` (line 33).
- `nlohmann_json` – nlohmann/json, `GIT_TAG v3.12.0` (line 41).
- `Catch2` – catchorg/Catch2, `GIT_TAG v3.15.3` (line 52), only when
  `Pedalboard3_BUILD_TESTS` is `ON`.

`fmt`, `spdlog`, and `nlohmann_json` are linked into the main `Pedalboard3`
target; `Catch2` is consumed by `tests/CMakeLists.txt`.

## Constraints

CPM.cmake itself requires CMake 3.14 or newer, but the root `CMakeLists.txt`
sets `cmake_minimum_required(VERSION 3.24)`, so the effective minimum is 3.24.
The downloaded sources are cached under the CPM source directory at configure
time; no vendored copies live in this folder.
