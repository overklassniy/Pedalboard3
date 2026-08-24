# cmake/

CMake helper modules for Pedalboard3.

## Contents

- `CPM.cmake` — CPM.cmake v0.40.0 dependency manager. Downloaded from
  https://github.com/cpm-cmake/CPM.cmake/releases. Used to fetch fmt,
  spdlog, nlohmann_json, and Catch2 at configure time.

## Integration

Included by the root `CMakeLists.txt` via `include(cmake/CPM.cmake)`.
