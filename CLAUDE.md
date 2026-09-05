# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### Building the project
```bash
# Standard release build
make

# Debug build
make debug

# Clean build artifacts
make clean
```

The project uses CMake. The Makefile wraps CMake commands for convenience:
- `make` - Builds with CMake in Release mode (`-DCMAKE_BUILD_TYPE=Release`)
- `make debug` - Builds with CMake in Debug mode (`-DCMAKE_BUILD_TYPE=Debug`)
- Build output binaries go to `bin/` directory
- Build output libraries go to `lib/` directory

For development, always choose debug build!

### Code formatting
```bash
# Format all C++ code
make fmt

# Check format for modified files only (fast)
make fmt-check

# Check format for all files (slow)
make fmt-check-all
```

The project uses clang-format with a `.clang-format` configuration file.
- Format checking is enforced via GitHub Actions on PRs, and it checks only
  the files a PR **changed** (`git diff origin/master HEAD`), using whatever
  clang-format `ubuntu-latest` installs (18.x as of 2026-08).
- **Do not run bare `make fmt` before committing — the tree is not v18-clean.**
  1,253 `src/` files were formatted with an older clang-format, so `make fmt`
  reformats them all and buries your diff in unrelated churn. Format only what
  you touched:
  ```bash
  git diff --name-only master HEAD | grep -E '\.(cpp|h|hpp)$' | xargs clang-format -i
  ```
- Touching a file that was never v18-formatted (the Exchange handlers were
  merged unformatted) makes CI demand you reformat it. That is expected;
  the reformat lands in your PR.
- `make fmt` / `fmt-check-all` cover `src/` **and** `tests/`, matching what CI
  checks.

### Tests

```bash
# Build and run the contract suite with a C++20-capable compiler AND library
make test

# Same suite, but built inside the container off a local workspace. On a
# Windows host this is the one to use — see "Building in the container" below.
make dev-test
```

Prefer the pinned Zig container. Ubuntu 20.04's distro GCC 9 lacks the
required library facilities; CMake checks `jthread`, `stop_token`, and
stop-aware condition-variable waits at configure time. The C++20 workflow
now runs the Debug suite and production builds on PRs and master pushes.

#### Building in the container

**Do not compile straight off the bind-mounted checkout.** Measured inside
the container, the Windows mount costs ~160x on `stat` and ~145x on reads
versus the container's own filesystem, and since every translation unit opens
dozens of headers the build becomes I/O bound: a full build took ~20 minutes
at ~20% CPU on 8 cores. `tools/devbuild.sh` syncs the build *inputs*
(`cmake/`, `src/`, `tests/`, the top-level CMake/Makefile — ~37 MB) into a container
volume, builds there with Ninja and ccache, and copies only generated test
data back. Same build: **~3.5 minutes at ~95% CPU**, and a no-op rebuild in
seconds instead of minutes.

```bash
make dev-test                      # build wire_tests + ctest
bash tools/devbuild.sh test --record   # re-record goldens, then run
make dev-build                     # all production targets
make dev-shell                     # shell in the workspace
make dev-clean                     # drop the workspace + compiler-cache volumes
```

Needs the image once: `docker build -f Dockerfile.dev -t darkeden-dev .`.
It pins Zig 0.16.0 (Clang 21.1.0) and also carries Ninja, ccache and rsync.
Artifacts live in compiler/target/build-type-specific directories in
the volume, so `bin/` and `lib/` in the checkout are **not** updated by these
targets. `bash tools/devbuild.sh output-dir` prints the active lane's artifact
root.

- The suite (in `tests/`) pins the client/server wire contract: golden byte
  fixtures and loopback round-trips for representative packets, a generated
  wire-layout inventory (`tests/wire-layout.txt`) over every packet factory,
  and the shrink-only ratchets from `docs/RESTRUCTURING.md`
  (`tests/ratchet/ratchets.sh`).
- A failing golden or inventory diff is a **protocol change**: the client
  repo's hand-maintained packet copies must ship the identical change.
  Re-record deliberately with `UPDATE_GOLDENS=1 ./bin/wire_tests`.
- Ratchet numbers only go down. When one drops, tighten the baseline in
  `tests/ratchet/ratchets.sh` AND `docs/RESTRUCTURING.md` in the same commit.
- The whole packet set is compiled once, macro-free, in the `de-kernel`
  library (task 2.4) — the same archive the deployed servers and the tests
  link; membership is `tests/arch/kernel_files.txt`. `TestPackets` is only
  the define-free factory/validator trio on top of it. Kernel files may not
  mention server-type macros or `__COMBAT__` (K2), so the wire layer stays
  buildable — and identical — alone. (Note: this is not the client's
  config, which defines __GAME_CLIENT__=1.)
- `docs/RESTRUCTURING.md` is the living restructuring plan; update task
  `> **Status:**` lines in the same commit as the work.

## Project Architecture

This is the **DarkEden** game server - an MMORPG server written in C++20.

### Server Architecture

The server is split into multiple coordinated processes:

1. **loginserver** - Handles authentication and character selection
2. **sharedserver** - Manages shared data (e.g., guild info) across game servers
3. **gameserver** - The main game logic server (one per world/zone group)

### Build System Structure

- **CMake** is the primary build system (CMakeLists.txt files throughout)
- **Legacy Makefiles** exist in subdirectories but are superseded by CMake
- Source files are organized by module under `src/`

### Key Directory Structure

```
src/
├── Core/                      # Core library - shared utilities, no server-type dependencies
│   ├── Packets/               # Protocol packet definitions (GC, CG, CL, LC, GL, LG, GS, SG, GG)
│   └── [core utilities]       # Socket, datagram, player info, items, skills, etc.
├── server/
│   ├── database/              # Database abstraction layer and connection management
│   ├── gameserver/            # Main game server executable
│   │   ├── skill/             # Skill system module
│   │   ├── item/              # Item system module
│   │   ├── billing/           # Billing/payment module
│   │   ├── war/               # War system module
│   │   ├── couple/            # Couple/party system module
│   │   ├── mission/           # Mission system module
│   │   ├── ctf/               # Capture the flag module
│   │   ├── quest/             # Quest system (with Lua scripting)
│   │   ├── mofus/             # Game events module
│   │   └── exchange/          # Player exchange/auction system
│   ├── loginserver/           # Login server executable
│   └── sharedserver/          # Shared server executable
└── Core/CMakeLists.txt        # Defines packet libraries and Core library
```

### Packet System

Packets are the primary communication mechanism between servers and clients. They are organized by direction:

- **GC** (Game → Client): Server sends to client
- **CG** (Client → Game): Client sends to game server
- **LC** (Login → Client): Login server sends to client
- **CL** (Client → Login): Client sends to login server
- **GL** (Game → Login): Game server communicates with login server
- **LG** (Login → Game): Login server communicates with game server
- **GS** (Game → Shared): Game server communicates with shared server
- **SG** (Shared → Game): Shared server responds to game server
- **GG** (Game → Game): Inter-game-server communication

Each packet type typically has two files:
- `PacketName.cpp` - Packet class definition
- `PacketNameHandler.cpp` - Handler that processes the packet

### Preprocessor Macros

Key compile definitions that control behavior:
- `__GAME_SERVER__` - Compiled for gameserver
- `__LOGIN_SERVER__` - Compiled for loginserver
- `__SHARED_SERVER__` - Compiled for sharedserver
- `__COMBAT__` - Enables combat-related code

### Configuration

Server configurations are in `conf/`:
- `gameserver.conf` - Game server configuration
- `loginserver.conf` - Login server configuration
- `sharedserver.conf` - Shared server configuration

Important settings:
- `HomePath` - Repository directory path (must be set correctly)
- `DB_HOST` - Database IP address
- `LoginServerIP` - Login server IP

**Note**: Database `WorldDBInfo` and `GameServerInfo` tables must match config file settings.

## Database Setup

The project requires MySQL 5.7 or 8 with specific SQL mode settings:

```sql
-- Remove NO_ZERO_DATE and STRICT_TRANS_TABLES from sql_mode
set @@global.sql_mode = 'ONLY_FULL_GROUP_BY,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';
```

Databases:
- `DARKEDEN` - Main game database
- `USERINFO` - User account database

Load schema with:
```bash
mysql -h 127.0.0.1 -u elcastle -D 'DARKEDEN' -p < initdb/DARKEDEN.sql
mysql -h 127.0.0.1 -u elcastle -D 'USERINFO' -p < initdb/USERINFO.sql
```

## Dependencies

Required libraries:
- **libmysqlclient-dev** (5.7) - MySQL client library
- **lua5.1-dev** or **luajit** - Lua scripting (used by quest system)
- **xerces-c** (3.2.3) - XML parsing (used by SXml in Core)

Install on Ubuntu/Debian:
```bash
sudo apt install libxerces-c-dev libmysqlclient-dev liblua5.1-dev
```

## Key Game Concepts

### Races
- **Slayer** - Human vampire hunters
- **Vampire** - Vampire race
- **Ousters** - Another playable race

### Core Game Systems
- **Zone/ZoneGroup** - Geographic areas where players exist
- **Creature** - Base class for all entities (players, monsters, NPCs)
- **PlayerCreature** - Player-controlled creatures (Slayer, Vampire, Ousters)
- **Effect** - Time-based effects applied to creatures
- **Skill** - Combat and utility abilities
- **Guild/Party** - Social grouping systems
- **DynamicZone** - Instanced content (e.g., dungeons)

## Thread ownership

The gameserver's threading contract, as the code actually implements it
(task 3.4 of `docs/RESTRUCTURING.md`). This documents the EXISTING design;
known violations are listed at the end, not silently fixed.

### Threads in the gameserver process

- **Main thread** — runs `GameServer::init()` (single-threaded startup:
  config, DB, zone loading), then `GameServer::start()`, which spawns the
  threads below and finally becomes the `ClientManager::run()` infinite
  loop: accepting client TCP connections and driving the pre-zone
  login/handshake phase before a player is handed to a zone group.
- **`ZoneGroupThread` (one per `ZoneGroup`,** via
  `ThreadManager`/`ThreadPool`) — the owner of all zone-group state. Its
  loop is: lock the group's mutex → `ZoneGroup::processPlayers()` (socket
  `select`, read, parse, `PacketDispatcher::dispatch` of **CG** packets)
  and `ZoneGroup::heartbeat()` (NPC/monster AI, effects, zone systems) →
  unlock. So all CG handler code runs on the zone thread **with the group
  mutex held**. Each zone thread registers its own DB `Connection` keyed
  by thread id (`DatabaseManager::addConnection(Thread::self(), …)`) — DB
  connections are thread-local by convention, never shared.
- **`LoginServerManager` thread** — UDP datagram link to the loginserver;
  dispatches **LG** and **GG** packets on its own thread under its own
  `m_Mutex`.
- **`SharedServerManager` thread** — TCP link to the sharedserver;
  dispatches **SG** packets on its own thread under its own `m_Mutex`.
- **`BillingPlayerManager`, `MPlayerManager` (mofus), `GDRLairManager`** —
  auxiliary threads with their own loops. (`SMSServiceThread` exists but is
  not started, and `CBillingPlayerManager`'s start is compiled out —
  `__CONNECT_CBILLING_SYSTEM__` is commented out in
  `chinabilling/CBillingInfo.h` — so it never runs either.)

The gameserver workers above use `ManagedThread` (`std::jthread`). Start and
stop are serialized; stop-before-start is terminal, and join allows a
concurrent stop request. Derived destructors must stop/join before destroying
members. SIGTERM/SIGINT request process shutdown; main exits its client loop,
requests every worker to stop, and joins them while dependencies remain alive.
The process then uses `_Exit` to reclaim the legacy singleton graph without
running its unaudited destructors. This does not add a world-save operation.
A 30-second watchdog forces a nonzero exit if startup, I/O, or a heartbeat
prevents shutdown. See `docs/TOOLCHAIN.md` for the full contract.

### The mutation rule

**Zone-group state (Zones, Creatures in them, the group's
`ZonePlayerManager`) may only be touched while that group's mutex is
held.** The group's own `ZoneGroupThread` holds it for its entire tick;
any other thread must take it explicitly, e.g.
`__ENTER_CRITICAL_SECTION((*(pZone->getZoneGroup())))` — `GDRLairManager`
does this at most (not all — see Known violations) of its zone-mutation
sites. Note `Zone::m_Mutex` is a **different, narrower** lock some
main-thread heartbeats take (war/ctf via `pZone->lock()`); holding it does
NOT exclude the zone-group tick and does not satisfy this rule.

This is mutex-guarded ownership, not pure thread-affinity: the guarded
region is the contract. Under `DE_OWNERSHIP_CHECKS` — defined only for
Debug builds; this project deliberately never defines `NDEBUG`, so the
checks ride their own macro and every optimized build compiles them away
completely — `ZoneGroup::lock()` records the holding thread
(`pthread_equal` + a valid flag, never a raw `==` or zero-tid sentinel)
and `ZoneGroup::assertOwned()` **calls `abort()`** on a violation. It
must not throw: an `AssertionError` is a `Throwable`, and the
`catch (Throwable&)` blocks sitting on these very paths would swallow it,
turning a detected race into a silently half-applied mutation. The check
is armed by `ZoneGroupThread::run()`, so single-threaded startup/loading
is exempt. Coverage is exactly the five `Zone` gateways
`addPC` (both overloads)/`addCreature`/`deleteCreature`/`moveCreature`;
`Zone::movePC`/`deletePC`/`pushPC`/`addItem`/`deleteItem` and direct
`Tile` writes are **not** gated — the assert is a tripwire on the main
gateways, not a full guarantee.

### Cross-thread communication

- Cross-thread packet handlers (SG/LG/GG, on the manager threads) reach
  player creatures through `g_pPCFinder` under **its** critical section
  (`getCreature_LOCKED`), then use `pPlayer->sendPacket(...)` — sending
  to a player's socket is the main legitimate cross-thread operation.
- Players enter a zone group through the `ZonePlayerManager` under its
  lock; the zone thread integrates them on its next tick.

### Known violations (documented, not yet fixed)

- SG/LG/GG handlers **mutate** creature/guild state (e.g.
  `SGAddGuildMemberOKHandler` rewrites guild membership on the
  `SharedServerManager` thread) holding only the `PCFinder` lock. The
  `PCFinder` lock serializes lookup/removal, but NOT against the zone
  thread concurrently mutating the same creature under the group mutex.
  Long-standing data race; fixing it means routing these mutations
  through the owning group's mutex or a per-group command queue (Phase 3
  work).
- `EventMorph.cpp` mutates `Tile` contents directly
  (`tile.addCreature(...)`) below the `Zone` gateways, so the ownership
  assert cannot see such call sites — the assert covers the gateway
  methods only.
- **Cross-group `ZoneGroup::addZone()` race**: `DynamicZone.cpp` (reached
  from `CGSelectWayPointHandler` / `ActionEnterQuestZone` on the
  *requesting player's* zone thread) inserts the new zone into the
  **template zone's** group — generally a different group — while that
  group's own thread iterates `m_Zones` in its heartbeat
  (`unordered_map` rehash-during-iteration). Ungated by the assert
  (`addZone` is not a gateway); the fix is a deferred handoff to the
  owning thread, and simply taking the target group's mutex risks a
  lock-ordering deadlock while the caller holds its own group's.
- `GDRLairManager` locks correctly at most sites but not all:
  `GDRLairIcepole::start`, `GDRLairScene6::start` (iterates the zone's
  PCManager and registers objects) and `GDRLairEnding::start` mutate zone
  state from the GDR thread without the group mutex. None hits a gated
  gateway, so the assert stays blind to them.
- ~~Packets pipelined behind `CGReady` drained on the main thread after
  `GPS_NORMAL` opened the validator gate, reaching the gateways with no
  group mutex~~ — **fixed**: `GamePlayer::processCommand` stops the
  main-thread (IncomingPlayerManager) drain once the status flips to
  `GPS_NORMAL`; the zone thread's `ZonePlayerManager` drains the rest on
  its next tick.

## Running the Servers

Start servers in this order:
```bash
./bin/loginserver -f ./conf1/loginserver.conf
./bin/sharedserver -f ./conf1/sharedserver.conf
./bin/gameserver -f ./conf1/gameserver.conf
```

## Development Notes

- Source file encoding is **UTF-8** (project was migrated from legacy encodings)
- Use **English** as code comment, there are some legacy Korean or maybe garbled encoding, translate them to English whenever possible
- C++20 is the required project language standard, verified with the pinned
  Zig/Clang container toolchain.
- Threaded architecture with `ZoneGroupThread` for parallel zone processing
- Extensive use of inheritance (Creature → PlayerCreature → Slayer/Vampire/Ousters)
- Lua scripting is integrated for quest systems (see `quest/luaScript/`)
- Exchange system in `gameserver/exchange/` handles player trading
