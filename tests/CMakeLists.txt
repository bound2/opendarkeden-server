# =====================================================================# Wire-contract test suite (docs/RESTRUCTURING.md Phase 1)
#
# Built only with -DDARKEDEN_BUILD_TESTS=ON (make test does this). Runs
# locally by design — CI is deliberately Phase 6.
# =====================================================================
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
# a plain `cmake --build build` must not drag the TestPackets library and
# googletest into the production build. `make test` names the target
# explicitly.
add_executable(wire_tests EXCLUDE_FROM_ALL
    support/TestStreams.cpp
    outcome_test.cpp
    packet_roundtrip_test.cpp
    packet_encrypter_test.cpp
    packet_exchange_test.cpp
    wire_layout_test.cpp
    xml_parse_test.cpp
)

target_include_directories(wire_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
    ${CMAKE_SOURCE_DIR}/src/Core
)

target_compile_definitions(wire_tests PRIVATE
    WIRETEST_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden"
    WIRETEST_LAYOUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/wire-layout.txt"
    WIRETEST_XML_PARSE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/golden/xml-parse.txt"
    WIRETEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data"
)

# TestPackets (defined in src/Core/CMakeLists.txt) is the define-free
# factory/validator set; the wire classes themselves come from de-kernel,
# which it links — the same macro-free compile the deployed servers link
# (task 2.4). See the note there — this is not the client's own
# configuration (the client defines __GAME_CLIENT__=1).
# The dependency between these archives is one-way since the kernel flip:
# Core's DatagramFactoryRead.cpp calls into PacketFactoryManager
# (TestPackets); TestPackets needs nothing from Core. The double listing
# is kept as insurance against the cycle ever coming back; de-kernel
# follows transitively, last.
target_link_libraries(wire_tests PRIVATE
    TestPackets
    Core
    TestPackets
    Core
    gtest_main
    Threads::Threads
)

add_test(NAME wire_tests COMMAND wire_tests)
# A hang here would otherwise sit for ctest's 1500s default with no output.
set_tests_properties(wire_tests PROPERTIES TIMEOUT 120)

# Repository/domain unit tests (docs/RESTRUCTURING.md 3.2): pure rules and
# fake-backed repository contracts. Header-only against the sources — no
# game or database libraries linked, which is the point of the seam.
add_executable(repository_tests EXCLUDE_FROM_ALL
    repository_test.cpp
)
target_include_directories(repository_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server/gameserver
)
target_link_libraries(repository_tests PRIVATE
    gtest_main
    Threads::Threads
)
# devbuild's test path builds the wire_tests target then runs all of ctest;
# hang this off it so the suite is always built.
add_dependencies(wire_tests repository_tests)

add_test(NAME repository_tests COMMAND repository_tests)
set_tests_properties(repository_tests PROPERTIES TIMEOUT 60)

# Pure-formula unit tests (docs/RESTRUCTURING.md 3.3). Linking ONLY de-core
# and gtest is deliberate: it proves de-core stays freestanding — pulling
# Core, transport, or a gameserver header into it breaks this link.
add_executable(formula_tests EXCLUDE_FROM_ALL
    formula_test.cpp
)
target_link_libraries(formula_tests PRIVATE
    de-core
    gtest_main
    Threads::Threads
)
# `make dev-test` builds the wire_tests target; ride along so ctest always
# has a fresh binary (same pattern as repository_tests above).
add_dependencies(wire_tests formula_tests)

add_test(NAME formula_tests COMMAND formula_tests)
set_tests_properties(formula_tests PROPERTIES TIMEOUT 60)

add_test(
    NAME ratchets
    COMMAND bash ${CMAKE_SOURCE_DIR}/tests/ratchet/ratchets.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Include-graph architecture test (docs/RESTRUCTURING.md 2.2): layering
# rules + shrink-only baseline in tests/arch/. Perl, not Python — the
# dev image ships perl and the repo's generators already use it.
add_test(
    NAME arch_includes
    COMMAND perl ${CMAKE_SOURCE_DIR}/tests/arch/check_includes.pl
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
