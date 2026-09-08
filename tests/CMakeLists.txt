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
    datagram_frame_test.cpp
    outcome_test.cpp
    packet_roundtrip_test.cpp
    packet_encrypter_test.cpp
    packet_exchange_test.cpp
    packet_field_bounds_test.cpp
    packet_gameserver_handshake_test.cpp
    packet_login_test.cpp
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
    integration/mysql_login_purge_repository_test.cpp
    integration/mysql_loginserver_repository_test.cpp
    integration/mysql_servercore_repository_test.cpp
    integration/mysql_sharedserver_repository_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLBalanceInfoRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLBloodBibleSignRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLBulletinBoardRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLComebackEventRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLContentInfoRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLExchangeRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLMessageRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLRegenZoneRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLZoneInfoRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLCharacterRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLCharacterPurgeRepository.cpp
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
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLSpecialEventRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLSessionRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLSkillSaveRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLStashRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/repository/MySQLWarInfoRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/loginserver/repository/MySQLLoginAccountRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/loginserver/repository/MySQLLoginCharacterPurgeRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/loginserver/repository/MySQLLoginCharacterRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/loginserver/repository/MySQLLoginConfigRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/sharedserver/repository/MySQLSharedConfigRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/server/sharedserver/repository/MySQLSharedGuildRepository.cpp
)
target_include_directories(mysql_repository_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/integration
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/gameserver
    ${CMAKE_SOURCE_DIR}/src/server/loginserver
    ${CMAKE_SOURCE_DIR}/src/server/sharedserver
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

# Cross-thread mailbox (src/server/Mailbox.h): header-only, no dependencies
# beyond the standard library, so nothing but gtest is linked.
add_executable(mailbox_tests EXCLUDE_FROM_ALL
    mailbox_test.cpp
)
target_include_directories(mailbox_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
)
target_link_libraries(mailbox_tests PRIVATE gtest_main Threads::Threads)
add_dependencies(wire_tests mailbox_tests)
add_test(NAME mailbox_tests COMMAND mailbox_tests)
set_tests_properties(mailbox_tests PROPERTIES TIMEOUT 30)

# Copy-on-write snapshot (src/server/Snapshot.h): header-only, standard
# library only.
add_executable(snapshot_tests EXCLUDE_FROM_ALL
    snapshot_test.cpp
)
target_include_directories(snapshot_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
)
target_link_libraries(snapshot_tests PRIVATE gtest_main Threads::Threads)
add_dependencies(wire_tests snapshot_tests)
add_test(NAME snapshot_tests COMMAND snapshot_tests)
set_tests_properties(snapshot_tests PROPERTIES TIMEOUT 60)

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

# Account password hashing (src/server/loginserver/PasswordHash.cpp) over the
# vendored argon2 (third_party/argon2). Upstream's own known-answer vectors
# pin the vendored library; the seed-account case reads initdb/DARKEDEN.sql
# (tools/devbuild.sh syncs initdb/ for it) and pins the shipped hashes.
add_executable(password_hash_tests EXCLUDE_FROM_ALL
    password_hash_test.cpp
)
target_compile_definitions(password_hash_tests PRIVATE
    PASSWORD_TEST_INITDB_DIR="${CMAKE_SOURCE_DIR}/initdb"
)
target_link_libraries(password_hash_tests PRIVATE PasswordHash gtest_main Threads::Threads)
add_dependencies(wire_tests password_hash_tests)
add_test(NAME password_hash_tests COMMAND password_hash_tests)
# A dozen 64 MiB argon2id computations; generous for a loaded CI runner.
set_tests_properties(password_hash_tests PROPERTIES TIMEOUT 120)

# The loginserver's character-creation decision (docs/RESTRUCTURING.md 3.1):
# decideCreatePC() returns Outcome<CreatedCharacter, CreatePCRejection>, so
# every rejection and the accepted rows are testable against a fake
# LoginCharacterRepository with no database. Only de-kernel is linked (the
# type tables, and filelog for the Ousters warning) — pulling the transport
# or the login server's own state into the decision would break this link.
# The Ousters warning appends to CreatePC.log in the working directory,
# hence the build-tree WORKING_DIRECTORY (same reason as scheduler_tests).
add_executable(character_creation_tests EXCLUDE_FROM_ALL
    character_creation_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/loginserver/CharacterCreation.cpp
)
target_include_directories(character_creation_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server/loginserver
)
target_link_libraries(character_creation_tests PRIVATE de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests character_creation_tests)
add_test(NAME character_creation_tests COMMAND character_creation_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(character_creation_tests PROPERTIES TIMEOUT 60)

# The loginserver's character-selection decision (docs/RESTRUCTURING.md 3.1):
# decideSelectPC() returns Outcome<SelectedCharacter, SelectPCRejection> over a
# fake LoginCharacterRepository and a fake SelectPCTopology, so every rejection
# and the routing of an accepted selection are testable with neither a database
# nor a game server. Only de-kernel is linked (the type tables) — pulling the
# transport, the server-info managers or the login server's own state into the
# decision would break this link.
add_executable(character_selection_tests EXCLUDE_FROM_ALL
    character_selection_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/loginserver/CharacterSelection.cpp
)
target_include_directories(character_selection_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server/loginserver
)
target_link_libraries(character_selection_tests PRIVATE de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests character_selection_tests)
add_test(NAME character_selection_tests COMMAND character_selection_tests)
set_tests_properties(character_selection_tests PROPERTIES TIMEOUT 60)

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

# The loginserver's login decision (docs/RESTRUCTURING.md 3.1): decideLogin()
# returns Outcome<LoginAccepted, LoginRejection>, so every refusal, the
# precedence between them and the content of the OK reply are testable
# against a fake LoginAccountRepository with no database. PasswordHash is
# linked because the password check runs against the real argon2id
# implementation; de-kernel carries VSDateTime and filelog. Pulling the
# transport or the login server's own state into the decision would break
# this link. The refusal logs (keydiff.txt) and the days-left log
# (PayPlayDateLog.txt) are appended in the working directory, hence the
# build-tree WORKING_DIRECTORY (same reason as scheduler_tests).
add_executable(login_decision_tests EXCLUDE_FROM_ALL
    login_decision_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/loginserver/LoginDecision.cpp
)
target_include_directories(login_decision_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server/loginserver
)
target_link_libraries(login_decision_tests PRIVATE PasswordHash de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests login_decision_tests)
add_test(NAME login_decision_tests COMMAND login_decision_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
# A handful of 64 MiB argon2id computations in the password cases.
set_tests_properties(login_decision_tests PROPERTIES TIMEOUT 180)

# The Exchange system's decisions (docs/RESTRUCTURING.md 3.1):
# decideBuyListing() / decideCancelListing() / decideBuyerClaim() /
# decideSellerClaim() return Outcome<..., ExchangeRejection>, so every
# refusal, the precedence between them and the terms an accepted buy is
# priced on are testable against a fake ExchangeRepository with no
# database. formatExchangeError() is here too: its strings are what
# GCExchangeBuy puts on the wire. ExchangeService itself is not linked -
# it needs a PlayerCreature and an Item, which is why the decisions were
# split out of it. de-kernel carries the ExchangeListing wire struct.
add_executable(exchange_decision_tests EXCLUDE_FROM_ALL
    exchange_decision_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/exchange/ExchangeDecision.cpp
)
target_include_directories(exchange_decision_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server/gameserver
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/exchange
)
target_link_libraries(exchange_decision_tests PRIVATE de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests exchange_decision_tests)
add_test(NAME exchange_decision_tests COMMAND exchange_decision_tests)
set_tests_properties(exchange_decision_tests PROPERTIES TIMEOUT 60)

# The loginserver's character-deletion decision (docs/RESTRUCTURING.md 3.1):
# decideDeletePC() returns Outcome<void, DeletePCRejection> over a fake
# LoginCharacterPurgeRepository, so the ownership check, the compare-and-set
# retirement and the precedence between the refusals are testable with no
# database. Only de-kernel is linked (the type tables) — pulling the
# transport or the login server's own state into the decision would break
# this link.
add_executable(character_deletion_tests EXCLUDE_FROM_ALL
    character_deletion_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/loginserver/CharacterDeletion.cpp
)
target_include_directories(character_deletion_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server/loginserver
)
target_link_libraries(character_deletion_tests PRIVATE de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests character_deletion_tests)
add_test(NAME character_deletion_tests COMMAND character_deletion_tests)
set_tests_properties(character_deletion_tests PROPERTIES TIMEOUT 60)

# The loginserver's reconnect decision (docs/RESTRUCTURING.md 3.1):
# decideReconnectLogin() returns Outcome<ReconnectAccepted,
# ReconnectRejection> over a fake LoginAccountRepository and a fake
# ReconnectSession, so every refusal, the precedence between them and the
# session state the decision writes are testable with neither a database nor
# a socket. Only de-kernel is linked (the type tables) — pulling the
# transport or the login server's own state into the decision would break
# this link.
add_executable(reconnect_decision_tests EXCLUDE_FROM_ALL
    reconnect_decision_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/loginserver/ReconnectDecision.cpp
)
target_include_directories(reconnect_decision_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server/loginserver
)
target_link_libraries(reconnect_decision_tests PRIVATE de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests reconnect_decision_tests)
add_test(NAME reconnect_decision_tests COMMAND reconnect_decision_tests)
set_tests_properties(reconnect_decision_tests PROPERTIES TIMEOUT 60)

# The loginserver's account-registration decision (docs/RESTRUCTURING.md
# 3.1): decideRegisterPlayer() returns Outcome<LoginNewAccount,
# RegisterPlayerRefusal> over a fake LoginAccountRepository, so every
# refusal, the precedence between them and the Player row an accepted
# registration produces are testable with no database. PasswordHash is
# linked because the decision hashes the password with the real argon2id;
# de-kernel carries the type tables. Pulling the transport or the login
# server's own state into the decision would break this link.
add_executable(registration_tests EXCLUDE_FROM_ALL
    registration_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/loginserver/Registration.cpp
)
target_include_directories(registration_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server/loginserver
)
target_link_libraries(registration_tests PRIVATE PasswordHash de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests registration_tests)
add_test(NAME registration_tests COMMAND registration_tests)
# A handful of 64 MiB argon2id computations in the accepted cases.
set_tests_properties(registration_tests PROPERTIES TIMEOUT 180)

# The gameserver's guild join and registration decisions
# (docs/RESTRUCTURING.md 3.1): decideGuildJoinAttempt,
# decideGuildRegistration, decideGuildJoinConfirm and decideGuildRequirements
# return Outcome<..., GuildJoinRejection> over a fake GuildRepository, so
# every refusal, the precedence between them, the quit-penalty date
# arithmetic and the response code each refusal carries to each race are
# testable with no database. The handlers themselves are not linked - they
# need a creature and a socket, which is why the decisions were split out of
# them. de-kernel carries the type tables and the NPC response codes.
add_executable(guild_join_decision_tests EXCLUDE_FROM_ALL
    guild_join_decision_test.cpp
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/guild/GuildJoinDecision.cpp
)
target_include_directories(guild_join_decision_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server/gameserver
    ${CMAKE_SOURCE_DIR}/src/server/gameserver/guild
)
target_link_libraries(guild_join_decision_tests PRIVATE de-kernel gtest_main Threads::Threads)
add_dependencies(wire_tests guild_join_decision_tests)
add_test(NAME guild_join_decision_tests COMMAND guild_join_decision_tests)
set_tests_properties(guild_join_decision_tests PROPERTIES TIMEOUT 60)
