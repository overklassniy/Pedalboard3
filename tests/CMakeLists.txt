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
catch_discover_tests(Pedalboard3_Tests)