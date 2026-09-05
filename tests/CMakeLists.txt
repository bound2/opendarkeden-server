# =====================================================================# Wire-contract test suite (docs/RESTRUCTURING.md Phase 1)
#
# Built only with -DDARKEDEN_BUILD_TESTS=ON (make test does this). Runs
# locally and in the pinned-Zig C++20 CI job.
# =====================================================================
include(FetchContent)

# Keep the test framework on a maintained release. The project was previously
# held at 1.12.1 solely by its C++11 language baseline.
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.zip
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
    packet_meta_test.cpp
    wire_layout_test.cpp
    wire_types_test.cpp
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

# MySQL-backed integration tier (docs/RESTRUCTURING.md 3.2): the REAL
# repository implementations against a throwaway MySQL 5.7 with the
# initdb/ schema — the authority the fakes above are corrected against.
# Deliberately NOT registered with ctest: it needs the database that
# tests/integration/mysql_test.sh provides (a handrolled container
# fixture); run `make integration-test` from the host. Compiles the MySQL
# impl sources directly and links the macro-free Database library — the
# repositories carry no server-type macros, so the objects match what the
# gameserver builds.
add_executable(mysql_repository_tests EXCLUDE_FROM_ALL
    integration/mysql_repository_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLBalanceInfoRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLBloodBibleSignRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLBulletinBoardRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLComebackEventRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLContentInfoRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLMessageRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLRegenZoneRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLZoneInfoRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLCharacterRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLGoldRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLGoodsRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLCoupleRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLFlagWarRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLMofusPointRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLFriendRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLSystemAvailabilityRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLGuildRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLItemObjectRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLItemRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLNicknameRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLPlayRecordRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLEffectSaveRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLFlagSetRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLGameInfoRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLQuestInfoRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLQuestItemRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLRankBonusRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLSMSAddressRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLSessionRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLSkillSaveRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLStashRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLWarInfoRepository.cpp
)
target_include_directories(mysql_repository_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/integration
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/gameserver
)
target_link_libraries(mysql_repository_tests PRIVATE
    Database
    ServerCore
    de-kernel
    ${MYSQL_LIBRARY}
    gtest
    Threads::Threads
)

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

# C++20 structured-concurrency contract. This stays isolated from the game
# libraries so lifecycle failures cannot be hidden by unrelated fixtures.
add_executable(cooperative_thread_tests EXCLUDE_FROM_ALL
    cooperative_thread_test.cpp
)
target_include_directories(cooperative_thread_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
)
target_link_libraries(cooperative_thread_tests PRIVATE
    gtest_main
    Threads::Threads
)
add_dependencies(wire_tests cooperative_thread_tests)

add_test(NAME cooperative_thread_tests COMMAND cooperative_thread_tests)
set_tests_properties(cooperative_thread_tests PROPERTIES TIMEOUT 10)

add_executable(managed_thread_tests EXCLUDE_FROM_ALL
    managed_thread_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/ThreadPool.cpp
)
target_link_libraries(managed_thread_tests PRIVATE ServerCore de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests managed_thread_tests)
add_test(NAME managed_thread_tests COMMAND managed_thread_tests)
# Headroom for the stop-aware-wait tests: a regression there sits out a
# five-second polling interval per case instead of waking immediately.
set_tests_properties(managed_thread_tests PROPERTIES TIMEOUT 30)

# War-system Schedule/Scheduler and the VSDateTime clock they run on —
# ported from the deleted gameserver/test and gameserver/testAlone cppunit
# suites. Links only de-kernel plus the two war/ sources under test; the
# heartbeat appends to Schedule.txt via filelog(), so run from the build
# tree rather than the source tree.
add_executable(scheduler_tests EXCLUDE_FROM_ALL
    scheduler_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/war/Schedule.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/war/Scheduler.cpp
)
target_include_directories(scheduler_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server/gameserver
)
target_link_libraries(scheduler_tests PRIVATE de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests scheduler_tests)
add_test(NAME scheduler_tests COMMAND scheduler_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(scheduler_tests PROPERTIES TIMEOUT 30)

# RAII critical sections (src/Core/Exception.h). Exception.h is a kernel
# header, so the test includes it directly; de-kernel is linked only for
# Throwable's out-of-line dependencies (StringStream).
add_executable(critical_section_tests EXCLUDE_FROM_ALL
    critical_section_test.cpp
)
target_include_directories(critical_section_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Core
)
target_link_libraries(critical_section_tests PRIVATE de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests critical_section_tests)
add_test(NAME critical_section_tests COMMAND critical_section_tests)
set_tests_properties(critical_section_tests PROPERTIES TIMEOUT 30)

add_executable(shutdown_probe EXCLUDE_FROM_ALL support/shutdown_probe.cpp)
target_include_directories(shutdown_probe PRIVATE ${CMAKE_SOURCE_DIR}/src/server)
target_link_libraries(shutdown_probe PRIVATE Threads::Threads)
add_dependencies(wire_tests shutdown_probe)
add_test(NAME shutdown_supervisor COMMAND bash
    ${CMAKE_CURRENT_SOURCE_DIR}/shutdown_supervisor_test.sh
    $<TARGET_FILE:shutdown_probe> ${CMAKE_SOURCE_DIR}/docker/start.sh)
# Two scenarios, each paying start.sh's 6s of staged startup, and the second
# also waiting out the supervisor's 8s bound on a server that ignores SIGTERM.
set_tests_properties(shutdown_supervisor PROPERTIES TIMEOUT 90)

# Call-site diagnostics (docs/TOOLCHAIN.md, "Diagnostics without location
# macros"): Assert/ProtocolAssert and the __END_CATCH stack trace, now carried
# by std::source_location instead of __FILE__/__LINE__/__PRETTY_FUNCTION__.
# Assert.h and Exception.h are de-kernel sources, so this links nothing else.
# A failing assert appends to assertion_failed.log in the working directory,
# hence the build-tree WORKING_DIRECTORY (same reason as scheduler_tests).
add_executable(diagnostics_tests EXCLUDE_FROM_ALL
    diagnostics_test.cpp
)
target_include_directories(diagnostics_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Core
)
target_link_libraries(diagnostics_tests PRIVATE de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests diagnostics_tests)
add_test(NAME diagnostics_tests COMMAND diagnostics_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(diagnostics_tests PROPERTIES TIMEOUT 30)

add_executable(database_timeout_tests EXCLUDE_FROM_ALL database_timeout_test.cpp)
target_link_libraries(database_timeout_tests PRIVATE Database ServerCore de-kernel ${MYSQL_LIBRARY} gtest_main Threads::Threads)
add_dependencies(wire_tests database_timeout_tests)
add_test(NAME database_timeout_tests COMMAND database_timeout_tests)
set_tests_properties(database_timeout_tests PROPERTIES TIMEOUT 20)

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

# Critical-section audit (src/Core/Exception.h). The __ENTER/__LEAVE block is
# owned by a scoped CriticalSection guard, so a hand-written x.unlock() inside
# one now unlocks a non-recursive pthread mutex twice - undefined behaviour, and
# invisible in a diff. This fails on that, on a __LEAVE_CRITICAL_SECTION whose
# argument disagrees with its __ENTER's, on an unclosed section, and on finding
# no sections at all (a wrong working directory must not read as a pass). It
# scans ./src, hence WORKING_DIRECTORY at the repo root. Perl, like tests/arch/ -
# the dev image ships it and has no python3. -q drops the informational lines;
# the ERROR lines and the verdict always print.
add_test(
    NAME critical_section_audit
    COMMAND perl ${CMAKE_SOURCE_DIR}/tests/tools/critical_section_audit.pl -q
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(critical_section_audit PROPERTIES TIMEOUT 60)
