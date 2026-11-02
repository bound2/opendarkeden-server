---
type: Record
title: "Next work plan"
description: "The ranked, self-contained tasks in the server as of 2026-11-02, each written so an agent with no other context can take it: a database error that ends a process, a sanitized run of the suite, the handler layer, the persistence design the login flows need, the entries recorded on 2026-10-27, housekeeping, and what stays blocked or with the owner; the published datagram socket and the port check's unescaped write are done."
tags: [review, plan]
---

# Next work plan

Written 2026-10-26, after the merges of PRs #374-#383, and ranked again on 2026-11-01 after
PRs #385-#395 closed tasks 1 to 7. The [restructuring plan](RESTRUCTURING.md) has two unticked
tasks (5.5 in progress, 6.2 blocked); ratchets R1-R5 and R7-R19 are at 0 and R6a-j at their
baselines (line counts that can still shrink); the [fix log](FIXES.md) holds 432 `fixed` Status
lines and 34 `recorded` ones (counted 2026-11-01). The `recorded` entries are the backlog: each
was found while restructuring and left as it was, by the rule that a move does not change
behaviour. Four new tasks came from reading the code on 2026-11-01: the gameserver's datagram
socket, which the compose file publishes, dispatched relayed GM commands from any sender (task
11, done on 2026-11-02, whose review found task 16, the port check's unescaped database write,
done the same day); a database error ends a process; nothing runs the suite sanitized; and the
handler layer has no instrument.
What is left is below, ranked by the harm it closes, with a cheap fix ahead of a dearer one of
the same harm. Each task names the code, the test it wants and the rule it must keep, so it can
be taken cold. One task per PR.

## How to take a task

1. Read `CLAUDE.md`, then the concept each task links. The rules there bind; this page points.
2. A behaviour change is written test-first. The runtime tests link the production objects
   without `main.cpp`, with supplied repositories ([test coverage](../testing/test-coverage.md));
   a new runtime target is added in `tests/CMakeLists.txt` next to the one it resembles.
   Comments under `src/` and `tests/` describe behaviour in English: no task numbers, dates,
   fix-log references or review history
   ([dead sources and source encoding](../architecture/dead-sources-and-encoding.md)).
3. A wire change is a protocol change: the client repo ships the identical change and the
   goldens are re-recorded ([wire contract](../protocol/wire-contract.md)). A change under
   `src/domain` is done only when `bash tests/tools/decore_client_diff.sh <client-root>` is
   clean against a resynced client ([de-core parity](../protocol/decore-parity.md)).
4. A new ratchet, or a move in R1-R19, changes `tests/ratchet/ratchets.sh` and the plan's
   ratchet table in the same commit ([ratchets](../testing/ratchets.md)). Work that advances
   task 5.5 or 6.2 updates that task's Status line in the same commit.
5. Every `Status:` line of the fix-log entry the task names (one per bullet where an entry has
   several) becomes `fixed (<branch>)` in the same commit
   ([fix log and task status](../testing/fix-log-and-status.md)). Format only the files you
   touched ([formatting](../build/formatting.md)).
6. Verify with `make dev-test` before pushing: CI runs on master pushes and manual dispatch
   only, to conserve Actions minutes ([CI](../build/ci.md)). The owner's standing instruction,
   not written in a concept: every PR gets a two-reviewer adversarial review of its whole diff
   before merge.
7. Update this page in the same commit: move the task to *Done* with its PR, or narrow it. A
   changed concept gets a `docs/log.md` entry; moving a task here needs none. Run
   `perl tests/tools/check_knowledge_bundle.pl .` for any docs change.
8. Do not decide an item under *Blocked or with the owner*; they are the repository owner's
   calls.

## Tasks

Ranked 2026-11-01. The order is the rank and the numbers are identities, so a *Done* entry
keeps its name: 16 came from task 11's review (both are under *Done*), 10 to 13 were new, 8
stays, 14 ranks the entries recorded on 2026-10-27 that no task held, 9 stays, 15 is housekeeping.

### 10. A database failure ends a process

**What:** `DatabaseError` (`src/server/database/DatabaseError.h`) is deliberately unrelated to
`Throwable` and to `std::exception`, as its header says: the `__END_CATCH_NO_RETHROW` sites
(`grep -rn '__END_CATCH_NO_RETHROW' src | wc -l`, 193 on 2026-11-01) swallow a `Throwable`, and
a database failure was not to be swallowed by them. The other side of that choice is that
nothing stops it on the way up either: `ZoneGroupThread::run`'s catches take `Throwable&` and
rethrow, `__END_DEBUG` rethrows, and `ManagedThread::start` turns anything that escapes a worker
into `ServerShutdown::fail()`. The fix log has two paths that end a process this way: *A flag
war step that throws after queuing its next step is queued twice* (`FlagWar::executeStart` and
`executeFinish`, whose database work throws a `DatabaseError` that "no `catch (Throwable&)`
stops, so it ends the main loop first") and *A database failure while a login waits on a kick
escapes to the main loop* (`LoginPlayer::processCommand`'s kick-wait branch, before the dispatch
task 3 wrapped, reaching `ClientManager::run`, "possibly after LOGON was written"). Any other
query that fails inside a zone-group tick or a heartbeat does the same.

**How:** the decision is per site, not per type: where a database error is stopped and what a
failed query means there. Keep the type as it is (its header records why) and catch it by name
at the three sites: `LoginPlayerManager::processCommands`, where the kick-wait path then retires
the login as task 3's wrap does; the flag war's steps, where a step whose database work throws
leaves one schedule, not two (queue the next step after the work that can throw, and stop the
error at the heartbeat); and the zone-group tick, where a failed query is logged and the tick
goes on with the player disconnected or the step skipped. Record the decision as a design note
in [exceptions and outcomes](../architecture/exceptions-and-outcomes.md). Tests: the runtime
tests link the production objects with supplied repositories
([test coverage](../testing/test-coverage.md)), so each path gets a test whose repository throws
a `DatabaseError` and expects the loop to continue, in the shape of
`tests/login_handler_failure_test.cpp` and `tests/game_flag_schedule_test.cpp`. Both entries'
Status lines become `fixed`. Rule kept: `throw Error("text")`, never a string
([exceptions and outcomes](../architecture/exceptions-and-outcomes.md)), and a process ends only
as [lifecycle rules](../startup/lifecycle-rules.md) say.

### 12. A sanitized run of the suite

**What:** `.github/workflows/cpp20.yml` runs `tools/devbuild.sh test` and `build`, Debug builds
in the Zig container, and the gateway's `cargo` checks; nothing in it is sanitized. The only
sanitized configure is the fuzz build (`DARKEDEN_BUILD_FUZZERS=ON`: ASan, UBSan and
`_GLIBCXX_ASSERTIONS` with clang-18 in `tools/fuzz/Dockerfile.fuzz`), which adds its flags to
every target it configures, so with `DARKEDEN_BUILD_TESTS=ON` beside it the whole suite builds
sanitized today (`CMakeLists.txt`); no job and no make target runs that pair
([fuzzing](../protocol/fuzzing.md)). `wire_tests`, the runtime tests and the decision tests
have never run under a sanitizer. The client runs its suite under ASan on three platforms and
UBSan on two, and those runs found what its plain suite could not. A first run on 2026-11-02
(clang 18 with `-fsanitize=address,undefined`, lld, outside the fuzz image) built the whole
suite and failed 14 of 93 tests, none of them the datagram tests or the fuzz replays: the eleven
test binaries that link `tests/support/AllocationProbe.cpp` abort on an alloc-dealloc mismatch
before their first suite runs, since the probe's `operator delete` frees with `free()` what
GoogleTest's `std::stable_sort` took from the library's `operator new(size_t, nothrow_t)`, which
the probe does not replace; `wire_tests` leaks 5896 bytes in 499 allocations (the first report's
site is `packet_store_test.cpp`'s own `newPetStashItem`), `database_timeout_tests` 656 bytes in
4 and `database_manager_tests` 22960 bytes in 140.

**How:** first run the existing pair from the fuzz image (a clang-18 configure with
`DARKEDEN_BUILD_FUZZERS=ON` and `DARKEDEN_BUILD_TESTS=ON`, then ctest) and record what it finds;
then a `make dev-test-asan` that wraps it, documented in
[build commands](../build/build-commands.md) and [testing overview](../testing/overview.md), and
a `workflow_dispatch`-only job, so master pushes do not double their minutes
([CI](../build/ci.md)). Leak detection on where the suite is leak-clean, otherwise off with the
reason recorded. Expect findings: each goes in the fix log with its test and is fixed in a PR
of its own. Rule kept: `NDEBUG` is never defined, and `DE_OWNERSHIP_CHECKS` is armed only in
Debug ([build commands](../build/build-commands.md)).

### 13. The handler layer: asserts and indices on wire values

**What:** measured 2026-11-01 with
`grep -rn 'Assert(' src/server/gameserver/handler --include=*.cpp | wc -l`: 879 `Assert`
lines in 152 of the 164 handler files, and no `ProtocolAssert`. Seven name a packet getter in
the assert and no subscript does; the rest take the value through a local, which a spelling
count cannot see. `GamePlayer::processCommand` (`src/server/gameserver/GamePlayer.cpp`) wraps
the dispatch in `catch (...)` and throws `DisconnectException`, so a failed `Assert` on a value
a client chose disconnects that client, appends to `assertion_failed.log` (never rotated, like
`datagram.txt`), and leaves what the handler changed before the assert, under the group mutex,
half applied. The fuzz targets stop at `read()` ([fuzzing](../protocol/fuzzing.md)); nothing
instruments the handlers.

**How, in order, each its own PR series:** (1) an audit script in `tests/tools/` over
`src/server/gameserver/handler/`: every subscript and table lookup whose index is a packet
value, followed by value through locals, failing closed on a container it does not know, with
an allowlist for the tables that range-check themselves; its first count is ratchet R20's
baseline ([ratchets](../testing/ratchets.md)). (2) The asserts a wire value can reach become
refusals: `ProtocolAssert` where the value is a protocol violation, or the refusal reply the
handler already sends for its other failures, each with a handler test in the runtime suite.
(3) A gameserver handler fuzz target over the runtime tests' world with supplied repositories,
seeded from the goldens, with a regression input and a test for every crash. Rule kept: a
`read()` refuses a malformed body only with a `ProtocolException`
([frame gate](../protocol/frame-gate.md)), and zone-group state is touched only under the
group's mutex ([the mutation rule](../threading/mutation-rule.md)).

### 8. Three login flows that cannot be resumed after a partial write

**What:** `recorded` entries of 2026-10-02: *Character creation can leave partial rows after a
later failure* (`CLCreatePCHandler`), *A failed deletion after Slayer retirement cannot be
resumed by the same request* and *A partial registration cannot be restarted and failed write
acknowledgements retain effects*. Each is a persistence design gap: independent repository
writes with no transaction or compensation, pinned as they are by
`tests/login_character_creation_test.cpp`, `login_character_deletion_test.cpp` and
`login_registration_test.cpp`.

**How:** one design, not three fixes. Add a repository operation per flow that runs its writes
in one transaction (`MySQL*Repository` impls behind the `*Repository.h` seams,
[repository seams](../architecture/repository-seams.md)) or a compensation the retry can finish.
Write the design as a short section in
[character and account flows](../login/character-and-account-flows.md) first and have it
reviewed before the code; `make integration-test` (MySQL 5.7 in docker) is where a rollback is
proven. Keep the phase rules those pages state: a reply is sent before session state is published.

### 14. The entries recorded on 2026-10-27 that no task held, one PR each

Found while tasks 1 to 7 were done, each with a Status line naming its branch; the two that end
a process are task 10's, and the dragon eye's return stays task 9's. Cheapest first:

- *`GCShowUnionInfo::read` takes any guild count*: gate the count at the factory's budget (the
  master guild and three others) before the first row, as its siblings do, with a
  `ListCountTest` row in `tests/packet_read_bounds_test.cpp`; the client's copy
  (`Client/Packet/Gpackets/GCShowUnionInfo.cpp`) gets the same cap in a linked commit. No byte
  moves ([wire contract](../protocol/wire-contract.md)).
- *A guild record's adder and writer take any roster count*: `GuildInfo2::addGuildMemberInfo`
  refuses a member past `kMaxMemberCount`, pinned in the wire tests.
- *A flag or sweeper offered outside its war gets no answer*: `CGRelicToObjectHandler`'s
  `executeFlag` and `executeSweeper` send `GCCannotAdd`, as the handler's other refusals do; a
  test in `tests/game_dragon_eye_test.cpp`'s shape.
- *A mysterious item's fallback is checked against the last failed draw*:
  `getRandomMysteriousItem` checks the fallback against the buyer, not the tenth draw;
  `tests/game_mysterious_item_test.cpp` extends.
- *A level war's schedule pointer outlives the schedule it names*, both bullets: the pointer is
  cleared when the queue takes the schedule and `~LevelWarManager` deletes only what it owns;
  the `hasWar()` and `hasToDayWar()` flags other zones' threads read become atomics, or the
  question is posted to the owning group
  ([cross-thread communication](../threading/cross-thread-communication.md));
  `tests/game_level_war_command_test.cpp` is the place.

### 9. Smaller recorded items

Each is one PR, with a fix-log entry naming the branch that found it:

- *A dragon eye sent home across groups can outrun its return's three attempts*: the return
  captures the row when it is posted (`war/WarZoneWork.cpp`). Fix: have the retry read the row
  when it runs, or have the delayed add carry the row's save.
- *The three races' guild zone counters share one word with no partition*: a race reaches the
  next one's range after 10000 guilds. Lowest, with the relic rows below.
- *A war relic's row outlives the run that made it*: a boot sweep of this server's relic rows
  (ids are partitioned per server); nothing reads them, so lowest.
- Not tasks: the *CGPortCheck* registration (`recorded, left as it is`; the entry says the
  registration is not the defect), the native-macOS entries (platform quirks with workarounds),
  and the shared-rules entries, which keep the server's own balance rules; two of them are
  listed under the owner below.

### 15. Housekeeping, low

- `tests/integration/mysql_test.sh` tests against `mysql/mysql-server:5.7` (`MYSQL_IMAGE`), end
  of life since 2023; add an 8.4 LTS leg while [database setup](../database/overview.md) says
  5.7 or 8.
- [Native macOS builds](../build/native-macos.md): the `assert.h` case clash that forces the
  system header in, and the three tests that fail only there (`ratchets`,
  `proxy_acceptor_tests`, `shutdown_supervisor`).
- 14 of the 465 registered factories have no golden
  ([wire contract](../protocol/wire-contract.md)): record which and why, and add one where the
  packet is client-facing (with the client's copy).
- The R6 god files can keep shrinking (`Zone.cpp` 1,172 lines, `ConsoleCommands.cpp` 1,574;
  [the plan](RESTRUCTURING.md)'s ratchet table).
- `bash tests/tools/wire_inventory_diff.sh <client-root>` fails on 42 read budgets on
  2026-11-01, `CGSMSSend` (114 here, 108 in the client) and GC packets this server budgets
  larger than the client reads. A CG budget the client's `write` can exceed is this
  repository's to raise; the GC ones are the client's plan's to read, since what matters there
  is whether this server can write more than the client admits.

## Blocked or with the owner

- **6.2 Integration tier in CI** is `blocked (no Actions minutes)` in the plan. `make
  integration-test` is the local stand-in; run it before a repository change, and do not add a
  workflow.
- **Shared rules residuals.** Task 3.6 is done; what its slices left over is listed in the
  client repo's `docs/reviews/RESTRUCTURING.md` task 4.12 and needs the owner: the
  advancement-class equip check, a separate castle-tax wire field (the fix log's *Shared castle
  tax* second bullet: the shop reports a ratio of 100 or below as a discount where the handler
  taxes only above 100), pets for any race. Also from the fix log: *Shared equip rules*' wrap
  (a level requirement wraps at 256, so a level-150 item with a +120 option asks for level 14;
  likely unreachable from the seed data, which an agent may check and record), and the party
  bonus that is never granted, a balance decision.
- **Transport encryption**: the native TCP link is unencrypted, login credentials included (the
  WebSocket gateway serves `wss://` with TLS in front). A design decision for the owner, taken
  with the client. Recommendation (2026-11-01): make the TLS
  [WebSocket gateway](../operations/websocket-gateway.md) the only public endpoint and stop
  publishing the raw TCP ports; native clients can use the gateway already, so no wire change
  is needed.
- **A world save on shutdown.** `docker compose down` drains the gameserver's workers and does
  not introduce a full world-save operation
  ([running the servers](../operations/running-the-servers.md)); whether one is wanted before a
  deployment that matters is the owner's call.
- **Task 11 (a)**: whether `9997/udp` stays published (a native client's port check arrives
  there, and with a routable catalogue address the loginserver's datagrams to the gameserver
  hairpin through it), what the datagram sockets bind, and what closes the forged-source
  residual the fix log records as *A forged source address passes the datagram gate*: binding
  the loginserver's socket, which no client uses, to a peer-facing or loopback address, ingress
  filtering, or a shared secret on the server links, a wire change shipped with the client
  ([datagram sources](../protocol/datagram-sources.md)).

## Done

PRs #386-#391 were closed unmerged: their commits reached master as the stack PR #392.

- **1. `*command LevelWar` edits a zone's schedule from another thread** (PR #385,
  `fix/level-war-command-post`): `opLevelWar` posts `LevelWarManager::manualStart` to the zone's
  group through `de::war::postToZone`; `tests/game_level_war_command_test.cpp` runs the command
  from another thread and sees the queued war pulled forward only when the group drains its
  mailbox.
- **2. `*CTF` races the main thread's flag-war heartbeat** (PR #386, `fix/flag-schedule-lock`):
  `FlagManager` pops the due schedule under a leaf schedule mutex and runs it after releasing it,
  every add and `manualStart` takes the mutex, and `startFlagWar` is handed the war's length;
  `tests/game_flag_schedule_test.cpp` heartbeats and pulls forward on two threads.
- **3. The loginserver's handler dispatch is not wrapped** (PR #387, `fix/login-dispatch-wrap`):
  `LoginPlayer::processCommand` passes a handler's `ProtocolException` and `ConnectException` on
  and turns anything else into a logged `DisconnectException`; `tests/login_handler_failure_test.cpp`
  throws each kind from a handler over a loopback connection.
- **4, one bullet: wolf form rejects every skill** (PR #388, `fix/wolf-form-skills`): the four
  `isAbleToUse*Skill` guards have their `&&` allow-lists again, and the object, tile and inventory
  skill handlers pass the skill being used, so a wolf howls and attacks and casts nothing else;
  `tests/game_wolf_form_skill_test.cpp` pins each guard per skill.
- **5. A slayer's shoulder armour leaving its shape is broadcast as the trousers** (PR #389,
  `fix/shoulder-takeoff-slot`): `Slayer::takeOffItem` and `Slayer::removeShape` take the
  `GCTakeOff` slot from one lookup, `slayerTakeOffSlot`, which names `ADDON_SHOULDER` for the
  shoulder armour; `tests/game_slayer_take_off_test.cpp` pins its slot per item class.
- **4, one bullet: `getRandomMysteriousItem` gives female vampires item type 1 for every class**
  (PR #390, `fix/mysterious-item-class`): the vampire fallback is
  `vampireMysteriousFallbackItemType`, type 1 only for a woman's vampire coat;
  `tests/game_mysterious_item_test.cpp` pins it.
- **4, one bullet: `Tile::deleteCreature(MoveMode)` picks the wrong `deleteObject`** (PR #391,
  `fix/tile-delete-by-mode`): nothing called it, so the overload is declared deleted.
- **6. Count gates the login and guild reads still lack** (PR #393, `next-plan/batch-2`): `LCWorldList`,
  `LCServerList` and `GuildInfo2` refuse a count past their factory budget before the first row,
  the four reads append a row before releasing its owner, and the sharedserver's
  `Guild::makeInfo` sends no more members than the read admits; `ListCountTest` in
  `tests/packet_read_bounds_test.cpp` pins the refusals and a full world list. No fuzz regression
  input was added: none fails on master, where the old out-of-bytes refusal and the new gate are
  both protocol errors the target treats as clean.
- **9, one item: a dragon eye has no place in the relic handlers** (PR #393, `next-plan/batch-2`):
  `CGRelicToObjectHandler` refuses a relic it has no branch for with `GCCannotAdd` instead of
  disconnecting the sender, and `deleteRelicEffect`'s `WAR_ITEM` case has its `break`;
  `tests/game_dragon_eye_test.cpp` pins the second.
- **4, the last three bullets** (PR #394, `next-plan/batch-3`): the never-firing checks are not defects
  (the learn-skill handler's implemented-skill list refuses an out-of-range type first, the pet
  item is the one just taken from the inventory, and `opset item_prob_ratio`'s check was turned
  off by an operator in the recovered source). The sharedserver's `GameServerPlayer`, the gameserver's `SharedServerClient` and
  mofus `MPlayer` declare `processCommand(bool) override`, and the unused `DummyQuestStatus` is
  deleted. `MotorcycleLoader::load(Creature*)` drops the motorcycle it parses, and
  `~PlayerCreature` deletes the summoned pet; no test owns the two leaks (no leak checker, and
  neither can be built without a database). The satellite bomb's skill slot cannot be null.
- **9, two items** (PR #394, `next-plan/batch-3`): a full guild id counter founds nothing,
  `decideAddGuild` refuses a race whose next zone id is the last, and only an accepted founding
  claims a guild id (`SharedAddGuild` in `tests/shared_add_guild_test.cpp`, `DecideAddGuild` in
  `tests/shared_guild_decision_test.cpp`).
  The character list sends `(CurrentHP, HP)` and `(CurrentMP, MP)` again, as the original's
  right-to-left argument evaluation most likely did; the client reads current then max and shows
  the maximum (`LoginCharacterList` in `tests/login_character_list_test.cpp`).
- **7. Fuzz the loginserver's GM datagrams and the stream's wrap-around** (PR #395, `next-plan/batch-4`):
  `tests/fuzz/fuzz_login_datagram.cpp` reads one datagram through `Datagram::read` as
  `GameServerManager::run` does, seeded by `golden2corpus.pl --datagram` and replayed by
  `fuzz_replay_login_datagram`; its first replay found `Datagram`'s raw reads asserting past the
  datagram, which now throw `InsufficientDataException`. The stream targets' fourth delivery
  starts the input short of the ring buffer's end, and their replay found nothing; nor did the
  first five-minute libFuzzer runs of the datagram and login stream targets
  ([fuzzing](../protocol/fuzzing.md)). Task 5.5 stays `in progress` for its CI fuzz job; the
  libFuzzer runs stay manual.
- **9, one item: the first guild founded after a restart reuses a guild zone id**
  (PR #395, `next-plan/batch-4`): `prepareZoneMaximum` starts each race's counter one past its stored
  maximum (`SharedGuildStartup` in `tests/shared_guild_startup_test.cpp`).
- **11. Unauthenticated datagrams on a published port** (PR #397, `fix/datagram-sources`): both datagram
  loops run each received datagram through `de::DatagramGate` (`src/server/DatagramAdmission.h`)
  before reading it: on the gameserver a client's port check from anywhere, the `LG` packets
  only from the configured `LoginServerIP` and the `GG` packets only from the gameservers the
  catalogue lists; on the loginserver the `GL` packets and `GMServerInfo` only from those
  gameservers; `DatagramPeerIPs` adds the addresses a NAT or a published port makes datagrams
  carry, validated at startup; nothing else from anyone. A refused datagram is counted, reported
  once per 30 seconds and dropped unread ([datagram sources](../protocol/datagram-sources.md),
  owned by `datagram_admission_tests`, the loops' wiring by `datagram_gate_audit`, since each
  loop opens a database connection before it reads and has no test of its own). The compose
  deployment with a routable catalogue address needs the pinned network and the
  `DatagramPeerIPs` its `docker/conf/*.conf` carry. The gate is an address check, not
  authentication: the forged-source residual, the sockets' bind address and the published port
  are the owner's (task 11 (a), under *Blocked or with the owner*). The gate closes the fix
  log's `datagram.txt` entry, and its review found task 16.
- **16. The port check writes the database for any sender, with an unescaped name** (PR #398,
  `fix/port-check-name`): `de::recordPortCheck` (`gameserver/PortCheck.h`) refuses, before any
  statement, a name no character could have (empty, past 20 bytes, a control byte, a token
  character creation refuses: `src/server/CharacterName.h`, the rule both servers share now) with
  `InvalidProtocolException`, and `MySQLSessionRepository` escapes the name of every `UserIPInfo`
  statement through `de::escapeSqlLiteral` (`database/SqlLiteral.h`), the escape the Exchange
  repository used alone and now delegates to; owned by `port_check_tests` over a fake session
  repository, `sql_literal_tests` for the pass without a connection, and `SessionMySQL`'s two
  UserIPInfo cases under `make integration-test`. Left, in the fix log's entry: `UserIPInfo.Name`
  is `varchar(10)`, so a longer name is stored truncated, and one row per distinct name is still
  any sender's to add.
