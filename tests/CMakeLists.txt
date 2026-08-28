# Pedalboard3 Tests
add_executable(Pedalboard3_Tests
    smoke_test.cpp
    filtergraph_test.cpp
    protection_test.cpp
)

target_link_libraries(Pedalboard3_Tests PRIVATE
    Catch2::Catch2WithMain
    fmt::fmt
    spdlog::spdlog
)

target_compile_definitions(Pedalboard3_Tests PRIVATE
    PEDALBOARD3_TESTS
)

include(CTest)
include(Catch)
# catch_discover_tests runs the test executable on the build host to
# enumerate tests. This fails when the target architecture differs from
# the host architecture:
#   - Linux cross-compile (CMAKE_CROSSCOMPILING is true)
#   - Windows Visual Studio targeting ARM64 on an x64 host
#     (CMAKE_CROSSCOMPILING is false because VS is a multi-platform
#     generator, but the ARM64 binary cannot execute on x64)
# Skip test discovery in both cases; tests can still be run manually on
# the target system.
set(_skip_test_discovery FALSE)
if(CMAKE_CROSSCOMPILING)
    set(_skip_test_discovery TRUE)
endif()
if(DEFINED CMAKE_VS_PLATFORM_NAME AND NOT CMAKE_VS_PLATFORM_NAME STREQUAL CMAKE_HOST_SYSTEM_PROCESSOR)
    set(_skip_test_discovery TRUE)
endif()
if(NOT _skip_test_discovery)
    catch_discover_tests(Pedalboard3_Tests)
endif()
unset(_skip_test_discovery)