# ============================================================================
# Wire-contract test suite (docs/RESTRUCTURING.md Phase 1)
#
# Built only with -DDARKEDEN_BUILD_TESTS=ON (make test does this). Runs
# locally by design — CI is deliberately Phase 6.
# ============================================================================

include(FetchContent)

# googletest release-1.12.1 is the last line supporting C++11, which is what
# this codebase builds as.
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(wire_tests
    support/TestStreams.cpp
    packet_roundtrip_test.cpp
    wire_layout_test.cpp
)

target_include_directories(wire_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
    ${CMAKE_SOURCE_DIR}/src/Core
)

target_compile_definitions(wire_tests PRIVATE
    WIRETEST_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden"
    WIRETEST_LAYOUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/wire-layout.txt"
)

# TestPackets (defined in src/Core/CMakeLists.txt) is the whole packet set
# compiled with NO server-type macro: handlers collapse to no-ops, exactly
# like the client's build of its packet copies, leaving pure wire code.
# Core and TestPackets are mutually dependent static archives (Core's
# Datagram.cpp calls back into PacketFactoryManager), so each is listed
# twice for the single-pass linker.
target_link_libraries(wire_tests PRIVATE
    TestPackets
    Core
    TestPackets
    Core
    gtest_main
    Threads::Threads
)
if(XERCES_LIBRARY)
    target_link_libraries(wire_tests PRIVATE ${XERCES_LIBRARY})
endif()

add_test(NAME wire_tests COMMAND wire_tests)

add_test(
    NAME ratchets
    COMMAND bash ${CMAKE_SOURCE_DIR}/tests/ratchet/ratchets.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
