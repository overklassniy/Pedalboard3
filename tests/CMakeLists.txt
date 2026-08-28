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
# When cross-compiling, catch_discover_tests tries to run the test
# executable on the build host, which fails because the binary targets a
# foreign architecture. Skip test discovery in that case; tests can still
# be run manually on the target system.
if(NOT CMAKE_CROSSCOMPILING)
    catch_discover_tests(Pedalboard3_Tests)
endif()