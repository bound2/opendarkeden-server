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
- A status line records the **current** state and what the next reader
  needs to act: conventions, open bugs, what remains. The per-change
  narrative (what moved, what review found, test lists, ratchet deltas)
  belongs in the PR description and commit message, not here.
- A task is only `done` when its **Owner** exists — the test/mechanism that
  keeps the rule true from then on. Landing the change without the owner is
  `in progress (owner missing)`.
- Ratchet numbers (below) may only go **down**. Re-measure with the given
  command before and after a change that claims progress; commit the updated
  number with the change.
- Run the suite locally before every push (`make dev-test` with the pinned
  toolchain). The C++20 workflow also runs the suite and production builds on
  master pushes/merges only; PRs are verified locally to conserve minutes.
  DB-backed integration CI remains deferred.

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
| R1 | `g_p*` global-singleton extern declarations | 327 | `grep -rE '^extern .*\* g_p' src --include='*.h' --include='*.cpp' \| wc -l` (332→331 on 2026-09-10 with the never-built `EventMonsterNameManager.h`, which redeclared `g_pMonsterNameManager`; 331→327 on 2026-09-10 with the never-built `EventBall.h` (two) and the commented-out `EffectBloodyWallLoader` and `EffectGrayDarknessLoader` declarations; a `default*Repository()` accessor is a function, not a global, so extractions do not move this number) |
| R2 | Files with inline SQL in gameserver root | 0 | `grep -lE 'executeQuery' src/server/gameserver/*.cpp src/server/gameserver/*.h \| wc -l` (non-recursive on purpose: a `repository/` MySQL impl does not count — R2 measures SQL *leaving the game logic*. Textual, so a commented-out `executeQuery` still counts. Baseline 104 on 2026-08-29; 7→0 on 2026-09-10, the last two live sites into `PlayRecordRepository::logPlayerTrade` and the new `SMSMessageRepository`, `CreatureUtil.cpp`'s commented-out `addOlympicStat` body deleted, and four never-built stale copies deleted with it. The root is clean; new SQL there fails the ratchet.) |
| R3 | Files with inline SQL outside `database/` and any `repository/` | 0 | `grep -rlE 'executeQuery' src --include='*.cpp' \| grep -v 'server/database' \| grep -v '/repository/' \| wc -l` (18→11 on 2026-09-10 with the seven gameserver-root files R2 counted; 11→0 the same day with the never-built `EventBall.cpp`, the `*notice` command that held the last live statement, and the nine files whose only `executeQuery` sat inside a comment block. `gameserver/repository/` joined the exclusion on 2026-09-01, 317→314: a seam that quarantines four tables from two files would otherwise *raise* a shrink-only ratchet; the loginserver's, sharedserver's and ServerCore's `repository/` directories were admitted on 2026-09-07 before they existed, so the count did not move. Textual — see the comment policy under 3.2. Counts unbuilt files and the other binaries' game logic too.) |
| R4 | Packet headers with `execute()` still on the packet | 0 | `grep -rlE 'void execute\(Player' src/Core --include='*.h' \| wc -l` |
| R5 | `__BEGIN_TRY` control-flow macro sites in de-core candidates | 5,719 | `grep -rE '__BEGIN_TRY' src/server/gameserver --include='*.cpp' \| grep -vE 'gameserver/(handler\|packetfill)/' \| wc -l` (handler/ and packetfill/ hold 2.4-moved sources from `src/Core`, never counted while they lived there; fold in with a re-baseline when they become 3.x extraction targets. 5,984→5,980 on 2026-09-02: the four macros inside the guild trio's deleted dead __SHARED_SERVER__ blocks. 5,980→5,899 on 2026-09-02, textual: ItemIDRegistry.cpp's 81 hand-expanded initItemIDRegistry bodies collapsed onto one macro, so the grep sees one #define line instead of 82 matched lines — 81 expansions plus the old macro's own; each method still has its try block. 5,897→5,790 on 2026-09-05: the never-built `gameserver/test/`, `testAlone/`, `mofus/testserver/` and `quest/Squest/` trees were deleted. 5,790→5,788 on 2026-09-08: the never-built `skill/Restore2.cpp`, a stale duplicate of `skill/Restore.cpp`, was deleted. 5,788→5,755 on 2026-09-08: the never-built `Vampire_backup.cpp`, a stale copy of `Vampire.cpp`, was deleted. 5,755→5,737 on 2026-09-10: the never-built `EventMonsterNameManager.cpp` (4), `GameServerInfoManager.cpp` (7) and `GameWorldInfoManager.cpp` (7) were deleted. 5,737→5,719 on 2026-09-10: the never-built `EventBall.cpp` (10) and `EventQuestRewardManager.cpp` (1) were deleted, and seven more sat in commented-out or empty bodies deleted from `mission/`, `skill/` and `war/`) |
| R6 | Line count of god files (each tracked separately) | see table below | `wc -l <file>` |
| R7 | Files using parenthesized `throw(...)` syntax — dynamic specifications plus expressions, see 5.4 | 0 | `grep -rlE 'throw[[:space:]]*\(' src --include='*.h' --include='*.cpp' \| wc -l` (real throw expressions were normalized to `throw expr`, making every future match unambiguously forbidden legacy syntax) |
| R8 | Non-comment lines using `__PRETTY_FUNCTION__` | 0 | `grep -rh '__PRETTY_FUNCTION__' src --include='*.h' --include='*.cpp' \| grep -vcE '^[[:space:]]*//'` (call-site diagnostics take the enclosing function from a defaulted `std::source_location` — see docs/TOOLCHAIN.md, "Diagnostics without location macros". Line-based: a line whose first non-blank text is `//` is a comment, so the comments that explain the equivalence may still name the macro) |
| R9 | Hand-written length-prefixed string reads left in `src/Core` | 0 | `grep -rhE 'iStream\.read\([A-Za-z_][A-Za-z0-9_]*, sz[A-Za-z0-9_]*\);' src/Core --include='*.h' --include='*.cpp' \| grep -vcE '^[[:space:]]*//'` (a string field is a BYTE length then that many bytes; `de::wire::readString`/`writeString` in `src/Core/WireString.h` carry it with the bounds stated once. The read may land in a member or in a local, so any identifier counts. Line-based with R8's comment rule, so `WireString.h`'s own example of the shape it replaces does not count itself) |
| R10 | Throws of a pointer into a local string, and the handlers that caught one | 0 (R10a), 0 (R10b) | R10a: `grep -rnE 'throw [A-Za-z_]+\.c_str\(\)' src \| wc -l` — a `throw x.c_str()` hands the handler storage that dies with the clause it came from; `END_DB`/`END_DB_EX` answer a failed statement with a `DatabaseError` (`src/server/database/DatabaseError.h`) that owns its message, so nothing in the tree has that shape. R10b: `grep -rn 'catch (const char\*' src \| wc -l` — the receiving end. All 34 handlers name `DatabaseError`, so the count is 0 and one left behind would either be dead or be reaching for one of the ~160 bare `throw "literal"` sites, which are a separate defect and are not answered this way. Textual, so a commented-out clause counts (the one inside `CGPortCheckHandler.cpp`'s commented-out retry moved with the live code) |

God-file baselines (R6):

All rows re-measured 2026-08-31 post-clang-format-18 (the 08-29 numbers
predated that pass) and again 2026-09-05; only the rows `ratchets.sh` names
are enforced so far.

| File | Baseline lines |
|------|---------------:|
| `src/server/gameserver/Zone.cpp` | 9,263 (9,297 on 2026-08-31) |
| `src/server/gameserver/skill/SkillUtil.cpp` | 6,739 (enforced by `ratchets.sh` R6a) |
| `src/server/gameserver/InitAllStat.cpp` | 4,803 (was 4,949 before the 3.3 bonus-formula extraction; enforced by `ratchets.sh` R6b) |
| `src/server/gameserver/handler/CGSayHandler.cpp` (moved from `src/Core` in 2.4) | 4,904 (4,905 on 2026-08-31) |
| `src/server/gameserver/Slayer.cpp` | 4,068 (4,375 on 2026-08-31) |
| `src/server/gameserver/skill/SkillFormula.cpp` | 820 (was 3,081 before the 3.3 computeOutput extraction — now thin adapters + the 11 dice-roll formulas; enforced by `ratchets.sh` R6d) |
| `src/server/gameserver/skill/HitRoll.cpp` | 774 (not a god file — an extraction-target pin, locked in with its 3.3 extraction; enforced by `ratchets.sh` R6c) |

Once Phase 1's test harness exists, encode R1–R8 as **ratchet tests**: the
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
  > **Status:** done — GoogleTest via FetchContent (1.12.1 at the time;
  > v1.18.0 since the 5.4 C++20 migration), `tests/` +
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
  > golden and header-width assertions. **2026-08-30: every encrypter
  > path is pinned** — `tests/packet_encrypter_test.cpp` covers all 19
  > `readEncrypt`/`writeEncrypt` users (17 shuffle packets, with the
  > abstract `GCAddItemToZone` pinned through its three concrete
  > subclasses, plus the unshuffled `CGAddMouseToZone`/`CGDropMoney`) at
  > encrypt codes 0..5, which reach every `code % N` case of every
  > `SHUFFLE_STATEMENT_N`; the shuffle tables themselves are asserted as
  > literals, and `ratchets.sh` fails on any new encrypter user without
  > goldens (`tests/ratchet/encrypter_exceptions.txt` for the base
  > class). The **CL/LC direction is pinned** — all 33 login-phase
  > packets have code-0 goldens, loopback round trips and
  > size/factory-max pins (`tests/packet_login_test.cpp`); the three
  > write/read disagreements it found are fixed and pinned as the
  > refusals they now produce (`PCSlayerInfo::write` lets its empty-name
  > refusal escape like its two sibling records, so `LCPCList` can no
  > longer emit a body shorter than the size it declares;
  > `LCRegisterPlayerOK`'s group name is capped at `maxNameLength` and
  > refused when empty; `LCServerList` / `LCWorldList` refuse an entry
  > past the count their factory max budgets).
  > The **gameserver handshake is pinned** — the 12 packets that cross
  > the gameserver's client socket between the TCP connect and
  > `GPS_NORMAL`, on the fresh-login and the zone-transfer path
  > (`CGConnectSetKey`, `CGConnect`, `CGReady`, the two hot-key packets
  > the `GPS_WAITING_FOR_CG_READY` gate whitelists, `CGVerifyTime`,
  > `GCSystemAvailabilities`, `GCUpdateInfo` with one golden per race,
  > `GCPetInfo` with and without a pet, `GCSetPosition`, `GCDisconnect`,
  > `GCReconnectLogin`), have code-0 goldens, loopback round trips and
  > size/factory-max pins
  > (`tests/packet_gameserver_handshake_test.cpp`); the write/read
  > disagreements it found are fixed and pinned as the behaviour they
  > now produce (`PCSlayerInfo2` and `SubItemInfo` let the exceptions
  > their `read`/`write` raise escape, and every `PCInfo2` record caps
  > its guild name at 30 in the setter, so the PC record can no longer
  > underflow the size `GCUpdateInfo` declares; `NPCInfo::getSize`
  > counts only the fields `write()` emits; `EffectInfo::getMaxSize`
  > covers a full 255-effect list, which is the one wire-layout move in
  > the set — nine `GCAdd*`/`GCUpdateInfo` max sizes grow by 766 bytes;
  > `InventoryInfo` / `GearInfo` / `ExtraInfo` / `RideMotorcycleInfo`
  > derive the count they put on the wire in `addListElement` and no
  > longer expose `setListNum`; the NPC record list, the blood bible
  > signs and the nickname stop at the widths their max sizes budget;
  > `GCUpdateInfo`, `GCPetInfo` and `NicknameInfo` initialise every
  > member and `GCUpdateInfo::read` allocates the blood bible sign
  > record). `GCReconnect` is excluded: no server source constructs it.
  > The **zone population scan is pinned** — the 18 packets
  > `Zone::addPC` and the `Zone::scan()` it triggers send to build a
  > player's view of a zone, plus the ones that later add or remove one
  > object from it (`GCAddSlayer`, with a second golden for the
  > pet-less / nickname-less / closed-store shape, `GCAddVampire`,
  > `GCAddOusters`, `GCAddMonster`, `GCAddBurrowingCreature`,
  > `GCAddBat`, `GCAddWolf`, `GCAddNPC`, the four corpse packets,
  > `GCAddEffect`, `GCAddEffectToTile`, `GCAddVampirePortal`,
  > `GCDeleteObject`, `GCDeleteEffectFromTile`, `GCFastMove`), have
  > code-0 goldens, loopback round trips and size/factory-max pins
  > (`tests/packet_zone_scan_test.cpp`); the write/read disagreements it
  > found are fixed and pinned as the behaviour they now produce
  > (`PCSlayerInfo3::read`/`write` let their refusals escape, so the PC
  > record can no longer underflow the size the packet declares;
  > `GCAddEffect::read` consumes exactly what `write` emits;
  > `PCVampireInfo3::getMaxSize` and `PCOustersInfo3::getMaxSize` count
  > the four-byte alignment field their `getSize` puts on the wire,
  > which is the one wire-layout move in the set — seven
  > `GCAdd*`/`GCMorphVampire2` max sizes grow by four bytes;
  > `PCVampireInfo3` refuses a coat type that does not fit the wire byte
  > rather than dropping its high half; the monster name, shop sign, pet
  > nickname, portal owner and NPC name stop at the widths their max
  > sizes budget, and `GCNPCInfo` caps its record list at the 255 its
  > count byte carries; the corpse packets initialise their treasure
  > count, the creature-add packets survive a missing effect record, and
  > none of them frees the pet or nickname record the creature owns).
  > `GCSetPosition` is covered by the handshake pins,
  > `GCAddNewItemToZone` and `GCAddInstalledMineToZone` by the encrypter
  > pins.
  > The **inter-server directions are pinned** — all 30 packets the three
  > server processes say to each other, on both links: the
  > gameserver/loginserver UDP link (the four `GL*`, the four `LG*`, the
  > three `GG*` and `GMServerInfo`) and the gameserver/sharedserver TCP
  > link (the eight `GS*` and ten `SG*` guild packets), have code-0
  > goldens, round trips and size/factory-max pins
  > (`tests/packet_interserver_test.cpp`), with extra goldens for the
  > absent-introduction branches, the empty guild table and the empty
  > zone table. The datagram half is pinned through a real `Datagram`,
  > whose header carries a *measured* body length, so the size field, the
  > datagram's length and `getPacketSize()` are checked against each
  > other; the socket hop itself stays in
  > `tests/datagram_frame_test.cpp`. Both ends of every one of the 30
  > are built from this repository, so a fix there is free to move
  > bytes. The **write/read disagreements this set found are fixed** and
  > pinned as the behaviour they now produce (`GGGuildChat::read` and
  > `GGServerChat::read` bound the message length against the message
  > instead of the *sender*, so a declared length past the 128 `write`
  > emits is refused; the guild introduction in `GSAddGuild`,
  > `GSModifyGuildIntro`, `SGAddGuildOK`, `SGModifyGuildIntroOK` and
  > `GuildInfo2` is cut to `GUILD_INTRO_MAX_LENGTH` in the setter and
  > refused past it in `write`, in place of guards that compared a
  > `BYTE` against 255 and 256; `SGGuildInfo` and `GuildInfo2` read
  > their lists back in the order `write` sent them; `SGGuildInfo`
  > refuses a table past the 500 guilds its factory max budgets, in
  > `addGuildInfo`, `write` and `read` alike; `GuildInfo2::getMaxSize`
  > counts the member-count word once, the one wire-layout move in the
  > set — `SGGuildInfo` 2916502 → 2915502, a server-side read-buffer
  > budget and not a field on the wire; and `GMServerInfo::read`
  > replaces the zone table it already holds instead of appending to
  > it). No golden changed.
  > The **social protocols are pinned** — the 47 party, guild and trade
  > packets a client and a game server exchange: the ten party packets,
  > the twenty-five registered guild packets and the twelve
  > face-to-face trade packets have code-0 goldens, loopback round trips
  > and size/factory-max pins (`tests/packet_party_test.cpp`,
  > `tests/packet_guild_test.cpp`, `tests/packet_trade_test.cpp`), with
  > extra goldens for the absent-introduction branches, the
  > guild-channel chat shape, the parameterless NPC response, the empty
  > rosters, tables and offer lists, and the traded item with neither an
  > option nor a sub-item. `CGExpelGuildMember`, `CGQuitGuild` and
  > `GCShowGuildRegist` are excluded: no server registers a factory for
  > them, so there is no wire contract to pin; the four `Exchange`
  > packets are covered by `tests/packet_exchange_test.cpp`. The
  > thirteen write/read disagreements it found are fixed and pinned as
  > the behaviour the packets now produce (`GuildInfo::getSize` counts
  > the expiry date and the length byte in front of it, so
  > `GCActiveGuildList` declares the body it sends; `GCActiveGuildList`,
  > `GCGuildMemberList`, `GCWaitGuildList` and `GCShowWaitGuildInfo`
  > read their lists front to back, in the order `write()` sends them;
  > `GCActiveGuildList::getListNum` and `GCWaitGuildList::getListNum`
  > return the wire's WORD count; the guild roster, the union offer
  > list, the party roster and `GCTradeAddItem`'s two lists refuse an
  > entry past the count their factory max budgets, with
  > `GuildMemberInfo::getMaxSize` now one record and
  > `GCUnionOfferList`'s max budgeting its count byte — the two
  > wire-layout moves in the set, `GCGuildMemberList` 5064 → 5502 and
  > `GCUnionOfferList` 1180 → 1181, both server-side read-buffer
  > budgets and not fields on the wire; the guild and member
  > introductions are cut to `GUILD_INTRO_MAX_LENGTH` in their setters
  > and refused past it in `write()`; the party names and chat messages
  > are held to the widths their factory maxima budget; `GCGuildChat`'s
  > sending guild name is refused empty or past 20, like its sender and
  > its message; `GCGuildResponse::getCode` and
  > `GCNPCResponse::getCode` return the WORD they hold; and
  > `GCTradeAddItem`'s sub-item count is derived from the list, with
  > `setListNum` gone). `GCModifyGuildMemberInfo` and `GCOtherGuildName`
  > round trip too: both `read()`s read the guild-name length byte
  > before testing it rather than testing an uninitialised one. Two
  > packets joined the file with the fix, `GCWaitGuildList` and
  > `GCShowWaitGuildInfo`, which carry the same guild record and had the
  > same reversal. No golden changed.
  > The **combat feedback set is pinned** — the 38 packets a game server
  > sends as the result of an attack or a skill use, to the actor, the
  > target and the observers, plus the status packets that ride along
  > (`GCAttack`, `GCGetDamage`, the three `GCAttackMeleeOK*`, the five
  > `GCAttackArmsOK*`, the six `GCSkillToObjectOK*`, the three
  > `GCSkillToSelfOK*`, the six `GCSkillToTileOK*`, the two
  > `GCSkillToInventoryOK*`, `GCSkillFailed1`, `GCSkillFailed2`,
  > `GCStatusCurrentHP`, `GCModifyInformation`, `GCOtherModifyInfo`,
  > `GCCreatureDied`, the three `GCThrowBombOK*` and the two
  > `GCMineExplosionOK*`), have code-0 goldens, loopback round trips and
  > size/factory-max pins (`tests/packet_combat_test.cpp`), with extra
  > goldens for the empty stat record, the refusal that changed nothing,
  > the tile sweep that caught nobody, and a record carrying every one of
  > the 73 `ModifyType` tags as both a short and a long entry — the
  > embedded `ModifyInfo` is a pair of counted type/value lists rather
  > than a flag word, so covering the tags covers the record.
  > `GCAddEffect` is covered by the zone-scan pins; `GCRemoveEffect`
  > belongs to effect expiry, and the attack path constructs neither.
  > The six write/read disagreements it found are fixed and pinned as
  > the behaviour the packets now produce (`ModifyInfo` derives each of
  > its two list counts from the list and refuses the entry past the 255
  > its count byte carries, in the adders and in `write()`, so the 256th
  > entry can no longer wrap the count while `write()` still emits it;
  > the five tile packets that carry a creature list bound theirs the
  > same way, `setCListNum` is gone and `popCListElement()` takes the id
  > off the count with the list; each tile factory max budgets a full
  > 255-id list, the five wire-layout moves in the set —
  > `GCSkillToTileOK1` 2060 → 3076, `GCSkillToTileOK2` 2061 → 3077,
  > `GCSkillToTileOK4` 270 → 1286, `GCSkillToTileOK5` 274 → 1290 and
  > `GCSkillToTileOK6` 2059 → 3075, all server-side read-buffer budgets
  > and not fields on the wire; `ModifyInfo::read()` and the tile
  > `read()`s replace the list the packet holds instead of appending to
  > it; a type tag past the last `ModifyType` is refused in the adders
  > and in `read()`, and `modifyType2String()` prints an unknown tag as
  > its number; and `GCSkillToTileOK3::getObjectID()` /
  > `GCSkillToInventoryOK2::getObjectID()` return the `ObjectID_t` they
  > hold). The two the review recorded rather than tested are fixed with
  > them and testable now: every packet in the set initialises every
  > member its `write()` emits, pinned by constructing each over poisoned
  > storage, and the two hit flags are read as a `BYTE` and narrowed
  > rather than loaded as a `bool`. `GCActiveGuildList`,
  > `GCWaitGuildList` and `GCShowWaitGuildInfo`'s founding-member list,
  > which the social set left uncapped, refuse an entry past the count
  > their factory maxima budget with them. No golden changed.
  > Five packets joined the file after it, `GCThrowBombOK1`/`2`/`3` and
  > `GCMineExplosionOK1`/`2`: the thrown bomb's and the stepped-on
  > mine's answers carry the same creature list the tile packets do,
  > with the same wrap and the same one-`ObjectID` budget, and are
  > bounded the same way. Their five wire-layout moves —
  > `GCThrowBombOK1` 2055 → 3071, `GCThrowBombOK2` 2058 → 3074,
  > `GCThrowBombOK3` 271 → 1287, `GCMineExplosionOK1` 2054 → 3070 and
  > `GCMineExplosionOK2` 267 → 1283 — are read-buffer budgets, not
  > fields on the wire; their goldens are new files.
  > The **movement, effect-lifecycle and NPC dialogue set is pinned** —
  > the 26 packets that carry a step, end an effect or run an NPC
  > conversation: `GCMove`, `GCKnockBack`, `GCFakeMove`, the six
  > unburrow and untransform packets (`CGUnburrow`, `GCUnburrowOK`,
  > `GCUnburrowFail`, `CGUntransform`, `GCUntransformOK`,
  > `GCUntransformFail`), the four transition packets that redraw the
  > creature as it reappears (`GCAddMonsterFromBurrowing`,
  > `GCAddMonsterFromTransformation`, `GCAddVampireFromBurrowing`,
  > `GCAddVampireFromTransformation`), `GCRemoveEffect`,
  > `GCRemoveInjuriousCreature`, `GCHPRecoveryEndToSelf`,
  > `GCHPRecoveryEndToOthers`, `GCMPRecoveryEnd`, `GCCannotUse`,
  > `CGNPCTalk`, `GCNPCAsk`, `GCNPCAskDynamic`, `GCNPCAskVariable`,
  > `GCNPCSay`, `GCNPCSayDynamic` and `GCNPCInfo`, have code-0 goldens,
  > loopback round trips and size/factory-max pins
  > (`tests/packet_movement_test.cpp`), with extra goldens for the
  > nameless monster and its absent effect record, the vampire
  > transition carrying no effect record, the sweep that removed no
  > effect, the script that substitutes nothing, the question with no
  > choices and the nameless NPC record. `CGMove`, `GCMoveOK`,
  > `GCMoveError` and `CGNPCAskAnswer` are covered by the encrypter
  > pins; `GCFastMove`, `GCSetPosition`, `GCAddEffect`,
  > `GCAddEffectToTile`, `GCDeleteEffectFromTile`, `GCDeleteObject`,
  > `GCAddBurrowingCreature` and `GCAddNPC` by the zone-scan and
  > handshake pins; `GCNPCResponse` by the social pins.
  > `GCKnocksTargetBackOK1`/`2`/`4`/`5` are excluded: they have
  > registered factories but no server source constructs one. The ten
  > write/read disagreements it found are fixed and pinned as the
  > behaviour the packets now produce (`GCNPCAskDynamic` counts the
  > contents byte in `getPacketSize()`, derives the choice count from
  > the list and refuses the choice past `kMaxCount` in the adder, in
  > `write()` and in `read()` — 15, the most `SCRIPT_MAX_CONTENTS`
  > lets a script carry and what its factory max now budgets, where the
  > old budget of ten was already under the twelve the shipped scripts
  > reach — and keeps on `read()` the empty choice `write()` emits as a
  > bare length word; `GCNPCSayDynamic`'s message and
  > `ScriptParameter`'s name and value go through
  > `de::wire::readString`/`writeString` at the 255 their length byte
  > can describe, so `GCNPCAskVariable` measures each parameter whole
  > and no string reaches the wire behind a wrapped length;
  > `GCRemoveEffect` derives its count from the list, refuses the id
  > past the 255 the count byte carries, `read()` replaces the list
  > instead of appending to it, and its maximum budgets a full list; the
  > two monster transition packets hold the name to the 32 bytes their
  > max budgets, the width `GCAddMonster` holds the same record to; and
  > `DIR_NONE` stays part of the contract for the packets that broadcast
  > a creature's direction, because `MonsterAI` computes it for a
  > monster already standing on its destination and the move mask has an
  > entry for it, while `CGUnburrow` and `CGMove` refuse a direction
  > outside the eight from a client). The three the review recorded
  > rather than tested are fixed with them and testable now: every
  > packet in the set initialises every member its `write()` emits,
  > pinned by constructing each over poisoned storage; `~GCNPCInfo`
  > destroys the records `read()` allocated while still only dropping
  > the ones a filler handed it; and the four transition packets'
  > `read()` frees the `EffectInfo` it replaces. Two `tests/wire-layout.txt`
  > lines move with them, `GCRemoveEffect` 255 → 515 and
  > `GCNPCAskDynamic` 11294 → 16425, both server-side read-buffer
  > budgets and not fields on the wire. No golden changed.
  > The **inventory and item handling set is pinned** — the 37 packets
  > the item handlers exchange to move, use, make, repair and throw an
  > item: the twenty client requests (`CGAddGearToMouse`,
  > `CGAddMouseToGear`, `CGAddInventoryToMouse`,
  > `CGAddMouseToInventory`, `CGAddMouseToQuickSlot`,
  > `CGAddQuickSlotToMouse`, `CGAddItemToItem`, `CGAddItemToCodeSheet`,
  > `CGMixItem`, `CGMakeItem`, `CGThrowItem`, `CGThrowBomb`,
  > `CGReloadFromInventory`, `CGReloadFromQuickSlot`,
  > `CGUsePotionFromQuickSlot`, `CGUseItemFromGQuestInventory`,
  > `CGUseMessageItemFromInventory`, `CGRequestRepair`,
  > `CGGetEventItem`, `CGRequestNewbieItem`) and the seventeen answers they
  > draw (`GCCannotAdd`, `GCCreateItem`, `GCDeleteInventoryItem`,
  > `GCDeleteandPickUpOK`, `GCUseOK`, `GCAddItemToItemVerify`,
  > `GCReloadOK`, `GCRemoveFromGear`, `GCAddGearToInventory`,
  > `GCAddGearToZone`, `GCMakeItemOK`, `GCMakeItemFail`, the three
  > `GCThrowItemOK` packets,
  > `GCGQuestInventory`, `GCTimeLimitItemInfo`), have code-0 goldens,
  > loopback round trips and size/factory-max pins
  > (`tests/packet_inventory_test.cpp`), with extra goldens for the item
  > that carries no options, the empty stat record, the empty material,
  > item, time-limit and creature lists, and the parameterless and
  > two-parameter shapes of the drop-one-item-on-another result.
  > `CGUseMessageItemFromInventory` inherits the encrypter from
  > `CGUseItemFromInventory` without calling it, so it is pinned here at
  > codes 0..5; the family's other encrypter users are covered by
  > the encrypter pins, the `PCItemInfo` / `SubItemInfo` /
  > `InventoryInfo` / `GearInfo` / `ExtraInfo` item records by the
  > handshake pins and `StoreInfo` by the zone-scan pins.
  > `GCModifyMoney` and `GCSubInventoryInfo` are excluded: no registered
  > factory and no source outside `src/Core` mentions them.
  > `GCAddItemToInventory` and `GCChangeInventoryItemNum` have no packet
  > id of their own and are pinned through `GCMakeItemOK` and
  > `GCMakeItemFail`, the only packets that put them on the wire.
  > The fourteen write/read disagreements it found are fixed and pinned
  > as the behaviour the packets now produce
  > (`GCAddItemToInventory::write()` emits its option count once, the
  > side the client's own copy of the record reads, and
  > `getPacketSize()` counts the item count behind it, so `GCMakeItemOK`
  > declares the body it sends and round trips; the record's option list
  > and `GCCreateItem`'s are held to `MAX_ITEM_OPTION_NUM` (30) in the
  > adder, the setter, `write()` and `read()` — the widest option list an
  > item carries, a code sheet's stone grid, where every other class
  > holds at most three — and `GCCreateItem`'s max budgets it;
  > `GCAddItemToItemVerify::getPacketSize()` counts both parameters of a
  > THREE_ENCHANT_OK result and the constructor sets the second one;
  > `GCChangeInventoryItemNum`'s count is its list, refused past the 255
  > its count byte carries in the adder and in `write()`, with
  > `setChangedItemListNum` gone, popping taking the entry off both
  > lists, `read()` replacing what the packet holds, and both crafting
  > maxima built from the two embedded records' own maxima so a full
  > material list fits; `GCGQuestInventory` holds its item list to the
  > 100 its max budgets in `addItem()`, in `write()` and in `read()`,
  > and `read()` replaces the list instead of appending to it; and every
  > one of the 37 initialises every member its `write()` emits, pinned
  > over poisoned storage). The one the test's header recorded rather
  > than tested is fixed with them:
  > `GCTimeLimitItemInfo::getTimeLimit()` returns `std::optional<DWORD>`
  > with `hasTimeLimit()` beside it, in place of an 0xffff a real
  > remaining time can equal. Three `tests/wire-layout.txt` lines move,
  > `GCCreateItem` 277 → 52, `GCMakeItemFail` 2297 → 3318 and
  > `GCMakeItemOK` 2552 → 3363, all server-side read-buffer budgets and
  > not fields on the wire. One golden pair moves deliberately:
  > `GCMakeItemOK`'s two goldens are one byte shorter, the duplicated
  > option count the client never read.
  > The **store, shop and stash dialogue is pinned** — the 36 packets a
  > client and a game server exchange over a shop counter: the eighteen
  > client requests (`CGStoreOpen`, `CGStoreClose`, `CGStoreSign`,
  > `CGDisplayItem`, `CGUndisplayItem`, `CGBuyStoreItem`,
  > `CGRequestStoreInfo`, `CGShopRequestList`, `CGShopRequestBuy`,
  > `CGShopRequestSell`, `CGStashList`, `CGStashDeposit`,
  > `CGStashWithdraw`, `CGStashRequestBuy`, `CGMouseToStash`,
  > `CGStashToMouse`, `CGDepositPet`, `CGWithdrawPet`) and the eighteen
  > answers they draw (`GCMyStoreInfo`, `GCOtherStoreInfo`,
  > `GCAddStoreItem`, `GCRemoveStoreItem`, `GCShopList`,
  > `GCShopListMysterious`, `GCShopBought`, `GCShopBuyOK`,
  > `GCShopBuyFail`, `GCShopSellOK`, `GCShopSellFail`, `GCShopSold`,
  > `GCShopVersion`, `GCShopMarketCondition`, `GCStashList`,
  > `GCStashSell`, `GCPetStashList`, `GCPetStashVerify`), have code-0
  > goldens, loopback round trips and size/factory-max pins
  > (`tests/packet_store_test.cpp`), with extra goldens for the item
  > that carries no options, the empty rack, stash and pet-stash
  > listings, the stall written closed to its owner and to a passer-by,
  > the empty stall window, the empty sign and the pet slot holding no
  > pet. No packet in the family calls the encrypter or derives from one
  > that does, so every golden is recorded at code 0 and each test also
  > asserts the bytes do not vary with the code. The `StoreInfo` /
  > `StoreOutlook` records are covered by the zone-scan pins and
  > `PCItemInfo` / `SubItemInfo` / `PetInfo` by the handshake pins; the
  > answers these handlers share with other families (`GCCannotAdd`,
  > `GCCreateItem`, `GCDeleteandPickUpOK`, `GCDeleteObject`,
  > `GCModifyInformation`, `GCNoticeEvent`, `GCSystemMessage`,
  > `GCNPCResponse`) are pinned in the inventory, zone-scan, combat,
  > handshake and social files. `GCShopList` had a frame-size pin in
  > `tests/packet_roundtrip_test.cpp` but no golden and gets one here.
  > The **nine write/read disagreements it found are fixed** and
  > pinned as the behaviour the packets now produce
  > (`GCShopBuyFail` declares the fail code and four-byte amount it
  > sends behind the NPC id, and its max budgets all three, where both
  > counted the id alone and every refusal overran the buffer sized for
  > it by five bytes; `CGStoreSign`'s sign goes through the `de::wire`
  > helpers at the 80 its max budgets, refused past that in the setter
  > too, so its length byte cannot wrap, and an empty sign is admitted,
  > so the bare zero length byte `write()` emits reads back; `StoreInfo`
  > cuts its sign at the 80 its max size budgets, the way `StoreOutlook`
  > already did, and bounds it there on the wire, so a full stall hits
  > the `GCMyStoreInfo` and `GCOtherStoreInfo` maxima exactly;
  > `GCShopBought`, `GCShopBuyOK`, `GCShopList` and `GCStashList` hold
  > their option lists to `MAX_ITEM_OPTION_NUM`, the widest an item
  > carries, in the adders, the setters, `write()` and `read()`, and
  > their maxima budget it instead of 255 options; the same four replace
  > the list a packet already holds on every read, with `GCStashList`
  > destroying the sub-item records it drops; `GCPetStashList` carries
  > the twenty slots and nothing else, the shape the client's own reader
  > takes, so the code byte no sender could deliver is gone with its
  > `setCode` calls; and `GCStashList`'s sub-item count for each slot is
  > the list the records come from, held to the eight belt pockets the
  > max budgets, so a sub-item added through `getSubItems()` is counted,
  > declared and sent). The two the poisoned-storage pin recorded are
  > fixed with them: all 36 initialise every member their `write()`
  > emits, and `GCMyStoreInfo` and `GCOtherStoreInfo` start with an
  > empty record pointer and refuse in `getPacketSize()` and `write()`
  > rather than follow it. Six `tests/wire-layout.txt` lines move —
  > `GCPetStashList` 1021 → 1020, `GCShopBought` 284 → 59,
  > `GCShopBuyFail` 4 → 9, `GCShopBuyOK` 287 → 62, `GCShopList`
  > 5515 → 1015 and `GCStashList` 21006 → 7506 — all read-buffer
  > budgets and not fields on the wire. Two entry counts the inventory
  > round left near-missed are bounded with them and pinned in
  > `tests/packet_inventory_test.cpp`: `GCTimeLimitItemInfo`'s 100
  > entries and `CGTypeStringList`'s 20 strings, each the number its own
  > max already budgets, so neither max moves. Three findings are
  > recorded in the test's header rather than fixed: the three list
  > `read()`s index their fixed slot arrays with a rack or slot byte off
  > the wire and never bound it, `GCShopBuyFail::read()` takes a code
  > byte it never compares against `GC_SHOP_BUY_FAIL_MAX`, and
  > `GCPetStashList` frees neither the slot it overwrites on a re-read
  > nor the `PetInfo` inside a slot it destroys. No golden changed.
  > The **character progression set is pinned** — the 29 packets a
  > client and a game server exchange while a character advances: the
  > nine client requests (`CGLearnSkill`, `CGDownSkill`,
  > `CGCastingSkill`, `CGSkillToNamed`, `CGUseBonusPoint`,
  > `CGUsePowerPoint`, `CGRequestPowerPoint`, `CGSelectRankBonus`,
  > `CGSelectBloodBible`) and the twenty answers they draw
  > (`GCSkillInfo`, `GCTeachSkillInfo`, `GCLearnSkillOK`,
  > `GCLearnSkillFailed`, `GCLearnSkillReady`, `GCDownSkillOK`,
  > `GCDownSkillFailed`, `GCCastingSkill`, `GCUseBonusPointOK`,
  > `GCUseBonusPointFail`, `GCUsePowerPointResult`,
  > `GCRequestPowerPointResult`, `GCRankBonusInfo`,
  > `GCSelectRankBonusOK`, `GCSelectRankBonusFailed`,
  > `GCSweeperBonusInfo`, `GCHolyLandBonusInfo`, `GCBloodBibleList`,
  > `GCBloodBibleStatus`, `GCBloodBibleSignInfo`), have code-0 goldens,
  > loopback round trips and size/factory-max pins
  > (`tests/packet_skill_test.cpp`), with one golden per race for the
  > skill book and extra goldens for the book with no record and the
  > record with no skill, the empty rank-bonus, sweeper, holy-land,
  > blood-bible and sign listings, the full sign list, the stat record a
  > spent bonus point produces with nothing in it, and the blood bible
  > that belongs to nobody. No packet in the family calls the encrypter
  > or derives from one that does, so every golden is recorded at code 0
  > and each test also asserts the bytes do not vary with the code. The
  > four aimed-skill requests these handlers also answer
  > (`CGSkillToObject`, `CGSkillToSelf`, `CGSkillToTile`,
  > `CGSkillToInventory`) are covered by the encrypter pins, the
  > `ModifyInfo` stat record and the `GCSkillFailed*` / `GCSkillTo*OK*`
  > results by the combat pins, and the `BloodBibleSignInfo` record by
  > the handshake pins. `GCPetUseSkill` is excluded: it has a registered
  > factory, but its only sender is `Pet.cpp`'s attack path, which
  > belongs to combat feedback rather than to progression. `GCSkillInfo`
  > had an ousters frame-size pin and `GCBloodBibleStatus` a name-width
  > pin in `tests/packet_roundtrip_test.cpp`; both are single-aspect and
  > neither had a golden, so both packets are pinned in full here.
  > The **nine write/read disagreements it found are fixed** and pinned
  > as the behaviour the packets now produce (a book carries one record
  > per skill domain, refused past that in `addListElement`, in `write()`
  > and in `read()`, so its count byte cannot wrap; the factory max
  > budgets the pc type byte, the record count byte and one full record
  > per domain, where it was one bare `SlayerSkillInfo` that a single
  > full record already outgrew; the three race records emit the skill
  > count their list holds and stop at the 255, 120 and 120 their own
  > maxima budget, with `setListNum` gone from all three and from the
  > `Slayer` / `Vampire` / `Ousters` fills that maintained it;
  > `GCSkillInfo::read` replaces the book the packet holds, destroying
  > the records it drops, and `GCRankBonusInfo`, `GCSweeperBonusInfo`,
  > `GCHolyLandBonusInfo` and `GCBloodBibleList` replace their lists the
  > same way; `GCSkillInfo` announces only the three pc types `read()`
  > builds a record for, refusing every other in the setter, in `write()`
  > and in `read()`, testing the raw byte before it reaches an enum;
  > those three bonus lists and `GCBloodBibleList` are held to the 100,
  > 12, 12 and 12 entries their factory maxima budget, in the adders, in
  > `write()` and in `read()`; `SweeperBonusInfo` and
  > `BloodBibleBonusInfo` are the race byte the client's own readers take
  > and nothing else, so the type that reached no wire is gone with its
  > accessors and the two `SweeperBonusManager` calls; and
  > `GCBloodBibleSignInfo`'s record pointer starts empty, with
  > `getPacketSize()` and `write()` refusing on it). All 29 initialise
  > every member their `write()` emits, pinned over poisoned storage.
  > Three of the four the test's header recorded are fixed with them:
  > `GCBloodBibleSignInfo` frees on a re-read and in its destructor only
  > the record `read()` allocated, so a sender still keeps the
  > character's own record it hands over; `BloodBibleSignInfo::read`
  > replaces the signs it holds and refuses a count past the six slots
  > `write()` emits; and each race record's learn flag and a slayer
  > skill's enable flag travel as a `BYTE` held to 0 or 1 before it
  > reaches a `bool`, which moves no byte because a sender writes a real
  > bool. The fourth stays recorded: the two power-point results take
  > code bytes they never compare against the enumerators their own
  > headers declare. One `tests/wire-layout.txt` line moves,
  > `GCSkillInfo` 4338 → 34706, a server-side read-buffer budget for a
  > packet no server reads and not a field on the wire. Four raw string
  > literals `__END_CATCH` could not catch are `InvalidProtocolException`s
  > with them — `GCShopList::getShopItem`,
  > `GCShopListMysterious::getShopItem` and `GCShopVersion`'s two version
  > accessors — pinned in `tests/packet_store_test.cpp`. No golden
  > changed.
  > The **quest, war and zone-selection set is pinned** — the 27 packets
  > a client and a game server exchange around a quest, a war, the flag
  > war, a castle's tax and the places a player picks: the thirteen
  > client requests (`CGSelectQuest`, `CGFailQuest`, `CGGQuestAccept`,
  > `CGGQuestCancel`, `CGSubmitScore`, `CGModifyTaxRatio`,
  > `CGWithdrawTax`, `CGDonationMoney`, `CGSelectRegenZone`,
  > `CGSelectPortal`, `CGSelectWayPoint`, `CGSelectTileEffect`,
  > `CGRelicToObject`) and the fourteen answers they draw
  > (`GCQuestStatus`, `GCSelectQuestID`, `GCMonsterKillQuestInfo`,
  > `GCGQuestStatusInfo`, `GCGQuestStatusModify`, `GCMiniGameScores`,
  > `GCWarList`, `GCWarScheduleList`, `GCFlagWarStatus`, `GCNotifyWin`,
  > `GCNoticeEvent`, `GCRegenZoneStatus`, `GCEnterVampirePortal`,
  > `GCAddHelicopter`), have code-0 goldens, loopback round trips and
  > size/factory-max pins (`tests/packet_quest_war_test.cpp`), with
  > extra goldens for the empty quest, kill-quest, general-quest,
  > score, war and war-schedule listings, the quest record carrying no
  > mission, the score table written past the ten `write()` caps it at,
  > the war schedule whose five challengers and reinforcement are all
  > nameless, and the notice code that carries no parameter. No packet
  > in the family calls the encrypter or derives from one that does, so
  > every golden is recorded at code 0 and each test also asserts the
  > bytes do not vary with the code. The answers these handlers share
  > with other families (`GCNPCResponse`, `GCModifyInformation`,
  > `GCAddEffect`, `GCRemoveEffect`, `GCDeleteObject`, `GCCannotAdd`,
  > `GCCreateItem`, `GCDeleteInventoryItem`, `GCAddVampire`,
  > `GCSweeperBonusInfo`) are pinned in the social, combat, zone-scan,
  > inventory and progression files; `GCAddHelicopter`, which the
  > zone-scan set deferred as waypoint travel, is pinned here.
  > `GCSystemMessage`, `GCSay` and `GCItemNameInfoList` are excluded:
  > generic in-game text and the zone's item-name listing, sent from
  > 138, 10 and 3 server sources across every family, and belonging to
  > the chat set rather than to quests or wars. `GCNotifyWin` had a
  > name-width pin in `tests/packet_roundtrip_test.cpp` and no golden,
  > as the `GuildWarInfo` and `MissionInfo` records had a width pin
  > each; all three are pinned in full here. Fifteen open write/read
  > disagreements are stated as tests that flip when fixed
  > (`GCRegenZoneStatus::read` appending its eight bytes behind the
  > eight the constructor already pushed, so the packet cannot round
  > trip at all; `GCMiniGameScores::getPacketSize` never advancing its
  > iterator, so it counts the first name's length once per entry and a
  > table of uneven names declares a size it does not send; the same
  > packet deriving each name's length byte with no bound while its max
  > budgets twenty bytes for one; `GCNoticeEvent::getCode` returning a
  > BYTE of the WORD it puts on the wire, and its
  > `setParameter(WORD, WORD)` assigning `makeDWORD` of the two halves
  > to the code instead of the parameter; `GCGQuestStatusInfo`,
  > `GCWarList`, `GCWarScheduleList`, `QuestStatusInfo`'s mission list
  > and the `ValueList` inside a guild and a race war each deriving a
  > count byte they cap nowhere, so the 256th entry wraps it to zero
  > while `write()` emits every one; those two packets and
  > `GCSelectQuestID`, `GCMonsterKillQuestInfo` and
  > `GCGQuestStatusInfo` appending to the list they already hold
  > instead of replacing it; `GCWarList`'s max budgeting twelve race
  > wars and twelve guild wars and neither the count byte nor the war
  > type byte in front of each record, so that very listing outgrows
  > it; `GCWarScheduleList`'s guild names not being held to the sixteen
  > its max budgets; `GCGQuestStatusModify` leaving uninitialised the
  > record pointer `getPacketSize()` and `write()` dereference; and
  > `MissionInfo::write` printing every mission to standard output on
  > the wire path), and 18 of the 27 leave at least one member the
  > default constructor never sets, pinned over poisoned storage. Six
  > more are recorded in the test's header rather than tested:
  > `GCWarList::read` casting the war type byte to the `WarType` enum
  > before switching, so any byte past 3 is an out-of-range enum load;
  > `GCFlagWarStatus` indexing its three-slot array with an unbounded
  > `Race_t`; `GCGQuestStatusInfo` freeing no record (the loop is
  > commented out) and `GCGQuestStatusModify::read` allocating one per
  > call and freeing none; `GCSelectQuestID` and
  > `GCMonsterKillQuestInfo` refusing a list past 255 through
  > `Assert()`, which writes `assertion_failed.log` before it throws;
  > `GCAddHelicopter`'s code getter being named `setCode`; and
  > `GCNoticeEvent` taking a code it never compares against
  > `NOTICE_EVENT_MAX`. No golden changed.
  > With CL/LC, the gameserver handshake, the zone population scan, both
  > inter-server links, the social protocols, the combat feedback set,
  > the movement, effect-lifecycle and NPC dialogue set, the inventory
  > and item handling set, the store, shop and stash dialogue, the
  > character progression set and the quest, war and zone-selection set
  > pinned, the remaining non-encrypter coverage outstanding is the rest
  > of GC/CG, whose largest unpinned group is the chat and notice
  > packets (`GCSystemMessage`, `GCSay` and their siblings).
  > **Adversarial review (2026-08-29) named the specific gaps, in
  > priority order:**
  > 1. ~~Only 2 of the 17 encrypter-using packets are pinned~~ — closed
  >    2026-08-30 as above. Pinning them surfaced three read/write/size
  >    disagreements, each stated as a fact by a test that flips when it
  >    is fixed (record the fix in `docs/FIXES.md`, task 5.3):
  >    - ~~`GCDropItemToZone::read()` consumes a leading `BYTE flag`
  >      that `write()` no longer emits (commented out) and
  >      `getPacketSize()` does not count~~ — **fixed 2026-09-09** with
  >      the social-protocol set: `read()` starts at the object id, the
  >      commented-out write is gone, and the packet round trips at
  >      every encrypt code. `GCCreatureDied`, `GCMove`, `GCNPCSay` and
  >      `GCSkillFailed2` carried the same shape and are fixed with it.
  >      `write()` never emitted the byte, so the goldens are unchanged
  >      and the client copy needs no change.
  >    - `CGUseItemFromInventory` and `CGSkillToInventory`:
  >      `getPacketSize()` counts `m_InventoryItemObjectID`, which
  >      `read()`/`write()` skip — over-reports by `szObjectID`. Dormant on
  >      the live wire only because the server never *writes* CG packets;
  >      verify the client copies do not share the bug in 1.4.
  >      **Fixed 2026-08-31** in the 1.4 max-size reconcile (the client
  >      copies never had the bug); the `..._WITH_SIZE_DRIFT` fact-tests
  >      flipped as designed and were retired.
  > 2. For the 459 unpinned packets, `getPacketMaxSize()` is invariant
  >    under field reordering and same-width type changes — e.g. swapping
  >    the two shuffle arguments in both `GCMoveError::read`/`write` passes
  >    the whole suite while transposing X/Y on every odd encrypt code.
  > 3. Test values in the pinned packets are all < 128, so a signedness
  >    flip on `Coord_t`/`Dir_t` yields byte-identical goldens. Prefer
  >    values ≥ 128 and distinct per field when adding packets. (The
  >    encrypter pins added 2026-08-30 follow this; the four original
  >    fixtures still use small values.)
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

- [x] **1.4 Share the inventory with the client repo.** Drop the same
  generator + committed inventory into `client` (its packet copies have
  already diverged textually — verified on `GCMoveOK.cpp`, 135 differing
  lines — while layouts are believed equal). First run produces the
  first-ever ground-truth diff of the two protocol copies; any layout
  mismatch found is a **bug to triage immediately**, recorded here.
  > **Status:** done — `wire_inventory_diff.sh` exits 0 (2026-08-31).
  > The Exchange reconcile (finding 1) merged in both repos
  > (`restructuring/exchange-reconcile`); the remaining 17 max-size
  > mismatches (findings 2–4 below) were fixed in
  > `restructuring/wire-maxsize-reconcile` in both repos — each side's
  > `getPacketMaxSize()` corrected to the true wire layout (details in
  > `docs/FIXES.md`). Notable resolutions: CGBloodDrain was *not* a live
  > layout mismatch — the client's X/Y/Dir were already commented out of
  > read()/write(), only its factory max was stale; LCPCList's trailing
  > `m_Agree` byte exists only under `__NETMARBLE_SERVER__` (never
  > defined), so the size accounting is now guarded the same way; the
  > client's GCUseOK cap is now `ModifyInfo::getPacketMaxSize()` (2042)
  > instead of a hardcoded 255 that dropped large use results. The
  > CGUseItemFromInventory / CGSkillToInventory phantom
  > `m_InventoryItemObjectID` over-report from 1.2 is fixed with it
  > (the two `..._WITH_SIZE_DRIFT` fact-tests flipped and were retired).
  > History of the triage that got here:
  > *(was)* in progress — inventories in both repos, cross-check
  > script, first diff triaged (2026-08-30). Client side:
  > `client/tests/unit/test_wire_layout.cpp` + `tests/wire-layout.txt`
  > (436 factories), generated by `client/tests/tools/gen_wire_inventory.pl`
  > — the client cannot link its packet `.cpp`s (handlers pull in the
  > game), so the generator lifts each factory's `getPacketID()` /
  > `getPacketMaxSize()` body into a plain function and the compiler
  > evaluates it against the real headers; a `wire_inventory_fresh`
  > ctest pins the generated file. Server side:
  > `tests/tools/wire_inventory_diff.sh [client-dir]` diffs the two
  > inventories (documented one-sided packets in
  > `tests/wire-layout-exceptions.txt`). **It exits 1 until the findings
  > below are fixed — that is the point.** Findings, by severity:
  > 1. **Packet-ID shift from 485 up — the Exchange feature cannot work
  >    on the wire.** The client's `Packet.h` has `PACKET_GC_USE_SKILLCARD_OK`
  >    at 485 (class `GCUseSkillCardOK` in `Gpackets/GCUseOK.h`); the
  >    server has no such enumerator, so every id after it differs by one:
  >    client `CGExchangeList` = 486 = server `CG_EXCHANGE_CREATE_LISTING`,
  >    and so on through `GCExchangeClaimList`. The `// 484` comments in
  >    both files are stale and hid this. **Fixed on the server side
  >    2026-08-30:** `PACKET_GC_USE_SKILLCARD_OK` restored to `Packet.h`
  >    at 485 (the server had dropped the packet and its enumerator
  >    together; the client kept both), the id comments corrected, the
  >    inventory re-recorded (`CGExchangeList` 486, `CGExchangeBuy` 489 —
  >    now equal to the client's). The client's `// 484` comments are
  >    still stale; fix them there.
  >    Doing this exposed more in the Exchange block:
  >    - The server's `GCExchangeList` / `GCExchangeBuy` had **no factory
  >      class** (the only GC packets without one), so they were invisible
  >      to the inventory. Factories added; their max sizes now compare.
  >    - `GCExchangeList`: client `getPacketMaxSize()` is a hardcoded
  >      2048; the server's write() layout is 14 + 1,855 per listing
  >      (strings at their 255 max) — 37,114 at the default page of 20.
  >      Any real listing page over 2048 bytes is dropped by the client
  >      (`Player.cpp:172`). Page size is also client-chosen and
  >      unbounded on the server (`CGExchangeListHandler.cpp:34`).
  >    - `GCExchangeBuy::write()` emits the message with **no length
  >      prefix** (`write(string)` is raw), so no receiver can frame it —
  >      and the client has no `GCExchangeBuy` class at all.
  >    - **The request layouts differ too**, so the feature still cannot
  >      work after the id fix: server `CGExchangeList::read()` consumes
  >      page, pageSize, itemClass, itemType, minPrice, maxPrice **and a
  >      BYTE-length string** (max 77); the client writes only the first
  >      six (19 bytes). Server `CGExchangeBuy::read()` consumes a listing
  >      id **and an idempotency key** (max 72); the client writes a
  >      4-byte listing id only. The two repos' Exchange packets were
  >      written against different specs — reconcile them as one change
  >      in both repos, then the diff's Exchange lines go green.
  >      **Reconciled 2026-08-30** (`restructuring/exchange-reconcile` in
  >      both repos; canonical string encoding = BYTE length always
  >      written + raw bytes, all four factory maxes equal across repos,
  >      pinned by `tests/packet_exchange_test.cpp` round-trips/goldens/
  >      size checks). Fixing the layouts uncovered and removed on the
  >      way:
  >      * server `CGExchangeBuy::read()`/`GCExchangeBuy::read()` called
  >        `iStream.read(std::string&)`, which resolves to the generic
  >        `template read(T&)`. That template's normal-order path is
  >        `buf = *(T*)(m_Buffer + m_Head)` — `std::string::operator=`
  >        against a **fabricated string object overlaid on the wire
  >        buffer**, so the copy follows a pointer and length taken from
  >        attacker-supplied bytes (arbitrary read, not merely a clobber),
  >        on every received buy request. Both now use BYTE-length
  >        encoding, and the call sites carry a warning comment;
  >      * server `GCExchangeList::read()` read `listingID` twice (once
  >        where write() emits `itemID`) and discarded most fields into
  >        locals — rewritten as write()'s exact mirror;
  >      * client `GCExchangeList` parsed only page/pageSize/total and
  >        capped at 2048 bytes; now parses the full 31-field listing
  >        layout with max 37114 = the server's;
  >      * client had no `GCExchangeBuy` class, and `GCExchangeListFactory`
  >        was **never registered** in its `PacketFactoryManager` — the
  >        listing reply was unroutable; both fixed.
  >
  >      Adversarial review of the reconcile branches (2026-08-30, one
  >      reviewer per repo) then found a further set, all fixed in the
  >      same branches:
  >      * **SQL injection.** `escapeSQL()`
  >        (`gameserver/exchange/ExchangeDB.cpp`, 18 call sites) doubled
  >        the single quote and nothing else, so a backslash before a
  >        quote escaped the doubled quote and broke out of the literal.
  >        Reachable by any logged-in player through
  >        `CGExchangeBuy`'s idempotency key, which this very branch had
  >        widened from 64 to 255 bytes;
  >      * **size/body disagreement on long strings.** `write()` truncated
  >        the length byte with `(uint8_t)s.length()` while
  >        `getPacketSize()` counted the untruncated length — and
  >        `SocketOutputStream::writePacket()` emits that size header
  >        *before* calling `write()`, so the peer frames on a wrong
  >        length and the stream desyncs permanently. (For the same
  >        reason, throwing from `write()` the way `CGSay` does is not a
  >        fix here — the header is already out.) Every string field in
  >        both repos now clamps identically in `write()`,
  >        `getPacketSize()` and `read()`, against a named per-field cap;
  >      * the idempotency key's wire cap is 64, not 255, because
  >        `PointLedger.IdempotencyKey` is `VARCHAR(64) UNIQUE`: under
  >        this project's mandated non-strict `sql_mode` a longer key was
  >        silently truncated on insert while the dedupe guard compared
  >        the full key, so the guard passed and the INSERT then hit the
  >        unique index and rolled the purchase back. `CGExchangeBuy`'s
  >        max is therefore 73 in both repos;
  >      * client `GCExchangeList::read()` trusted a server-supplied
  >        `uint16` listing count with no bound — the first client packet
  >        whose byte consumption was not bounded by the declared
  >        `packetSize`, so a hostile count silently ate following
  >        packets. Bounded by `kMaxListingsPerPage` (20), the same
  >        constant the 37114 max is computed from, and the server
  >        handler now clamps the client-chosen page size to it (it went
  >        straight into a SQL `LIMIT` before);
  >      * `CGExchangeBuyHandler` never called `setOrderID()`, so the
  >        order id the client now parses would have been 0 forever;
  >      * `read()` did not reset state (no `m_Listings.clear()`, no
  >        clearing of a string whose length byte is 0), so the two
  >        "mirrors" differed on a reused packet object;
  >      * 64-bit ids were logged through `(int)` casts in `toString()`,
  >        which runs on every packet. Fixing that surfaced a **stack
  >        buffer overflow in `StringStream`** itself: its `long` and
  >        `ulong` `operator<<` both `sprintf("%ld"/"%lu")` into a
  >        `char buf[12]` copy-pasted from the 32-bit `int` overload,
  >        while `long` is 64-bit on the LP64 build target and needs 21
  >        bytes. Any `<<` of a value outside the 32-bit range smashed
  >        the stack; buffers widened to 24 and switched to `snprintf`.
  >        (The exchange packets format their ids independently, so they
  >        never depended on that overload.)
  >      A second adversarial review pass over the fixes returned SHIP with
      no blockers, and found these, fixed in the same branch: the
      `float`/`double` `StringStream` overloads had the *same* buffer
      overflow as `long`/`ulong` (`"%f"` never uses exponent form, so a
      float ≥ 10,000 already overruns `buf[12]`); the server's
      `GCExchangeList::read()` lacked the listing-count bound its client
      twin has; and three comments claimed more than the code delivered
      (the `NO_BACKSLASH_ESCAPES` fallback is safe but *not* "exactly
      right" — it can alter a backslash; the wire cap does not make the
      stored key identical, because the ledger suffix trims it; and the
      fallback key is not yet unique across game servers, since
      `_getServerID()` is a hardcoded 1 and every containerised server is
      pid 1).
      Left as follow-ups, each recorded rather than fixed silently:
      * `ExchangeService::buyListing` checks
        `hasIdempotencyKey(rawKey)`, but only the suffixed `_buy`/`_sale`
        keys are ever inserted, so that early-out is dead code —
        duplicate protection currently rests on `adjustPoints`' own check
        inside the transaction. Combined with the client never setting a
        key, end-to-end dedupe is inert today;
      * `Statement::executeQuery`'s `vsnprintf` guard tests `> 2048`, so
        a query of exactly 2048 characters is silently truncated and
        executed;
      * `ExchangeService::buyListing` leaks the `ExchangeListing` from
        `ExchangeDB::getListing` on every path, and `GamePlayer` leaks a
        packet when `readPacket` throws;
      * `GCExchangeList`'s 37114 max is 4.5× the client's default 8 KB
        socket ring, which only grows opportunistically — worth either
        pre-sizing that ring or lowering the page bound;
      * on success `GCExchangeBuy`'s message field carries the bare
        decimal order id, now redundant with `setOrderID()`;
      * neither repo clamps the listing count in `write()`:
        `(uint16_t)m_Listings.size()` narrows silently while the body
        loop iterates the full vector, so at 65536+ listings the count
        wraps to 0 with the body still emitted and counted — header and
        bytes agree, but `read()` parses fewer listings and desyncs.
        Unreachable only because the handler clamps the page to 20, i.e.
        the invariant lives a layer above the packet. Identical in both
        repos, so it is not a divergence.
      Still open, deliberately out of scope for a layout change:
  >      `CGExchangeListHandler` ignores the `sellerFilter` the packet now
  >      carries (adding it means changing `ExchangeService::getListings`
  >      and its SQL), and the client UI sends `CGExchangeBuy` with an
  >      empty idempotency key (`VS_UI_PointExchange.cpp:436`), so the
  >      server auto-generates one per request and the double-click
  >      dedupe the field exists for is not yet achieved. Both UI send
  >      paths do exist (`VS_UI_PointExchange.cpp:390` and `:436`).
  > 2. **`CGBloodDrain` layout mismatch** — client writes
  >    `ObjectID, X, Y, Dir` (7 bytes); server reads `ObjectID` only and
  >    `getPacketMaxSize()` = 4, so `GamePlayer` would throw
  >    `InvalidProtocolException` (disconnect) on receipt. Latent: the
  >    client's only send site (`MPlayer.cpp:3457`) is commented out.
  > 3. **`GCUseOK` receiver max too small** — client hardcodes 255, server
  >    can write up to `ModifyInfo::getPacketMaxSize()` = 2042. A use
  >    result with more than ~36 modify entries is rejected by the client
  >    (`Player.cpp:172`, bug report + drop). `LCPCList` (client 249 vs
  >    server 250), `GCMorph1` / `GCUpdateInfo` (client 4 smaller: the
  >    client's `PCSlayerInfo2::getMaxSize()` has `; + szExp; + szBonus;`
  >    dead statements after `return`) reject only a maximal packet.
  > 4. **Estimate-only differences, layouts verified identical:**
  >    `GCExecuteElement` (server max 3 for a 7-byte body — server-side
  >    bug, harmless because the server only sends it), `GCNPCResponse`
  >    (server counts `szBYTE` for a `WORD` code), `GCAddMonsterCorpse`
  >    (server counts an extra `szbool`), `GCSubInventoryInfo` (client
  >    `InventoryInfo::getMaxSize()` adds two phantom `szCoordInven`),
  >    `CGSMSSend`, `CLLogin`, `GLIncomingConnectionError`, `LCServerList`,
  >    `LCWorldList` (formula differences, receiver side is the larger
  >    or the packet is not on the client wire). `CGUseItemFromInventory`
  >    / `CGSkillToInventory`: the client already dropped the phantom
  >    `m_InventoryItemObjectID` from its max (6 / 10 = the real body);
  >    the server's still counts it — confirms the 1.2 finding.
  > 5. Read/write field order is NOT compared by the inventory; the
  >    per-packet goldens (1.2) are the only pin for that, and the client
  >    has none yet.
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
  the then-948-file `TestPackets`. Fixed with explicit `=OFF` in those recipes
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

- [x] **2.1 Define the target layering in CMake.**
  - `de-kernel`: packet data classes, socket streams, `Types.h`, encrypt
    utilities, `Packet`/`PacketFactory` base. **No** MySQL, Lua, Zone/Creature
    headers, or server-type `#ifdef`s.
  - `de-core`: game domains (skill, item, quest, war, mission, exchange, …) —
    game rules against kernel types and repository *interfaces* only.
  - Apps: `gameserver`, `loginserver`, `sharedserver` executables — DB
    adapters, network transport, handler wiring, composition root.
  Initially the split is aspirational for existing files; new code must land
  in the right target from day one.
  > **Status:** done (2026-09-01) — the three layers exist as real,
  > linked targets: `de-kernel` (below) is the one wire archive every
  > server links; `de-core` (`src/domain/`, created by 3.3) is so far
  > linked only by the gameserver and holds only the extracted formulas —
  > the domain dirs named above are still gameserver app libraries, and
  > moving them is 3.x extraction work, not part of this task's "define
  > the layering". The file sort for legacy code continues under Phase 3
  > ratchets; layering is enforced by the 2.2 test + the kernel target's
  > pinned include path.
  > History: `de-kernel` became a real CMake target
  > (2026-08-31): a STATIC library whose membership is
  > `tests/arch/kernel_files.txt` (grown from the 57-file seed past a
  > thousand files with 2.4's packet directions and the non-packet
  > utilities — the 2.4 status tracks the exact count; Datagram/
  > SerialDatagram cpps were header-only members until the link flip
  > split their factory-calling receive paths into Core's
  > `DatagramFactoryRead.cpp`, letting the rest of the framing compile
  > in the kernel) and whose only include dir is `src/Core` (pinned as the
  > target's own INCLUDE_DIRECTORIES — the top-level directory include
  > path would otherwise leak `src/server` and MySQL in) — a kernel
  > source reaching for an app header fails to compile. Built in every
  > configuration; at first nothing linked it (apps got the objects
  > through `Core`, which kept its gameserver include leak until
  > 2.3/2.4) — resolved by the 2.4 link flip.
  > Getting the seed macro-free removed four dead `__GAME_CLIENT__`
  > branches from `Types.h`/`CreatureTypes.h`/`Packet.h` (the macro is
  > never defined in this repo; wire tests prove no layout change).
  - Owner: CMake `PRIVATE` include dirs on each target; membership file
    shared with the 2.2 test.

- [x] **2.2 Include-graph architecture test.** A Python script under `tests/`
  (run by `make test`) that parses `#include` edges and fails on forbidden
  ones: kernel → core/app; core → `mysql.h`, socket headers, Lua headers;
  any core/app → kernel-internal detail headers as they get marked. Keep the
  rule list in one file (sidecar's `ArchitectureRules` pattern: extend the
  list deliberately, never weaken a rule to fix a compile error — a violation
  means the class is in the wrong module).
  > **Status:** done — `tests/arch/check_includes.pl` (perl, not python:
  > the dev image has no python3 and the repo's generators are already
  > perl), run by ctest as `arch_includes` (2026-08-31). Rules: K1 a
  > kernel file quote-includes only kernel files (transitive by
  > construction), K2 no server-type macros in kernel files, C1 the
  > gameserver domain dirs (skill/item/quest/war/mission/couple/ctf/
  > mofus/exchange/billing) must not include MySQL, Lua, or
  > socket-transport headers. K rules have no baseline (the list is
  > defined as what complies); C1's 9 pre-existing violations are frozen
  > shrink-only in `tests/arch/baseline.txt` (billing/mofus Socket.h
  > users — all Player-transport classes, 2.3's problem).
  - Owner: the include-graph test.

- [x] **2.3 Strip `execute()` off packets; dispatch table at the composition
  root.** The crux. Today packet classes carry `execute()` → `*Handler` which
  reaches into gameserver internals, with `#ifdef __GAME_SERVER__` /
  `__GAME_CLIENT__` switching (vestige of the once-shared codebase). Replace
  with a per-executable dispatch table (packet ID → handler function)
  registered at each app's composition root; handlers move out of `Core` into
  the app/domain that owns them. Kills the macro switching and is what makes
  `de-kernel` actually framework-free. Migrate direction-by-direction under
  the Phase 1 pin (layout must not change — golden tests prove it).
  Track with ratchet R4 (packets still carrying `execute()`).
  > **Status:** done (2026-08-31; see the closing paragraph below) —
  > history of how it landed: infrastructure + the CG direction landed
  > (2026-08-31). `PacketDispatcher` (kernel: id → `void(*)(Packet*,
  > Player*)` table, written only at startup so zone threads read it
  > lock-free) is consulted first in all five receive loops
  > (GamePlayer, LoginPlayer, SharedServerClient, sharedserver's
  > GameServerPlayer, Core `Player`), falling back to the legacy
  > virtual for unmigrated packets. `Packet::execute` is no longer pure:
  > the default throws `InvalidProtocolException`, so a migrated id
  > received by a server that does not register it now disconnects the
  > sender instead of running a no-op handler — the only intended
  > behavior change, and only for protocol-violating peers.
  > All 150 CG packets are migrated: `execute()` deleted from their
  > headers/cpps (two intermediate bases, `DatagramPacket` and
  > `SerialDatagramPacket`, dropped their pure redeclarations), and
  > `src/server/gameserver/GamePacketDispatch.cpp` binds every CG id at
  > the gameserver composition root (`registerGameServerPacketHandlers()`
  > from `main()`; `CGPortCheck`'s player-less handler and `CGStashList`'s
  > `__BEGIN_DEBUG` wrapper preserved as explicit thunks). R4 481→329.
  > **GC migrated the same day**: all 255 GC handler bodies were
  > preprocessor-classified under the server defines (regex was not
  > enough — the guard vocabulary spans `__GAME_CLIENT__`,
  > `#if __TEST_CLIENT__`, `#elif __WINDOWS__`); exactly one is live
  > server-side, `GCFriendChatting` (the friend system rides this "GC"
  > packet client→server), now registered for real. The live client's
  > store UI also *sends* `GCAddStoreItem`/`GCRemoveStoreItem` (and
  > legacy paths `GCCannotUse`) — registered as explicit ignore-thunks
  > to preserve today's silent no-op, since the validator's `GPS_NORMAL`
  > set is `PIST_ANY` and would otherwise let the new default disconnect
  > a legitimate client; `GC_MY/OTHER_STORE_INFO` were already
  > force-rejected pre-dispatch by `GamePlayer`. `execute()` deleted from
  > all 258 GC packet cpps/headers. R4 329→74.
  > **All remaining directions migrated the same day** (R4 74→1 — only
  > `Packet.h`'s transitional default remains): every direction handler
  > was preprocessor-classified under all three server defines, and each
  > is live on exactly one server. Composition roots:
  > `LoginPacketDispatch.cpp` (16 CL + 4 GL + `GMServerInfo`, all
  > datagram GL riding `GameServerManager`'s socket),
  > `SharedPacketDispatch.cpp` (8 GS), and `GamePacketDispatch.cpp`
  > gains 10 SG + 4 LG + 3 GG (LG/GG arrive on `LoginServerManager`'s
  > datagram socket — GG is game→game UDP, not relayed through shared).
  > Both datagram receive loops are dispatch-first now too. All 17 LC
  > handlers are no-ops on every server (pure delete); `CLAgreement` is
  > netmarble-dead and in no validator whitelist, so it needs no
  > registration; `RCSay`/Upackets/TOpackets are not compiled by any
  > target (client-only or dead subsystems) and were stripped textually.
  > Registration macros live in `PacketDispatcher.h`
  > (`DE_REGISTER_PACKET_HANDLER[_NOPLAYER]`). Found + fixed on the way:
  > `SGModifyGuildMemberOK`'s handler had never run — misspelled
  > `#ifdef __GAME_SERER__` guard (`docs/FIXES.md`).
  > **Closed 2026-08-31** after a live smoke test of all three servers
  > against the real client (login, guild ops, friend chat): `Packet`
  > carries no `execute()` at all, `PacketDispatcher::dispatch` throws
  > `InvalidProtocolException` on an unregistered id, and the seven
  > receive loops call it unconditionally. R4 = 0, held by the ratchet.
  > Handler file moves out of `Core` are 2.4.
  - Owner: R4 ratchet test + include-graph test (a kernel packet including a
    Zone header fails).

- [x] **2.4 Move packet sources into the kernel target.** Once a direction's
  handlers are out (2.3), move those packet files under the `de-kernel`
  target. `Core`'s non-packet utilities get sorted kernel-vs-app as touched.
  > **Status:** done (2026-09-01) — the link flip landed: every kernel
  > .cpp is compiled exactly once, in `de-kernel`, and all three apps
  > plus the tests link that archive. `Core` shrank to the three files
  > the kernel cannot own (`SXml`, `TimeChecker`, and the new
  > `DatagramFactoryRead.cpp` — the first real link of the kernel
  > archive exposed that the GL/LG datagram packets call `Datagram`
  > string read/write whose bodies sat app-side, so the two
  > factory-calling receive paths were split out of `Datagram.cpp`/
  > `SerialDatagram.cpp`, whose remaining pure framing joined the
  > kernel; their already-mojibake `throw Error` strings were translated
  > to English in the move)
  > and links `de-kernel` PUBLIC, so consumers are unchanged; the
  > per-server packet libraries shrank to the three per-server `#if`
  > files (`PacketFactoryManager`/`PacketIDSet`/`PacketValidator`), and
  > the ~520-line hand-kept per-direction source lists are deleted —
  > membership lives in `tests/arch/kernel_files.txt` alone, which
  > `gen_factory_list.sh` (and thus the ratchet freshness check) now
  > reads instead of the CMake lists (regenerated `.inc` differs only in
  > its header comment: same 465 factories, proving the repoint is
  > faithful). The flip is behavior-preserving by construction: K1 means
  > kernel compiles see the same headers the per-server compiles saw, and
  > K2 — extended to ban `__COMBAT__` too, since a macro-conditional in a
  > kernel file would now silently compile as "off" for everyone — means
  > no kernel object ever depended on the per-server defines. Two
  > adversarial reviewers (2026-09-01) verified this empirically:
  > preprocessing all 545 kernel TUs under the old per-server flags and
  > include paths vs de-kernel's produced zero differing TUs in every
  > configuration. Their surviving findings, fixed: a `ratchets.sh`
  > failure message still pointed at the deleted CMake lists; the
  > `wire_tests` link comment described a Core↔TestPackets cycle the
  > Datagram split had just removed; new checker rule **K3** (the packet
  > libraries may define only K2-banned macros — "one meaning" enforced
  > at the definition site, not held by coincidence of today's `-D` set)
  > and a K1 ban on parent-relative includes (which resolve from the
  > including file's directory and could bypass both the checker's
  > basename match and the pinned include path). One tripwire was lost
  > knowingly: the old thin per-server packet archives made a
  > cross-server factory registration a link error; now every executable
  > links all packet objects, so per-server over-registration in
  > `PacketFactoryManager.cpp`'s `#if` blocks would link clean — the
  > per-server validator whitelists remain the runtime gate.
  > Steps that got here:
  > 1. The 271 no-op GC/LC handler files are **deleted** (2.3's
  >    classification proved the server never runs them; the client repo
  >    keeps its own copies), their dangling declarations stripped from
  >    the packet headers.
  > 2. The 197 live handlers moved out of `Core` into per-app
  >    `handler/` dirs (gameserver 168, loginserver 21, sharedserver 8) —
  >    plain app sources bound by the composition roots; the packet
  >    libraries carry only wire classes. Handler class *declarations*
  >    stay in the packet headers for now (no includes behind them; two
  >    lost their `#ifdef __GAME_SERVER__` around member decls, with
  >    `class Item;` forward-declared). The dead `CGAddInjuriousCreature`
  >    pair (no id enum, never in any build) is deleted.
  > 3. **The whole CG direction is kernel**: all 149 CG packet pairs +
  >    `Assert1.h` + `NicknameInfo` joined `tests/arch/kernel_files.txt`
  >    (360 files) after removing a handful of vestigial includes
  >    (`GamePlayer.h`, `ExchangeService.h`, `libcpsso.h` — leftovers of
  >    the removed `execute()`); `de-kernel` compiles them under K1/K2
  >    with zero new baseline entries.
  > 4. **The info classes + most of GC are kernel too** (914 files):
  >    membership computed by fixpoint against the include-graph
  >    checker — every candidate that passes K1/K2 joins; 23 GC packets
  >    stay out because they build wire fields from live game objects
  >    (`Item`/`Skill`/`PetItem` includes — `GCStashList` is the
  >    archetype), and `PetInfo` joins header-only for the same reason.
  >    A second dead pair surfaced and was deleted:
  >    `GCMonsterKillQuestStatus` (id enum never existed, commented out
  >    of every build — same story as `CGAddInjuriousCreature`).
  > 5. **CL/LC and every inter-server direction are kernel** (1,038
  >    files total): the same fixpoint admitted all of CL/LC/GL/LG/GS/
  >    SG/GG/`GMServerInfo` except `CLSelectPC` (includes `Player.h`,
  >    the transport base). A third dead pair fell out: `CLAgreement`
  >    (no id enum — which is why it was in no validator whitelist).
  > 6. **The last held-back wire classes are kernel** (1,098 files): the
  >    23 game-coupled GC packets, `CLSelectPC` and `PetInfo` joined
  >    after their game-object member *definitions* moved to
  >    `src/server/gameserver/packetfill/` (declarations stay in the
  >    headers with the game types forward-declared); GCAttackArmsOK1–5
  >    and GCSkillToTileOK2 instead needed the `SkillTypes` enum + name
  >    table extracted from gameserver's `skill/Skill.h` into
  >    `src/Core/types/SkillTypes.h` (wire vocabulary, not game logic).
  >    `PetInfo::write()` resolves the pet-item ObjectID through a
  >    type-erased thunk the app-side setter installs — still a LIVE
  >    read at write time (an earlier cached-id version shipped stale
  >    ids and asserted on unregistered items; caught in the 2.4
  >    adversarial review), same bytes, no game include. Dead
  >    `__GAME_CLIENT__` branches in five
  >    GC files were removed (this repo never defines the macro; the
  >    client keeps its own copies). R5 scope note: `packetfill/` is
  >    excluded alongside `handler/` (one `__BEGIN_TRY` moved there).
  > 7. **`Core`'s non-packet utilities are sorted** (1,121 files):
  >    Geometry, Shape, HashMap, VSTemplateLib, ValueList, SlotInfo, the
  >    WarInfo family, Assert1.h, Datagram/SerialDatagram,
  >    `Player.{h,cpp}` and the Update/Resource
  >    families all joined on the first fixpoint pass. Never-compiled
  >    `SlotInfo.cpp` lost its stale `throw()` specs; dead-on-arrival
  >    `AttributeListPacket` deleted. Held out by design:
  >    `PlayerStatus.h`/`PacketIDSet`/`PacketValidator`/
  >    `PacketFactoryManager.cpp` (per-server `#if` is their purpose),
  >    `TimeChecker` (server Timeval), `SXml` (tinyxml2 binding),
  >    `libcpsso.h` (billing SSO), `Rpackets`/`Upackets`/`TOpackets`
  >    — all three relic packet dirs are now **deleted** (`Upackets`/
  >    `TOpackets` with the dead `ClientManager.cpp` phone-home beacon;
  >    `Rpackets` in the follow-up, closing `factory_exceptions.txt` to
  >    zero entries). The two dead server trees that still referenced
  >    the deleted headers, `theoneserver/` and `updateserver/`, were
  >    deleted on 2026-09-05 (see "Legacy service cleanup").
  > 8. **Core's gameserver include leak is gone**: with the splits above,
  >    nothing Core compiles needs a gameserver header, so the PUBLIC
  >    `src/server/gameserver[/item]` exports on `Core` and the private
  >    gameserver dirs on all four packet libraries are removed.
  - Owner: CMake target membership + include-graph test.

**Phase exit criteria:** `de-kernel` builds standalone with no MySQL/Lua/Zone
includes (include-graph test green); at least GC/CG fully migrated off
`execute()`; all three servers boot and pass a manual smoke test against the
live client.

---

## Phase 3 — Testable domain logic: Outcomes, repositories, ownership rules

Runs as ongoing background work; every extraction is independently mergeable
and sheltered by Phase 1 tests. Ratchets R2/R3/R5 make progress monotonic.

- [x] **3.1 `Outcome<Events, Rejection>` result type.** C++20 template in
  `de-kernel` mirroring sidecar's `kernel.domain.Outcome`: gameplay mutations
  return `Ok(events)` or `Rejected(reason)`; exceptions reserved for
  programming/config errors. New/refactored domain code uses it; the
  `__BEGIN_TRY/__END_CATCH` macros stop being control flow (ratchet R5).
  > **Status:** done (2026-09-09). `src/Core/Outcome.h` is a `[[nodiscard]]`
  > `std::variant`-backed kernel type with unit tests. The shape every
  > adopter settled on: a `decide*` function takes a repository or topology
  > interface plus plain values, answers the events to perform — rows to
  > write, packets with their recipients, record mutations, in order — or
  > a typed rejection the handler maps to a wire code or to silence; the
  > handler gathers the facts, calls the decision and performs the events,
  > so every packet and every write keeps the old order and bytes. Each
  > adopter has a fake in `tests/support/` and a self-contained block at the
  > end of `tests/CMakeLists.txt`. Production callers, by process:
  > loginserver — every `CL*` decision: `CharacterCreation`,
  > `CharacterSelection`, `LoginDecision`, `CharacterDeletion`,
  > `ReconnectDecision`, `Registration`, `WorldSelection` (all under
  > `src/server/loginserver/`); no `CL*` handler decides by throwing.
  > gameserver — `exchange/ExchangeDecision` + `ExchangeService`,
  > `guild/GuildJoinDecision`, `party/PartyInviteDecision`,
  > `trade/TradePrepareDecision`, `trade/TradeTableDecision` (all under
  > `src/server/gameserver/`). sharedserver — `GuildDecision` +
  > `GuildStepRunner` for the five guild-mutation handlers. Assessed and
  > deliberately not adopted, with the reason at the site: `TradeManager::canTrade`
  > / `processTrade` and `CGTradeFinish` (a `goto` and an early return
  > interleave with the inventory simulation, so any split changes which
  > error code some inputs get), `CGPartyLeave` (its rules live in
  > `GlobalPartyManager`; the handler has nothing to decide),
  > `GSModifyGuildIntro` / `GSGuildMemberLogOn` (a lookup and one write).
  > Further decisions adopt the same shape as they are touched; the type
  > and the convention are complete.
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
  > **Status:** in progress (2026-09-06) — 34 seams under
  > `src/server/gameserver/repository/` (interface `*Repository.h`, impl
  > `MySQL*Repository.cpp`, reached through `default*Repository()`
  > accessors, never `g_p*` externs). R2 104→8 and R3 317→19 since the
  > pilot; every extraction is one branch and one PR
  > (`restructuring/*-repositor{y,ies}`, #18 through #85, then
  > `restructuring/motorcycle-redeem` and
  > `restructuring/quest-action-statements` and
  > `restructuring/handler-bookkeeping` and
  > `restructuring/comeback-event-handlers` and
  > `restructuring/creatureutil-purge` and
  > `restructuring/cgsay-statements`). The per-round
  > narrative — what moved, what the two adversarial reviews caught, the
  > byte-fidelity checks, the test list — lives in those PR descriptions
  > and commit messages, not here. Each repository header carries its
  > tables' quirks and an explicit **"not enclosed"** list of the SQL on
  > the same tables the seam does not cover; read the header before
  > extending a seam, and grep the whole tree (loginserver/,
  > sharedserver/, unbuilt files included) before rewriting that list.
  >
  > **Seams** (the header is the authority): `BalanceInfo`, `GameInfo`,
  > `ContentInfo`, `ZoneInfo`, `QuestInfo` — the read-only boot-time
  > catalogues (exp/attr ladders, skill/monster/NPC/script/option info,
  > zone config incl. ZoneEffectInfo, quest catalogues); `Character`
  > (race-table loads, saves, tinysave), `Gold`, `Stash`, `SkillSave`,
  > `RankBonus`, `BloodBibleSign` (read-only), `FlagSet`, `EffectSave`
  > (the persisted effects), `SMSAddress`, `QuestItem`, `PlayRecord`,
  > `Goods`, `Nickname` — per-character state; `ItemObject` (every
  > `*Object` table's create/save/load/destroy behind a per-table spec
  > row with object-shape and info-shape enums — a loader refuses a table
  > of the wrong shape) and `Item` (trace logs, counters, UniqueItemInfo,
  > TimeLimitItems, the ItemID registry probes); `Guild`, `Couple`,
  > `Friend`, `Message`, `Session` (login/logout bookkeeping on the
  > account and GuildMember rows), `WarInfo` (shrines, castles, sweepers,
  > schedules, histories, reinforcement, race-war limits), `FlagWar`,
  > `RegenZone`, `BulletinBoard`, `ComebackEvent`, `MofusPoint`,
  > `SystemAvailability`, `SpecialEvent` (the one seam on the
  > world-default connection — `getConnection(int)`, see its header),
  > `CharacterPurge` (deletePC's 109-statement character-deletion list,
  > one method, one Statement, in the original order — every table on
  > it is another seam's, and those headers say so); `Exchange` (the
  > Exchange feature's own access class relocated under the convention:
  > ExchangeListing, ExchangeOrder, the AccountPoint / PointLedger
  > statements that ask for a USERINFO connection and reach DARKEDEN,
  > and the transaction pair — the header says what each does against
  > the shipped schema). In ServerCore, under `src/server/repository/`
  > and compiled into all three binaries: `PayPlay` (PaySystem's Player
  > pay-play columns and the PC-room tables, on the dist connection) and
  > `ServerInfo` (GameServerInfo with its NonPKServerList and
  > CastleStatInfo flags, and WorldInfo). In the loginserver,
  > under `src/server/loginserver/repository/` with a `Login` prefix
  > (the integration binary links every impl, so names must not
  > collide with the gameserver's): `LoginCharacterPurge`
  > (CLDeletePCHandler's ownership check, Slayer retirement, DeleteChar
  > record and 112-statement purge on the per-world connection, plus
  > ItemDestroyer's uncalled 41-table sweep); `LoginAccount` (the Player
  > row through a session — the three login projections, the LogOn /
  > LoginIP / server-id writes that answer whether a row changed, the
  > current world, group and slot, the stored password hash,
  > registration — with TestClientUser, WebLogin, IPBlockInfo,
  > Event200501Main, PrivateAgreementRemain, PCRoomUserInfo and the
  > USERINFO.LoginPlayerData record); `LoginCharacter` (the race tables
  > as the character list, creation with its balance-table probes,
  > selection, the FlagSet preset, all on the per-world connection);
  > `LoginConfig` (GameServerGroupInfo in both projections, ZoneInfo,
  > ZoneGroupInfo, ClientVersion). In the sharedserver, under
  > `src/server/sharedserver/repository/` with a `Shared` prefix:
  > `SharedGuild` (its own Guild/GuildManager persistence — GuildInfo,
  > GuildMember, the guild's GuildUnionMember rows and the
  > WarScheduleInfo cancel — plus the GS handlers' race-table GuildID
  > and Gold writes and their Messages inserts, the two whitespace
  > spellings behind an enum) and `SharedConfig` (the boot-time reads
  > of GameServerGroupInfo, GameServerInfo, ZoneInfo's resurrection
  > columns and SSStringPool).
  >
  > **Conventions the rounds settled** (each one cost a review finding):
  > - Statements move **byte-for-byte**, quirks included (backticked
  >   `Rank`, mixed-case keywords, copy-paste whitespace, `%ld` fed a
  >   DWORD). 3.2 moves SQL; it does not fix it. A bug found in a
  >   statement is recorded in the header, pinned by a test, and gets
  >   its own PR.
  > - Repository SQL uses the parameterized `executeQuery` form, never
  >   string concatenation; StringStream chains become format strings
  >   with the same bytes. `executeQueryString` survives only where the
  >   statement takes no arguments or must be logged verbatim.
  > - Rows are typed to the driver getter the inline code called
  >   (`getInt`→int, `getBYTE`→BYTE, `getString`→std::string); the
  >   caller keeps its casts and narrowings. Loads return a vector read
  >   in full before the caller acts on the first row.
  > - Same statement, different bytes → a spelling enum over a spec
  >   table, one literal written once. Different projection → distinct
  >   methods (distinct arities make a mix-up a compile error).
  >   Different value only → one format string with a typed parameter.
  > - Column names chosen at runtime (by race, by sex) move into the
  >   seam with the statements; the game-side lookup table is deleted.
  > - **Integration tier over fakes**: `mysql_repository_tests`
  >   (tests/integration/, `make integration-test`, needs docker) runs
  >   the real impls against MySQL 5.7 loaded with `initdb/` and the
  >   production sql_mode; 194 tests, all green since the width fix
  >   (see the DWORD bullet below). A quirk is replayed there
  >   before it is written down — the first rounds' fakes documented three
  >   behaviours the server refuted. Only the six pilot-era seams keep a
  >   fake (tests/support/). A seam whose callers are compiled out still
  >   gets a test: the compiler will never check them.
  > - R2/R3 are **textual** greps. A commented-out block that names code
  >   the conversion deleted is rewritten to name the seam method (it is
  >   otherwise wrong); a self-contained commented-out block is deleted,
  >   with the dead function around it when nothing calls it.
  >
  > **Knowing changes common to every seam:** the Statement is freed on
  > every success path (the originals leaked one on dozens of paths,
  > counted per round in the PRs); `DBError.log` and the `const char*`
  > `END_DB` rethrows name the repository method, not the caller; a SQL
  > failure crosses the seam as that `const char*`, so callers that
  > caught `SQLQueryException` were rewritten to catch `const char*`
  > (Mofus.cpp, whose swallow is load-bearing) or shown equivalent
  > (CGConnectHandler, whose `catch (...)` above disconnects either
  > way); `SELECT MAX()` empty-table guards that could never fire (MySQL
  > answers one NULL row, which the original `atoi`'d and crashed on)
  > now raise the intended Error. Two behavioural fixes made on purpose,
  > each disclosed in its PR: `WarScheduler::load` ran its inner query
  > on the Statement it was iterating and so loaded only the first guild
  > war per zone; `GuildUnionManager::removeMasterGuild`'s non-master
  > branch read a freed Result, so a non-master guild could never leave
  > a union.
  >
  > **Pre-existing bugs found, pinned by the tier, deliberately not
  > fixed** (each named in its header):
  > - `FriendList`/`FriendHistory` are not in `initdb/`; opening the
  >   friend list disconnects the client.
  > - The FlagWar roll-up's GROUP BY is refused under ONLY_FULL_GROUP_BY
  >   (1055); the escaping `const char*` reaches `main.cpp`'s
  >   `catch (...)`, so the first flag war to end takes the process
  >   down. Dead by config: `ActiveFlagWar` is 0 in both shipped confs.
  > - `RaceWarLimiter::clearPCList` reads the Name column as the race
  >   and indexes a three-element array with it.
  > - `GoodsRepository::takeOne` on a Num=0 row raises
  >   ER_DATA_OUT_OF_RANGE and leaves the purchase stuck.
  > - `giveGoldMedal`'s `INSERT INTO GoldMedalCount` names a table
  >   `initdb/` does not create, so every gold-medal award fails as a
  >   SQL error (PlayRecordRepository::insertGoldMedal, pinned).
  > - LearningItem's UPDATE says `Storage=%s` for an int;
  >   VampirePortalItem's zone loader reads eleven getters over an
  >   eight-column SELECT; CodeSheet's zone SELECT names columns its
  >   table lacks.
  > - DWORD fields through `%lu`/`%ld` (exp saves, item ids, Key.Target)
  >   worked only by GCC codegen and were preserved bit-for-bit by the
  >   extraction rounds. **Under the pinned Zig/Clang 21 toolchain they
  >   did not work (found 2026-09-06):** the integration tier on
  >   unmodified master failed 11 of its 169 tests
  >   (`CharacterMySQL.SlayerExpsTailLandsInFull` and ten
  >   `ItemObjectMySQL` round-trips) because a 32-bit argument read
  >   through a 64-bit conversion takes whatever the upper half of the
  >   register holds — a Fame of 777 landed as 4294967295 (clamped by
  >   the unsigned column), an UPDATE keyed by `ItemID=%ld` matched no
  >   row (Fame_t and Exp_t are both DWORD). **Fixed the same day**
  >   (`fix/varargs-width-conversions`): every conversion is now the
  >   argument's own width — `%u` for the DWORD/WORD/BYTE arguments,
  >   `%d` for the one int fed `%ld`, `%ld` for the `time_t` DayTime and
  >   NextTime that were exact or that `%d` had been truncating — 449
  >   conversions across seven seam impls and 48 `sprintf`-built tinysave
  >   fragments, messages and one pointer print in game code. The bytes
  >   MySQL receives are
  >   identical for every value the argument can hold, so this is a
  >   byte change in the source and not on the wire. To keep it fixed,
  >   `Statement::executeQuery` now carries `__attribute__((format(printf,
  >   2, 3)))` and the build compiles with `-Wformat` instead of
  >   `-Wno-format`; the tree is warning-free under it. The one class it
  >   cannot see is a format reached through a pointer (the per-table
  >   spec rows in the ItemObject, EffectSave and ComebackEvent seams);
  >   those were retyped by hand and the tier pins them.
  > - GuildUnionOffer's PK is OwnerGuildID alone, so an ESCAPE insert
  >   over a standing JOIN/QUIT row throws out of CGQuitUnionHandler.
  > - ActionShowGuildDialog gates guild creation on a hardcoded seven
  >   days ahead of the handler's QUIT_GUILD_PENALTY_TERM.
  >
  > **Three one-line Core defects every seam inherits — owed a Core
  > round; two now fixed:** `END_DB` threw `msg.c_str()` of a local
  > string, so every handler received a dangling pointer — **fixed**, the
  > macros throw a `DatabaseError` (`src/server/database/DatabaseError.h`)
  > that owns the `DBError.log` line and the 34 handlers name that type
  > (`docs/FIXES.md`; `MySQLSMSMessageRepository` keeps its own
  > `END_DB_RETHROW`, which its thread's reconnect branch catches as a
  > `SQLQueryException`); `__LEAVE_CRITICAL_SECTION` released only on
  > `Throwable&`, so a repository call inside a critical section left its
  > mutex held on failure — **fixed 2026-09-05**, the section is now a
  > scoped guard that releases on any exit (`docs/FIXES.md`);
  > `SAFE_DELETE(pStmt)` sits inside every seam's try, so a
  > non-SQLQueryException throw (`bad_alloc`, `OutOfBoundException`)
  > leaks the Statement — open.
  >
  > **What remains.** R2 is 0: the gameserver root holds no SQL, live or
  > commented out. `TradeManager.cpp`'s TradeLog INSERT is
  > `PlayRecordRepository::logPlayerTrade`, still assembled as a string
  > and sent through `executeQueryString` because a trade's log is as
  > long as the two inventories make it and `executeQuery`'s format
  > buffer holds 2048 bytes; the MySQL tier pins it, including a content
  > past that width. `SMSServiceThread.cpp`'s three relay statements are
  > `SMSMessageRepository`, which owns the SMS_DB_* connection because
  > the relay is not one of DatabaseManager's servers; live but dormant
  > (`GameServer::start()` does not start the thread) and unpinnable by
  > the tier, since `uds_msg` and `msg_queue` are not in `initdb/`.
  > `CreatureUtil.cpp`'s commented-out `addOlympicStat` body is deleted
  > (the function stays: its dozen callers record nothing).
  > `EventMonsterNameManager`, `GameServerInfoManager` (a stale third
  > copy), `GameWorldInfoManager` (a stale fork of ServerCore's live
  > loader) and `MoonCardUtil` (a stale subset of `EventItemUtil.cpp`)
  > are deleted — they were in no CMakeLists and never compiled (no
  > `file(GLOB)` exists anywhere). R3 is 0 too. `item/EventBall.cpp`
  > and its header are deleted: seven live statements against
  > `EventBallObject` and `EventBallInfo`, neither table in `initdb/`,
  > the file in no CMakeLists and the class named nowhere else.
  > `handler/CGSayHandler.cpp`'s last live statement went with the
  > `*notice` operator command, whose entire body was an INSERT into
  > `quick1001` on a hard-coded remote BBS host: the host, database,
  > user and **password were literals in the source** and the GM's chat
  > text was interpolated **unquoted and unescaped** (`docs/FIXES.md`).
  > The command answered the player nothing, so its dispatch branch and
  > its body are gone; the declaration stays in `src/Core/CGSay.h`,
  > which the client repo mirrors.
  > Everything else R3 counted was an `executeQuery` inside a comment
  > block, deleted with the block: five `mission/` files (among them
  > `EventQuestRewardManager`, whose only member was that commented-out
  > body and which nothing constructs), the commented-out
  > `EffectBloodyWallLoader` and `EffectGrayDarknessLoader` in both
  > their headers and their implementations, `SiegeWar`'s
  > `recordSiegeWarStart` and `recordSiegeWarEnd` (declarations, bodies
  > and both call sites) and the loginserver's `addLogoutPlayerData`
  > (declaration, body and its call from `disconnect`).
  - Owner: R2/R3 ratchet tests; repository unit tests (fake/in-memory
    implementations for domain tests; MySQL-backed integration tier runs
    locally against the existing docker + `initdb/` schema).

- [ ] **3.3 Pure formula functions with unit tests.** Extract
  `SkillFormula`/`SkillUtil` math and stat calculations (`InitAllStat.cpp`)
  into pure functions in `de-core`. These are the highest-value tests in the
  game — they encode balance — and the cheapest to write.
  > **Status:** in progress (no named extraction targets remain — new
  > formulas join as code is touched; updated 2026-09-01 after the
  > InitAllStat review round) — the `de-core` STATIC target
  > exists (`src/domain/`, freestanding by construction) with its first
  > content: all of `AbilityBalance.cpp` (HP/MP/to-hit/defense/protection/
  > damage/attack-speed/critical/steal per race) plus `computeFinalDamage`,
  > `getDistance`, `computeRankExp` and `decreaseConsumeMP` from
  > `SkillUtil.cpp`, transplanted verbatim into `src/domain/Formulas.cpp`
  > (narrow-integer wrap-around preserved) behind thin adapters at the old
  > entry points. `formula_tests` (ctest, links ONLY de-core + gtest) pins
  > the math including the wrap cases; R6 is now enforced by `ratchets.sh`
  > for `SkillUtil.cpp`/`InitAllStat.cpp`. **`HitRoll.cpp`'s success-ratio
  > formulas are extracted too** (melee/blood-drain/magic-per-race/curse/
  > dispel/flare/rebuke/self-buff/hallucination/backstab — the dice rolls
  > and live-state gates stay in the adapters; the `__CHINA_SERVER__`
  > variants stay behind their #ifdef there; `isCriticalHit`'s additive
  > ratio and the blood-drain defense gathering remain inline), pinned by
  > 19 more tests (62 assertions) including the floorless negative
  > `flareRatio` and the toward-zero negative-bonus truncation;
  > `HitRoll.cpp` joins R6 as R6c.
  > **`SkillFormula.cpp` is extracted (2026-09-01)**: 293 of the 304
  > per-skill `computeOutput` formula bodies moved verbatim to
  > `src/domain/SkillOutputFormulas.cpp` (decore::skillformula — mirror
  > SkillInput/SkillOutput structs with identical field names/enum values
  > so the diff is a pure move; the legacy comments — double-encoded
  > EUC-KR/GBK mojibake — were then machine-recovered and translated to
  > English in a follow-up commit, code untouched by comment-stripped
  > diff); the member functions are now one-line delegation macros
  > (SkillFormula.cpp 3,081→820, joins R6 as R6d). The 11 formulas that
  > roll dice inline (`Random()`/`rand()` — CriticalGround, MeteorStrike,
  > DuplicateSelf, the four axe-throw skills, Cannonade, SelfDestruction,
  > BloodCurse, VoodooRing) keep their original bodies in the adapter
  > file: the roll stays out of de-core, the HitRoll rule. Three
  > impurities were externalized, each preserving observable behavior: the
  > `g_pSkillInfoManager->getGradeByDomainLevel` call becomes a
  > `DomainGrade` input fetched only by the three grade-using adapters
  > (ContinualLight/Purify/DetectInvisibility — same call, same possible
  > throw, on the same invocations); the `Item::ItemClass` comparisons
  > become a `GunClass` enum the adapter maps (four gun classes + Other);
  > `HeadShot`'s `Assert(false)` on a non-gun class fires in the adapter
  > before delegation (equivalent: all 393 compiled call sites pass a
  > freshly zeroed SkillOutput, and no formula body reads an output field
  > before writing it, so the copy-back of all six fields is identical to
  > the original partial assignments; the one output-reusing caller was
  > in the never-built legacy `gameserver/test/` dir, deleted 2026-09-05).
  > `formula_tests` pins every gun-class branch (MultiShot, HeadShot,
  > MoleShot), every grade switch including the unset-grade default, and
  > the no-break HeadShot fallthrough where every in-range Range cascades
  > to the case-1 damage, plus a representative spread (party boosts,
  > Revealer's Delay-before-boost ordering quirk, clamps, negative
  > outputs, Delay=Duration couplings, empty formulas). The adapter's
  > field mapping itself is the one surface no suite can see (the tests
  > deliberately link only de-core) — hand-verified in the adversarial
  > review, flagged as such in the code. Both reviewers (2x xhigh,
  > 2026-09-01) returned SHIP; their byte-level audit found 286 of the
  > 293 moved bodies byte-identical and the other 7 differing only by
  > the documented substitutions.
  > **The `InitAllStat.cpp` bonus formulas are extracted (2026-09-01)**:
  > 19 pure functions joined `Formulas.{h,cpp}` — Concealment's
  > divide-then-float-scale bonuses, Will of Iron's truncated 15%, both
  > Liveness grade tables (normal keeps its level>=125 hpPercent
  > override; the `__CHINA_SERVER__` selection stays behind the #ifdef in
  > the adapter), Sniping's divide-first percents, the four slayer
  > weapon-domain passives (sword mastery / concentration / evasion /
  > shield mastery, including evasion's negative-term truncation below
  > level 20), the vampire wolf/werwolf damage bonuses and Extreme's
  > capped bonuses, Intimate Grail's shared penalty ratio, Summon
  > Sylph's floored bonuses, and Hide Sight's two level bands with the
  > 10% truncated bump at exactly exp level 30. The adapters keep every
  > live-state gate (canUse, effect flags, item class, isRealWearing)
  > and every member write incl. the per-race caps — same split as the
  > HitRoll extraction.
  > **The adversarial review round (2x xhigh, 2026-09-01) proved the 19
  > transplants exact** — one reviewer ran a differential harness
  > compiling master's removed expressions verbatim (at master's declared
  > widths) against libde-core: 59.7M input combinations at -O0 and -O2,
  > zero mismatches — **but falsified the first draft's "no formula
  > content left" claim and caught a divergence the extraction itself
  > created** (the slayer's third Intimate Grail block kept `10+level/10`
  > inline while the vampire/ousters copies got the pinned function). The
  > fix round extracted everything the reviewers named: the slayer grail
  > ratios (`intimateGrailRatio`, sign of application stays at the call
  > sites, + the 6.6-divisor `intimateGrailHPRatio`), the gun-domain /10
  > damage term, Vampire Nail Mastery and the DEX→HPRegen ladder, the six
  > Ousters soul-stone passive points, and the three per-race
  > BloodBibleSign fame ladders (whose thresholds had already drifted
  > between races — now pinned per race). de-core now owns 33 InitAllStat
  > formulas. Deliberately NOT extracted, with reasons: percentValue
  > applications of effect-carried parameters and rank bonuses applied as
  > stored points (parameter application, no formula), Mephisto's capped
  > percent application (same category), Monster::initAllStat's
  > hardcoded event `HP*10` for four monster ids (no stat/level
  > composition), and the flat arms-mastery constants (`ToHitBonus += 5`
  > etc. — no computation). The `__CHINA_SERVER__` liveness path is
  > compiled by no build config; it was hand-compiled clean in the
  > review, and `livenessBonusChina` is now compiled and unit-tested for
  > the first time. InitAllStat.cpp 4,949→4,803 across both commits (R6b
  > tightened).
  - Owner: the formula test suite; R6 line ratchets on `SkillUtil.cpp` /
    `InitAllStat.cpp` / `HitRoll.cpp` / `SkillFormula.cpp`.

- [x] **3.4 Codify thread ownership.** Document (in CLAUDE.md) which state is
  owned by which thread: zone-group state mutated only on its
  `ZoneGroupThread`, cross-group communication via queues only. Add
  debug-build `assertOwnedByZoneThread()` checks on Zone/Creature mutation
  entry points (sidecar analog: "never block the registry mailbox" — the
  invariant is written down *and* asserted).
  > **Status:** contract documented in CLAUDE.md ("Thread ownership",
  > 2026-08-31): ownership is mutex-guarded, not thread-affine — the
  > `ZoneGroupThread` holds the group mutex for its whole tick and other
  > threads must take it. Debug-only `ZoneGroup::assertOwned()` guards
  > the eight `Zone` mutation gateways `addPC`×2/`replacePC`/
  > `addCreature`/`deleteCreature`/`moveCreature`/`addCreatureToTile`/
  > `deleteCreatureFromTile`. Hardened by the adversarial review:
  > the machinery rides `DE_OWNERSHIP_CHECKS` (Debug-only compile flag —
  > this repo never defines `NDEBUG`, so gating on it was a no-op and
  > the bookkeeping was live in release), a violation now `abort()`s
  > instead of throwing (an `AssertionError` is a `Throwable`, and the
  > `catch (Throwable&)` on these very paths swallowed it — e.g.
  > `GamePlayer::disconnect`'s empty catch would have skipped the
  > character save), `pthread_equal` + a valid flag replace the raw
  > compare/zero sentinel, and the review's main-thread hole is closed:
  > packets pipelined behind `CGReady` no longer drain on the main
  > thread after `GPS_NORMAL` opens the validator gate. No creature is
  > written to a `Tile` outside `Zone.cpp` any more: the race-swap sites
  > go through `replacePC`, the move-mode swaps, knockback/warp moves and
  > corpse paths through the tile-only pair (CLAUDE.md has the list of
  > what is still not gated). The three `GDRLair*::start` loops now take the
  > group mutex like the file's other sites (2026-09-05). The
  > cross-group `DynamicZone` `addZone()` race is fixed (2026-09-05):
  > the group zone map and the `ZoneInfoManager` tables are
  > `de::Snapshot`s (copy-on-write, `src/server/Snapshot.h`), a recycled
  > instance's `init()` is posted to the owning group, and
  > `DynamicZoneGroup` serialises selection under its own mutex. **2026-09-05: the SG/LG/GG one is
  > fixed for creature state** — `GamePlayer` carries a mailbox
  > (`src/server/Mailbox.h`) that the manager owning the player drains
  > each tick (the zone manager under the group mutex; the main thread
  > only for player-scoped commands), and
  > `de::postToPlayer` routes the six guild handlers' and
  > `LGKickCharacter`'s mutations through it; the
  > "cross-group communication via queues only" rule above now has its
  > queue. The handlers' `Guild`/`GuildMember` writes are covered too: the
  > member maps and counters, the per-member flags, and the guild's own
  > scalar fields — integral ones relaxed atomics, strings copied under the
  > guild's leaf mutex.
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

- [ ] **4.1 GM-command router for `CGSayHandler.cpp`** (4,904 lines). A
  `CommandRouter` with one class/function per GM command and **gating
  declared at registration** — sidecar's `on(name, limiter, handler)` /
  `onGated(...)` insight: hand-kept permission maps drift silently, so the
  registration site is the single source of truth. A test enumerates
  registered commands and asserts each declares a permission level.
  > **Status:** not started
  - Owner: router registration + the enumeration test; R6 ratchet.

- [ ] **4.2 Split `Zone.cpp`** (9,263 lines) by concern: movement, broadcast,
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

- [x] **5.3 Fix log.** When restructuring uncovers real bugs (1.4 layout
  diffs, races, double-frees), record them in `docs/FIXES.md` with sidecar's
  `> **Status:**` convention rather than fixing silently.
  > **Status:** done — `docs/FIXES.md` created with the 1.4 max-size
  > reconcile entries (2026-08-31); the earlier Exchange defect set stays
  > recorded inline in 1.4 where it was written. Ongoing discipline, not a
  > one-shot: new finds keep landing there.

- [x] **5.4 Language standard: C++11 → C++20 required.**
  Assessed 2026-08-30. No open task was *blocked* on the standard, but 3.1
  `Outcome` wants `std::variant` + `[[nodiscard]]` (an unchecked rejection
  becomes a compiler warning instead of a convention), the packet layer
  wants `string_view`/`optional`/`if constexpr`, and googletest is pinned at
  1.12.1 only because it is the last C++11 release. At assessment time the
  image's Ubuntu 20.04/GCC 9.4 compiler had full C++17 but only partial
  `-std=c++2a`. The completed work avoids a distro/dependency bump by making
  both builders use the already-added Zig toolchain: pinned Zig 0.16.0 ships
  Clang 21.1.0 while the runtime remains Ubuntu 20.04.
  The main mechanical migration cost was pre-C++11 dynamic exception
  specifications.
  A full production-target build also found three previously uncompiled uses
  of removed library facilities: `binary_function`, `bind2nd`, and
  `set_unexpected`; all three are now gone.

  | Form | Pre-migration | C++17 | C++20 |
  |------|---------:|-------|-------|
  | typed `throw(Error)`, `throw(ProtocolException, Error)`, … | 139 files / ~650 sites | hard error | hard error |
  | empty `throw()` | ~2,600 sites | accepted (= `noexcept`) | removed (GCC/Clang warn; `-pedantic-errors` fails) |

  Plan: one mechanical PR that (a) **deletes** typed specs — never replace
  with `noexcept`: `throw(Error)` means *may throw Error*, and `noexcept`
  would turn every thrown `Error` into `std::terminate`; (b) deletes the
  empty `throw()` in the same sweep so the later C++20 flip is free; (c) sets
  `CMAKE_CXX_STANDARD 17`; (d) v18-reformats every touched file — CI demands
  it anyway, and this is the one-time reformat CLAUDE.md has been deferring,
  so land it when no feature branch is in flight; (e) `make test` green: the
  goldens and inventory prove the wire layer did not move. Then rewrite 3.1's
  "C++11 template" to `std::variant` + `[[nodiscard]]` and unpin googletest.
  Not required for the mechanical migration: modules and coroutines still need
  architecture/build evidence before adoption; concepts become useful only at
  focused boundaries such as the packet stream and factory contracts described
  in `docs/TOOLCHAIN.md`.
  > **Status:** done (2026-09-04) — C++20 is the required project standard
  > (`CMAKE_CXX_EXTENSIONS=OFF`). The initially verified C++17 transition lane
  > was retired deliberately when `std::jthread`/`std::stop_token` entered the
  > production `ZoneGroupThread` lifecycle; non-20 configurations now fail at
  > CMake configuration instead of failing partway through compilation.
  > Docker and development-volume builds pin
  > Zig 0.16.0/Clang 21.1.0, isolate build trees, output roots and compiler
  > caches by Zig version, target and build type. The migration was validated
  > under both C++17 and C++20 before the rollback lane was retired; the
  > production image now always builds C++20, and all three server binaries
  > have complete runtime linkage. All dynamic
  > exception specifications are gone; legacy destructors that were declared
  > as potentially throwing retain that behavior with `noexcept(false)`.
  > `Outcome` uses
  > `std::variant`/`[[nodiscard]]`, and GoogleTest is updated to v1.18.0.
  > The runtime remains Ubuntu 20.04; its distro GCC is no longer the compiler.
  > Project-specific C++20 adoption priorities and guardrails are documented in
  > `docs/TOOLCHAIN.md` under “Where C++20 pays off in DarkEden.”
  > The first adoption slice also makes thread status atomic, adds reusable
  > cooperative-worker lifecycle tests, and gives the zone thread pool a real
  > stop-all-then-join-all shutdown path. `GameServer` destroys that pool before
  > zone/database dependencies, preventing workers from observing freed state.
  > Adversarial-review follow-up (2026-09-05): all gameserver auxiliary workers
  > now use the managed backend too; startup rollback, concurrent lifecycle
  > operations, and worker failure reporting have regression tests. SIGTERM
  > drains the client loop and joins every worker, with a 30-second failed-exit
  > deadline for blocked work. Main then lets the OS reclaim the legacy graph
  > rather than invoking unaudited singleton destructors; no new world-save
  > guarantee is implied. MySQL operations have finite timeout options.
  > CMake probes the C++20 library and a pinned-Zig workflow tests/builds master.
  > Extended to the other two processes (2026-09-05): the loginserver and
  > sharedserver `GameServerManager` workers and `SMSServiceThread` moved off
  > the legacy `Thread` too, and the never-compiled
  > `NetmarbleGuildRegisterThread` (in no CMake target, its every call site
  > commented out) was deleted rather than migrated, so `ManagedThread` is now
  > its only subclass and the pthread-only `detach()` and unused static
  > `join()` overloads are gone. R1: 333 -> 332. Both processes install the
  > SIGTERM/SIGINT handler, end their main loop on the request, stop and join
  > their worker while its dependencies are alive, and `_Exit` with the
  > failure code under their own 30-second watchdog. The loginserver's UDP
  > listener became nonblocking so an idle link cannot hold the worker inside
  > `recvfrom`. `docker/start.sh` now bounds the login/shared drain at 8
  > seconds after the gameserver's 35, staying inside Compose's 45-second
  > grace period, and the shutdown watchdog now names the process it kills.
  - Owner: ratchet R7, held at 0.

---

## Phase 6 — CI (deliberately last; limited by GitHub Actions minutes)

The C++20 lifecycle work now has a pinned-Zig Debug test and production-build
workflow (6.1, master pushes/merges only). Execution still depends on Actions
minutes being available. The remaining broader CI rollout below is deferred:

- [x] **6.1 Build + test workflow.** Pinned Zig Debug contract/lifecycle tests,
  all production targets, and the production image; master pushes/merges only.
  > **Status:** done — `.github/workflows/cpp20.yml`; running the jobs still
  > requires available Actions minutes. Local verification remains mandatory.

- [ ] **6.2 Integration tier in CI.** MySQL service container + `initdb/`
  schema for the repository integration tests.
  > **Status:** blocked (no Actions minutes)

---

## Legacy service cleanup (2026-09-05)

- [x] **Remove China billing.**
  > **Status:** Removed the entire `src/server/chinabilling/` tree (including
  > its stress/test servers), both CBilling library targets and legacy Makefile
  > links, `EventCBilling`, the player-info mixins, dormant login/game hooks,
  > and the dedicated database connection API. The feature switch was already
  > disabled, so supported builds retain their existing login/payment behavior.
  > The separate `gameserver/billing/` and `PaySystem` remain intact.
  > Client packet IDs/error codes, string-table IDs/data, and the historical
  > `src/build_error.sql` build transcript are deliberately unchanged.
  > R1: 351 → 345; R3: 85 → 84; R5: 5,899 → 5,897.
  > The ratchet suite rejects China billing references in source/build files.
  > Verified with the pinned Zig/C++20 Debug toolchain: all three production
  > servers build, all nine CTest suites pass, and touched C++ files pass
  > clang-format 18. No live deployment was restarted for this cleanup.

- [x] **Remove `theoneserver`.**
  > **Status:** Removed on 2026-09-05, following the audit below. Deleted
  > the whole `src/server/theoneserver/` tree (its private `Connection`/
  > `Statement`/`Result`/`DatabaseManager` forks, `GameServerManager`, the
  > unbuilt `KillManager` duplicate, `UDPManager`, thread-pool forks,
  > `PacketFactoryManager`, `TheOneServer` and `main.cpp`) and the three
  > `alltheoneserver` recipes in `src/Makefile`, `src/Core/Makefile` and
  > `src/server/Makefile`. Nothing else referenced the tree: no CMake
  > target, install entry, configuration, or Docker startup ever did.
  > R1: 345 → 338 (the tree's seven `g_p*` externs); R3: 84 → 81 (its
  > three SQL-bearing forks). R5 is gameserver-scoped and unchanged.
  > The ratchet regression scan that rejects China billing references now
  > also rejects `theoneserver`/`TOpackets` in source and build files;
  > the historical `src/build_error.sql` transcript is deliberately
  > untouched. Generic `GGCommand` handling, the shared `database/`
  > classes, and the login/shared-server managers are unchanged — the
  > supported servers still use them.
  >
  > **Audit (2026-09-05, pre-removal):** obsolete in the supported
  > repository build and deployment.
  >
  > Its role was publisher server registration/policy enforcement, not a
  > fourth gameplay service. `TheOneServer.cpp` created a database manager
  > and UDP `GameServerManager`; the latter bound `TheOneServerUDPPort`
  > and dispatched received datagrams. Its local `PacketFactoryManager.cpp`
  > registered only `GTOAcknowledgement`.
  >
  > The deleted handler (inspect
  > `git show 25f25ee6^:src/Core/TOpackets/GTOAcknowledgementHandler.cpp`)
  > recorded server IP/port/heartbeat information in
  > `TheOneServerRules_New`, consulted `NEWSERVER_POLICY`, and sent
  > `GGCommand` remote-control commands such as `*shutdown 0` under
  > deny/kill policies. This is historical functionality, not a live path.
  >
  > Commit `25f25ee6` already deleted the TOpackets protocol and the
  > China/Thailand-only phone-home sender/timer in gameserver's
  > `ClientManager.cpp`. No remaining caller outside the dead tree was
  > found. The legacy `alltheoneserver` Makefile recipes required the
  > deleted `libTheOneServerPackets.a` and packet headers and could not
  > resurrect a working service. `KillManager.{h,cpp}` was an unbuilt
  > duplicate of `GameServerManager`, not a separate active kill worker;
  > `UDPManager` construction was commented out and it was absent from
  > the legacy object list. The audit covered the checked-in
  > build/deployment, not independently maintained out-of-tree binaries
  > or installations. The tree remains recoverable from git history
  > (`git show <this commit>^:src/server/theoneserver/`).

- [x] **Remove `updateserver`.**
  > **Status:** Removed on 2026-09-05. Deleted `src/server/updateserver/`
  > (the TCP patch-distribution daemon `UpdateServer`/`UpdateServerPlayer`,
  > `main.cpp`, and ~7,000 lines of one-off patch-manifest generators —
  > `update.cpp`, `update2.cpp`, `p.cpp`, `p21.cpp`, `semiup.cpp`,
  > `info.cpp`, `fuck.cpp` — that are not valid C++ and were never in any
  > object list), `src/server/old_update.tar` (a 2000s-era tarball of the
  > same tree), `conf/updateserver.conf`, and the Core patch-manifest
  > classes `Update.{h,cpp}` / `UpdateManager.{h,cpp}` whose only consumer
  > was this daemon (dropped from `tests/arch/kernel_files.txt`; they held
  > no packet factory, so the wire inventory is unchanged). `UpdateDef.h`
  > stays: `Resource` uses its size typedefs. Also removed the
  > `UpdateServerDatabase` CMake target — a fourth copy of `database/`
  > compiled under `__UPDATE_SERVER__`, which no source in `database/`
  > tested and no executable linked — plus the `.us.o` suffix rules, the
  > `libUpdateServerPackets.a`/`libUpdateServerDatabase.a` recipes and
  > the `updateserver` clean hooks in the legacy Makefiles. The
  > `__UPDATE_SERVER__`/`__UPDATE_CLIENT__` `PlayerStatus` enum branches
  > are gone; they were `#elif` alternatives to the game/login/shared
  > branches, so no supported build's enum values move.
  >
  > **Audit:** the daemon served the launcher's file-patch protocol
  > (`Upackets`: `CUBeginUpdate`/`CURequest`/`CUEndUpdate`/`UCUpdateList`)
  > over its own TCP port from `PatchDir`, versioned by `PatchVersion`.
  > `Upackets` was deleted in commit `25f25ee6`, so the tree had not
  > compiled since; it was already commented out of the legacy
  > `src/server/Makefile` `all` target, and no CMake target, Docker
  > startup, or `docker/conf` entry referenced it. The loginserver's
  > `CLVersionCheck` still enforces the client version independently.
  > R1: 338 → 337 (the tree's `g_pConfig` extern); R3/R5 unchanged. The
  > ratchet regression scan now also rejects `updateserver`/`Upackets`/
  > `__UPDATE_SERVER__`/`__UPDATE_CLIENT__` in source and build files.

- [x] **Remove `cacheserver`.**
  > **Status:** Removed on 2026-09-05. `src/server/cacheserver/` was a
  > two-file (~220 line) "Database Cache Server" prototype from February
  > 2002 by a Metrotech contractor, untouched since the ragezone recovery.
  > `main.cpp` was the gameserver's main loop with the names swapped,
  > calling a `CacheServer` class defined nowhere in the repository and
  > including the gameserver's `GameServer.h`; `Query.h` was a broken
  > query-record class whose setters had degraded into invalid text. No
  > CMake target, legacy Makefile recipe, Docker/config entry, or source
  > outside the directory referenced it. It held no `g_p*` externs or
  > SQL, so R1–R7 are unchanged; the ratchet regression scan now rejects
  > `cacheserver` too.

- [x] **Remove the dead sub-trees, stale forks and VSS residue.**
  > **Status:** Removed on 2026-09-05. Nothing below was compiled by any
  > CMake target; the legacy Makefiles carried only commented-out or
  > include-path references.
  >
  > Deleted inside `gameserver/`: `test/` (a 2002 cppunit suite plus
  > `MockPlayer`/`MockZone`/`MockSkillInput`, driven by a `-t` argv flag
  > and `g_pTestConfig`; its `GameServerTester.h` include in
  > `GameServer.cpp`, the `test` include path, the `-t` parsing in
  > `main.cpp` and the `g_pTestConfig` global in Core `Properties` went
  > with it), `testAlone/` (a standalone scheduler/war harness carrying
  > private forks of `VSDateTime`, `Mutex`, `StringStream`, `Schedule`,
  > `Scheduler`, `WarSystem`), `mofus/testserver/` (a fake mofus peer),
  > `quest/Squest/` (the pre-Lua "simple quest" system, reachable only
  > through `__ACTIVE_QUEST__` blocks that no build defines),
  > `gameguard/` (a 2003 INCA nProtect `CSAuth` SEED-cipher header and
  > `.tab`/`.idx` tables; `ObjectManager.cpp`, `CGAuthKeyHandler.cpp` and
  > `CGSayHandler.cpp` still included the header around commented-out
  > calls — the includes and `ObjectManager`'s two "CSAuth ...
  > Initialization" print pairs are removed, so startup logs four fewer
  > lines), `billing/test/` (config fixtures),
  > `item/_{weapon,armor,gear,accessory,etc}/` (leftover `create`
  > scripts) and `quest/luaScript/test/` (xmas-event Lua fixtures with
  > compiled `.luac` copies). The stale commented-out `test_exchange`
  > block in `gameserver/CMakeLists.txt` referred to a file that no
  > longer existed.
  >
  > Deleted in the `src/server/` root: the unbuilt ServerCore forks
  > `Restore.cpp`, `Restore2.cpp`, `ZoneUtil.cpp`,
  > `IncomingPlayerManager.cpp` (each a stale copy of a gameserver file)
  > and `UserGateway.{h,cpp}` (referenced only from comments).
  >
  > Deleted everywhere: all 27 `vssver.scc` Visual SourceSafe files.
  >
  > **Ported to the gtest suite** rather than deleted: the
  > `GameServerSkillTest` sharp-shield expectations now live in
  > `tests/formula_test.cpp` against `decore::skillformula::SharpShield`
  > (with the 2003 "Delay equals Duration" balance, which the old test
  > predated); `GameServerWarTest`'s `VSDateTime` arithmetic and
  > `testAlone`'s `ScheduleTest`/`WarSystemTest` scheduler behaviour are
  > `tests/scheduler_test.cpp`, a new `scheduler_tests` target linking
  > de-kernel plus the live `war/Schedule.cpp`/`Scheduler.cpp`. The live
  > `Schedule::heartbeat()` reads the wall clock (the old harness stepped
  > a fake one), so the port uses past/future scheduled times instead.
  > Not portable: `GameServerItemTest`/`ExpTest` (need
  > `g_pVariableManager`/`g_pLuckInfoManager` loaded from the DB),
  > `SpeedCheckTest` (needs a live `GamePlayer` socket) and
  > `UserGatewayTest` (its subject was deleted).
  >
  > R1: 337 → 333 (`g_pTestConfig` plus three externs in the deleted
  > trees); R3: 81 → 74 (the four root forks and three `Squest` files);
  > R5: 5,897 → 5,790 (the deleted gameserver sub-trees). Four
  > `tests/arch/baseline.txt` C1 entries for `mofus/testserver/` went too.
  >
  > Left for a later pass: commented-out `UserGateway`/`CSAuth` calls in
  > `ClientManager.cpp`, `IncomingPlayerManager.cpp`, `EventAuth.cpp` and
  > `GamePlayer.h`, and the `__ACTIVE_QUEST__` blocks in `NPC.cpp` and
  > `PlayerCreature.cpp` that still name the deleted `QuestBoard`.

## Appendix — measured inventory (2026-08-29)

- ~502k LOC across 4,271 C++ files. `Core` 149k (1,410 packet-prefixed files
  in its root), `gameserver` 120k, `skill` 103k / 1,031 files, `item` 51k,
  `quest` 23k (Lua-integrated). The legacy `chinabilling`, `theoneserver`,
  `updateserver` and `cacheserver` trees counted here were deleted on
  2026-09-05 (see "Legacy service cleanup").
- 433 packet types in `src/Core/Packet.h`
  (`grep -cE 'PACKET_(GC|CG|CL|LC|GL|LG|GS|SG|GG)[A-Z_]* *[,=]' src/Core/Packet.h`).
- Wire encryption: per-session encrypt code reorders field read/write order
  via `SHUFFLE_STATEMENT_*` (`src/Core/EncryptUtility.h`) — part of the
  contract, must be covered by golden fixtures.
- Client repo carries divergent hand-copies of all packet classes
  (`client/Client/Packet/{Gpackets,Cpackets,Lpackets,Rpackets,Types,Upackets}`);
  server `Core` handlers still contain `#ifdef __GAME_CLIENT__` vestiges of
  the shared origin.
- At this inventory date, existing `test*` directories were ad-hoc standalone
  test servers and CI was clang-format only. The current `tests/` suite and
  C++20 workflow were added subsequently.
- Sidecar reference (local clone at `../sidecar`): module split
  `sidecar-kernel` ← `sidecar-core` ← `sidecar-app`; enforcement vocabulary in
  `sidecar-kernel/src/test/java/.../kernel/arch/ArchitectureRules.java`;
  conventions in its `CLAUDE.md`.
