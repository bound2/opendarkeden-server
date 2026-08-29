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
# Build and run the wire-contract test suite (local only — no CI tier yet)
make test
```

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
- The `TestPackets` library compiles the whole packet set with NO server-type
  macro (note: not the client's config, which defines __GAME_CLIENT__=1) —
  handlers must keep their server logic behind
  `#ifdef __GAME_SERVER__` (etc.) so the wire layer stays buildable alone.
- `docs/RESTRUCTURING.md` is the living restructuring plan; update task
  `> **Status:**` lines in the same commit as the work.

## Project Architecture

This is the **DarkEden** game server - an MMORPG server written in C++11.

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
- C++11 standard is used
- Threaded architecture with `ZoneGroupThread` for parallel zone processing
- Extensive use of inheritance (Creature → PlayerCreature → Slayer/Vampire/Ousters)
- Lua scripting is integrated for quest systems (see `quest/luaScript/`)
- Exchange system in `gameserver/exchange/` handles player trading
