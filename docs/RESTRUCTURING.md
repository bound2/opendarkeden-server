# Server Restructuring Plan

Living, trackable plan for restructuring the DarkEden server toward the
architecture style of `sidecar` (gcci-labs): strictly layered modules, every
architectural rule owned by a test that fails — never by a memory — and
shrink-only ratchets for legacy debt. No language port; no behavior changes on
the wire or in game rules unless a task says so explicitly.

Modeled on sidecar's `docs/RESTRUCTURING.md` + `CODE_REVIEW` fix-log
conventions.

## How to use this document

- Every task has a checkbox and a `> **Status:**` line. Update the status line
  **in the same commit** as the work it describes (sidecar rule). Allowed
  values: `not started` | `in progress (<what remains>)` | `done (<commit>)` |
  `dropped (<why>)`.
- A task is only `done` when its **Owner** exists — the test/mechanism that
  keeps the rule true from then on. Landing the change without the owner is
  `in progress (owner missing)`.
- Ratchet numbers (below) may only go **down**. Re-measure with the given
  command before and after a change that claims progress; commit the updated
  number with the change.
- All tests run **locally** (`make test` once Phase 1 lands). CI is
  deliberately the *last* phase — GitHub Actions minutes are exhausted, and
  nothing here depends on CI existing. Until then the discipline is: run the
  suite before every push.

## Goals / non-goals

**Goals**

1. The client/server wire contract (433 packet types, shuffle-encryption
   included) pinned by tests so protocol drift is a reviewable diff.
2. Three strictly layered CMake modules — `de-kernel` ← `de-core` ← per-server
   apps — with the layering enforced at build/test time.
3. Game logic testable without a live MySQL or a socket: repositories for
   persistence, an `Outcome` type for gameplay results, pure formula functions.
4. God files decomposed behind routers with declared gating.

**Non-goals**

- Porting to another language. (Analysis 2026-08-29: full rewrite of ~502k
  LOC / 433-packet byte-exact protocol; not economical. The wire pin in
  Phase 1 is the prerequisite for a port anyway, if that ever changes.)
- Rewriting the threading model. One `ZoneGroupThread` per zone group with
  serial in-group processing is sound; Phase 3 codifies it, nothing replaces it.
- Touching the client repo beyond the shared wire-inventory file (Phase 1.4).

## Ratchets (shrink-only)

Baselines measured 2026-08-29. Run commands from repo root (bash).

| # | Metric | Baseline | Command |
|---|--------|---------:|---------|
| R1 | `g_p*` global-singleton extern declarations | 351 | `grep -rE '^extern .*\* g_p' src --include='*.h' --include='*.cpp' \| wc -l` |
| R2 | Files with inline SQL in gameserver root | 104 | `grep -lE 'executeQuery' src/server/gameserver/*.cpp src/server/gameserver/*.h \| wc -l` |
| R3 | Files with inline SQL anywhere outside `database/` | 320 | `grep -rlE 'executeQuery' src --include='*.cpp' \| grep -v 'server/database' \| wc -l` |
| R4 | Packet headers with `execute()` still on the packet | 481 | `grep -rlE 'void execute\(Player' src/Core --include='*.h' \| wc -l` |
| R5 | `__BEGIN_TRY` control-flow macro sites in de-core candidates | 5,984 | `grep -rE '__BEGIN_TRY' src/server/gameserver --include='*.cpp' \| wc -l` |
| R6 | Line count of god files (each tracked separately) | see table below | `wc -l <file>` |

God-file baselines (R6):

| File | Baseline lines |
|------|---------------:|
| `src/server/gameserver/Zone.cpp` | 7,616 |
| `src/server/gameserver/skill/SkillUtil.cpp` | 5,631 |
| `src/Core/CGSayHandler.cpp` | 3,967 |
| `src/server/gameserver/InitAllStat.cpp` | 4,158 |
| `src/server/gameserver/Slayer.cpp` | 3,511 |
| `src/server/gameserver/skill/SkillFormula.cpp` | 2,640 |

Once Phase 1's test harness exists, encode R1–R5 as **ratchet tests**: the
checked-in expected count lives next to the test, the test fails when the
measured count *exceeds* it, and lowering it is part of the shrinking commit
(sidecar's `ConventionScanTest` shrink-only pattern).

---

## Phase 1 — Pin the wire contract

The 433 packet classes are duplicated by hand in the client repo
(`client/Client/Packet/*`); the byte layout is the only contract, and today
nothing checks it. This phase makes protocol drift visible and reviewable
before anything else moves. Everything later shelters under this pin.

- [x] **1.1 Test harness in CMake.** Add GoogleTest (FetchContent or vendored),
  a `tests/` tree, and a `make test` target that builds and runs it locally.
  Debug build, no MySQL/network required for the unit tier.
  > **Status:** done — GoogleTest 1.12.1 via FetchContent, `tests/` +
  > `make test` (`-DDARKEDEN_BUILD_TESTS=ON`); the `TestPackets` library
  > compiles the full packet set with no server-type macro (7 files needed
  > the standard `#ifdef __GAME_SERVER__` guards they were missing).
  - Owner: the harness itself; `make test` documented in CLAUDE.md.

- [ ] **1.2 Packet round-trip tests with golden byte fixtures.** For each
  packet direction (start GC/CG, then LC/CL, then inter-server), a test that
  constructs a canonical instance, `write()`s it, compares against a
  checked-in golden byte file, then `read()`s it back and compares fields.
  Must cover the `__USE_ENCRYPTER__` path for every `code % 3` shuffle branch
  of `SHUFFLE_STATEMENT_*` (`src/Core/EncryptUtility.h`) — the shuffle *is*
  part of the wire format.
  > **Status:** in progress — harness + goldens and loopback round-trips
  > landed for GCMoveOK, CGMove, CGSay, CGWhisper, plus a framed-bytes
  > golden and header-width assertions; remaining GC/CG coverage
  > outstanding. **Adversarial review (2026-08-29) named the specific
  > gaps, in priority order:**
  > 1. Only 2 of the 17 encrypter-using packets are pinned, and both use
  >    `SHUFFLE_STATEMENT_3`. `_4` (case 3 = `D;A;C;B`) and `_5` (cases
  >    3/4) are **non-rotations** — exactly what a hand-maintained client
  >    copy gets wrong — and nothing pins them. `kEncryptCodes` stops at 3,
  >    so `_5` case 4 is unreachable; extend it when adding a `_5` packet.
  >    Unpinned: `CGSkillToSelf`, `CGUseItemFromGear`, `GCMoveError` (_2);
  >    `CGAttack`, `CGDissectionCorpse`, `CGSkillToTile` (_4);
  >    `CGAddZoneToInventory`, `CGSkillToInventory`, `GCAddItemToZone` (_5).
  > 2. For the 459 unpinned packets, `getPacketMaxSize()` is invariant
  >    under field reordering and same-width type changes — e.g. swapping
  >    the two shuffle arguments in both `GCMoveError::read`/`write` passes
  >    the whole suite while transposing X/Y on every odd encrypt code.
  > 3. Test values in the pinned packets are all < 128, so a signedness
  >    flip on `Coord_t`/`Dir_t` yields byte-identical goldens. Prefer
  >    values ≥ 128 and distinct per field when adding packets.
  - Owner: the golden files — any layout change is a byte-diff in the commit.
  - Note: packets whose `read`/`write` depend on game-state globals need those
    globals stubbed; list any such packet here as found (they are also the
    first candidates for 2.3). Found so far: `GCStashList::setStashItem`
    (builds wire fields from live `Item*`; now guarded server-only), and the
    stream-level `EncryptData` "viva 2008" layer is dead code (returns
    immediately), so framing + per-field encrypter is the whole transform.

- [x] **1.3 Wire-layout inventory (`tests/wire-layout.txt`).** A test walks
  every packet factory, records packet ID, name and max body size,
  regenerates the file, and compares it to the committed copy (sidecar's
  `WireFieldInventoryTest` / `wire-fields.txt` pattern). A protocol change
  becomes a reviewable diff in the same commit, not a client crash later.
  Field-order pinning is per-packet via the golden fixtures (1.2) — C++
  offers no reflection to enumerate fields generically.
  > **Status:** done — 463 factories inventoried
  > (`tests/generated/AllPacketFactories.inc`, generated by
  > `tests/tools/gen_factory_list.sh`); the test also proves packet-ID
  > uniqueness and factory/packet ID agreement. Curious find: packet ID 0
  > (CGAddSMSAddress) can never pass `writePacket`'s `Assert(packetID != 0)`.
  - Owner: the inventory test + the ratchet script's freshness/subset checks.

- [ ] **1.4 Share the inventory with the client repo.** Drop the same
  generator + committed inventory into `client` (its packet copies have
  already diverged textually — verified on `GCMoveOK.cpp`, 135 differing
  lines — while layouts are believed equal). First run produces the
  first-ever ground-truth diff of the two protocol copies; any layout
  mismatch found is a **bug to triage immediately**, recorded here.
  > **Status:** not started
  - Owner: matching inventory files in both repos; a cross-check script that
    diffs them (runnable locally from the parent dir).

- [x] **1.5 Ratchet tests for R1–R5.** Encode the ratchet table as tests with
  checked-in expected counts, failing on increase.
  > **Status:** done — `tests/ratchet/ratchets.sh`, run by ctest; fails on
  > increase AND on unrecorded decrease (tighten the baseline in the same
  > commit). Also pins `AllPacketFactories.inc` freshness and checks every
  > `PacketFactoryManager` registration is inventoried (client-only CR/RC
  > factories excepted in `tests/ratchet/factory_exceptions.txt`).
  - Owner: the ratchet tests.

**Phase exit criteria:** `make test` green locally; golden fixtures for at
least all GC/CG packets; inventory committed in both repos with zero
unexplained layout diffs (or every diff triaged and logged here).

### Defects found by adversarial review of this phase (2026-08-29)

Two independent reviewers attacked the suite's own claims. Fixed in the
same branch; recorded here because each is a trap worth not re-entering:

- **CRLF made the whole suite fail on any fresh checkout.** `core.autocrlf`
  is `true` on the dev machine and `.gitattributes` covered only `*.sh` /
  `Dockerfile*` / `*.conf`. A clone produced `0b1603\r\n` goldens, and the
  tests run in a Linux container where `ifstream` does no translation — all
  16 goldens, the inventory and the ratchet freshness check would fail,
  reported as "protocol-breaking change". Fixed with `eol=lf` on
  `tests/golden/*.hex`, `tests/wire-layout.txt`, `tests/generated/*.inc`,
  `tests/ratchet/*.txt`. **Any new committed test-data type needs the same
  treatment.**
- **The frame header had no byte pin.** `writeBody()` calls `packet.write()`,
  never `writePacket()`, so widening `PacketID_t`/`PacketSize_t`/
  `SequenceSize_t` left every golden and the inventory unchanged while
  desynchronising all 463 packets at byte one — the header tests read back
  through the same typedefs and agreed with themselves. Fixed with a framed
  golden plus explicit width assertions.
- **`UPDATE_GOLDENS` accepted any value**, so `UPDATE_GOLDENS=0` silently
  re-recorded every pin and asserted nothing. Now requires exactly `1`.
- **8 of 16 goldens were identical duplicates**: CGSay/CGWhisper never
  reference the encrypter, so their four per-code files advertised coverage
  that did not exist. Reduced to one each, with a test that fails if either
  packet ever starts encrypting.
- **`pump()` could hang forever** on an over-reporting `getPacketSize()`
  (a known drift mode — `SocketOutputStream` logs it to
  `packetsizeerror.txt`). The accepted socket is now non-blocking with a
  bounded idle-poll budget, and `wire_tests` has a ctest `TIMEOUT`.
- **`make test` poisoned the production build cache.** `DARKEDEN_BUILD_TESTS`
  is a cached BOOL and `make debug`/`release` reused the same `build/` dir
  without resetting it, so a later production build pulled in googletest and
  the 948-file `TestPackets`. Fixed with explicit `=OFF` in those recipes
  plus `EXCLUDE_FROM_ALL` on both test targets.
- **`ratchets.sh` overwrote a tracked file with no trap** — an interrupt
  between generate and restore left `AllPacketFactories.inc` clobbered. It
  now generates into a scratch tree.
- Docs corrected: the re-record command pointed at `./build/tests/wire_tests`
  (binaries land in `bin/`), and "the same shape the client builds" was
  wrong — the client compiles with `__GAME_CLIENT__=1`, so `#ifndef
  __GAME_CLIENT__` blocks resolve the opposite way in `TestPackets`.

---

## Phase 2 — Module split: `de-kernel` ← `de-core` ← apps

Sidecar's kernel/core/app with the framework-free lower layers. The CMake
target graph is the first enforcement (a kernel target cannot see core/app
headers); an include-graph test is the ArchUnit analog for what target
visibility can't express.

- [ ] **2.1 Define the target layering in CMake.**
  - `de-kernel`: packet data classes, socket streams, `Types.h`, encrypt
    utilities, `Packet`/`PacketFactory` base. **No** MySQL, Lua, Zone/Creature
    headers, or server-type `#ifdef`s.
  - `de-core`: game domains (skill, item, quest, war, mission, exchange, …) —
    game rules against kernel types and repository *interfaces* only.
  - Apps: `gameserver`, `loginserver`, `sharedserver` executables — DB
    adapters, network transport, handler wiring, composition root.
  Initially the split is aspirational for existing files; new code must land
  in the right target from day one.
  > **Status:** not started
  - Owner: CMake `PRIVATE` include dirs on each target.

- [ ] **2.2 Include-graph architecture test.** A Python script under `tests/`
  (run by `make test`) that parses `#include` edges and fails on forbidden
  ones: kernel → core/app; core → `mysql.h`, socket headers, Lua headers;
  any core/app → kernel-internal detail headers as they get marked. Keep the
  rule list in one file (sidecar's `ArchitectureRules` pattern: extend the
  list deliberately, never weaken a rule to fix a compile error — a violation
  means the class is in the wrong module).
  > **Status:** not started
  - Owner: the include-graph test.

- [ ] **2.3 Strip `execute()` off packets; dispatch table at the composition
  root.** The crux. Today packet classes carry `execute()` → `*Handler` which
  reaches into gameserver internals, with `#ifdef __GAME_SERVER__` /
  `__GAME_CLIENT__` switching (vestige of the once-shared codebase). Replace
  with a per-executable dispatch table (packet ID → handler function)
  registered at each app's composition root; handlers move out of `Core` into
  the app/domain that owns them. Kills the macro switching and is what makes
  `de-kernel` actually framework-free. Migrate direction-by-direction under
  the Phase 1 pin (layout must not change — golden tests prove it).
  Track with ratchet R4 (packets still carrying `execute()`).
  > **Status:** not started
  - Owner: R4 ratchet test + include-graph test (a kernel packet including a
    Zone header fails).

- [ ] **2.4 Move packet sources into the kernel target.** Once a direction's
  handlers are out (2.3), move those packet files under the `de-kernel`
  target. `Core`'s non-packet utilities get sorted kernel-vs-app as touched.
  > **Status:** not started
  - Owner: CMake target membership + include-graph test.

**Phase exit criteria:** `de-kernel` builds standalone with no MySQL/Lua/Zone
includes (include-graph test green); at least GC/CG fully migrated off
`execute()`; all three servers boot and pass a manual smoke test against the
live client.

---

## Phase 3 — Testable domain logic: Outcomes, repositories, ownership rules

Runs as ongoing background work; every extraction is independently mergeable
and sheltered by Phase 1 tests. Ratchets R2/R3/R5 make progress monotonic.

- [ ] **3.1 `Outcome<Events, Rejection>` result type.** C++11 template in
  `de-kernel` mirroring sidecar's `kernel.domain.Outcome`: gameplay mutations
  return `Ok(events)` or `Rejected(reason)`; exceptions reserved for
  programming/config errors. New/refactored domain code uses it; the
  `__BEGIN_TRY/__END_CATCH` macros stop being control flow (ratchet R5).
  > **Status:** not started
  - Owner: R5 ratchet + convention grep test (no new `__BEGIN_TRY` in
    de-core sources).

- [ ] **3.2 Repository extraction.** Pull inline `executeQuery` SQL out of
  game logic into `*Repository` classes behind interfaces (game logic takes
  the interface; the app wires the MySQL implementation). Sidecar convention
  adopted verbatim: **legacy schema quirks live in the repository on
  purpose** — zero-date columns, denormalized tables, encoding oddities are
  quarantined and documented *there*, never leaked into domain types. Order
  of attack: `PlayerCreature`/`Slayer`/`Vampire`/`Ousters` persistence first
  (biggest testability win), then Zone, then the long tail. Ratchets R2/R3.
  > **Status:** not started
  - Owner: R2/R3 ratchet tests; repository unit tests (fake/in-memory
    implementations for domain tests; MySQL-backed integration tier runs
    locally against the existing docker + `initdb/` schema).

- [ ] **3.3 Pure formula functions with unit tests.** Extract
  `SkillFormula`/`SkillUtil` math and stat calculations (`InitAllStat.cpp`)
  into pure functions in `de-core`. These are the highest-value tests in the
  game — they encode balance — and the cheapest to write.
  > **Status:** not started
  - Owner: the formula test suite; R6 line ratchet on `SkillUtil.cpp` /
    `InitAllStat.cpp`.

- [ ] **3.4 Codify thread ownership.** Document (in CLAUDE.md) which state is
  owned by which thread: zone-group state mutated only on its
  `ZoneGroupThread`, cross-group communication via queues only. Add
  debug-build `assertOwnedByZoneThread()` checks on Zone/Creature mutation
  entry points (sidecar analog: "never block the registry mailbox" — the
  invariant is written down *and* asserted).
  > **Status:** not started
  - Owner: the debug asserts.

- [ ] **3.5 Globals → context (long tail).** No big-bang DI. Introduce a
  `GameContext` owning the managers; converted subsystems take it (or narrow
  interfaces) explicitly; the old `g_p*` externs become shims into it until
  their last caller is converted. Ratchet R1.
  > **Status:** not started
  - Owner: R1 ratchet test.

**Phase exit criteria:** no hard gate — this phase *is* the ratchets trending
down. Review checkpoint: when R2 hits 0, close 3.2 and re-baseline R3.

---

## Phase 4 — God files behind routers

- [ ] **4.1 GM-command router for `CGSayHandler.cpp`** (3,967 lines). A
  `CommandRouter` with one class/function per GM command and **gating
  declared at registration** — sidecar's `on(name, limiter, handler)` /
  `onGated(...)` insight: hand-kept permission maps drift silently, so the
  registration site is the single source of truth. A test enumerates
  registered commands and asserts each declares a permission level.
  > **Status:** not started
  - Owner: router registration + the enumeration test; R6 ratchet.

- [ ] **4.2 Split `Zone.cpp`** (7,616 lines) by concern: movement, broadcast,
  spawn/despawn, scan/visibility, persistence (→ 3.2 repository). Mechanical,
  many small commits, each verified by build + smoke.
  > **Status:** not started
  - Owner: R6 ratchet per extracted file.

- [ ] **4.3 Race-class cleanup.** `Slayer.cpp`/`Vampire.cpp`/`Ousters.cpp`
  share large duplicated blocks; factor shared behavior toward
  `PlayerCreature` or free functions as formulas from 3.3 make the
  differences explicit.
  > **Status:** not started
  - Owner: R6 ratchet; formula tests.

**Phase exit criteria:** every GM command behind the router with declared
gating; `Zone.cpp` under 2,000 lines.

---

## Phase 5 — Process scaffolding (runs alongside all phases)

- [ ] **5.1 Upgrade server CLAUDE.md to sidecar format.** From build manual to
  "non-obvious rules and gotchas that cost time to rediscover", each rule
  pointing at the test that owns it. Add the `make test` loop, the ratchet
  rules, and this document's status conventions.
  > **Status:** not started

- [ ] **5.2 `.claude/skills/add-packet` skill.** Modeled on sidecar's
  `add-sidecar-domain`: the checklist for adding/changing a packet — layout
  inventory regenerated in *both* repos, golden fixture added, shuffle
  branches covered, handler registered at the composition root (never on the
  packet), client-repo counterpart commit linked.
  > **Status:** not started

- [ ] **5.3 Fix log.** When restructuring uncovers real bugs (1.4 layout
  diffs, races, double-frees), record them in `docs/FIXES.md` with sidecar's
  `> **Status:**` convention rather than fixing silently.
  > **Status:** not started

---

## Phase 6 — CI (deliberately last; blocked on GitHub Actions minutes)

Everything above runs locally by design. When Actions minutes are available
again:

- [ ] **6.1 Build + test workflow.** Extend beyond the current
  `format-check.yml`: debug build + `make test` (unit tier; no DB). Note
  sidecar's hard-won Actions economics: CI on master pushes only, PRs verified
  locally before opening, no jar/binary artifacts uploaded per-run.
  > **Status:** blocked (no Actions minutes)

- [ ] **6.2 Integration tier in CI.** MySQL service container + `initdb/`
  schema for the repository integration tests.
  > **Status:** blocked (no Actions minutes)

---

## Appendix — measured inventory (2026-08-29)

- ~502k LOC across 4,271 C++ files. `Core` 149k (1,410 packet-prefixed files
  in its root), `gameserver` 120k, `skill` 103k / 1,031 files, `item` 51k,
  `quest` 23k (Lua-integrated), plus legacy `chinabilling` / `theoneserver`.
- 433 packet types in `src/Core/Packet.h`
  (`grep -cE 'PACKET_(GC|CG|CL|LC|GL|LG|GS|SG|GG)[A-Z_]* *[,=]' src/Core/Packet.h`).
- Wire encryption: per-session encrypt code reorders field read/write order
  via `SHUFFLE_STATEMENT_*` (`src/Core/EncryptUtility.h`) — part of the
  contract, must be covered by golden fixtures.
- Client repo carries divergent hand-copies of all packet classes
  (`client/Client/Packet/{Gpackets,Cpackets,Lpackets,Rpackets,Types,Upackets}`);
  server `Core` handlers still contain `#ifdef __GAME_CLIENT__` vestiges of
  the shared origin.
- Existing `test*` directories are ad-hoc standalone test servers, not unit
  tests; current CI is clang-format only.
- Sidecar reference (local clone at `../sidecar`): module split
  `sidecar-kernel` ← `sidecar-core` ← `sidecar-app`; enforcement vocabulary in
  `sidecar-kernel/src/test/java/.../kernel/arch/ArchitectureRules.java`;
  conventions in its `CLAUDE.md`.
