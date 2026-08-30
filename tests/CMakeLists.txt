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

# googletest declares its targets in the default `all`, so a stale
# DARKEDEN_BUILD_TESTS=ON cache would drag them into a production build.
# FetchContent's own EXCLUDE_FROM_ALL option needs CMake 3.28; this project
# requires 3.16, so set the property directly.
foreach(gtest_target gtest gtest_main)
    if(TARGET ${gtest_target})
        set_target_properties(${gtest_target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endforeach()

# EXCLUDE_FROM_ALL: even if a stale cache leaves DARKEDEN_BUILD_TESTS=ON,
# a plain `cmake --build build` must not drag the 948-file TestPackets
# library and googletest into the production build. `make test` names the
# target explicitly.
add_executable(wire_tests EXCLUDE_FROM_ALL
    support/TestStreams.cpp
    packet_roundtrip_test.cpp
    packet_encrypter_test.cpp
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
# compiled with NO server-type macro: handlers collapse to no-ops, leaving
# pure wire code. See the note there — this is not the client's own
# configuration (the client defines __GAME_CLIENT__=1).
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
# A hang here would otherwise sit for ctest's 1500s default with no output.
set_tests_properties(wire_tests PROPERTIES TIMEOUT 120)

add_test(
    NAME ratchets
    COMMAND bash ${CMAKE_SOURCE_DIR}/tests/ratchet/ratchets.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
