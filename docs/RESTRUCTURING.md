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
| R1 | `g_p*` global-singleton extern declarations | 34 | `grep -rE '^extern .*\* g_p' src --include='*.h' --include='*.cpp' \| wc -l` (332→331 on 2026-09-10 with the never-built `EventMonsterNameManager.h`, which redeclared `g_pMonsterNameManager`; 331→327 on 2026-09-10 with the never-built `EventBall.h` (two) and the commented-out `EffectBloodyWallLoader` and `EffectGrayDarknessLoader` declarations; a `default*Repository()` accessor is a function, not a global, so extractions do not move this number; 327→325 on 2026-09-13 with the never-built `item/SubInventory.h` (two); 325→319 on 2026-09-17 with six globals that were declared and never created (`g_pCombatSystemManager`, `g_pItemNumberManager`, `g_pHolyLandRaceBonus`, `g_pObjectRegistry`, `g_pSkillParentInfoManager`, `g_pZonePlayerManager`); 319→315 on 2026-09-17 with the four quest scripting managers, whose only readers were `quest/` and the composition root, and which `ObjectManager` now owns and registers on `de::GameContext`; the remaining nine of the 325→306 span went with the sources no target compiled; 306→285 on 2026-09-17 with the twenty-one `g_pEffect*Loader` globals that were declared and defined but never created and never read; 285→269 on 2026-09-17 with the sixteen `g_pEffect*Loader` globals `EffectLoaderManager` did create, which it now reaches through the `m_pEffectLoaders` table it was already filling; 269→266 on 2026-09-17 with `g_pMonsterNameManager`, `g_pWeatherInfoManager` and `g_pDynamicZoneFactoryManager`, which `ObjectManager` now owns and registers on `de::GameContext`; 266→265 on 2026-09-17 with `g_pBillingPlayerManager`, which went with the billing module; 265→178 on 2026-09-17 with the eighty-seven `g_p*Loader` item globals `ItemLoaderManager` created, which it now reaches through the `m_pItemLoaders` table it was already filling; 178→176 on 2026-09-17 with `g_pVolumeInfoManager` and `g_pDefaultOptionSetInfoManager`, which `ObjectManager` now owns and registers on `de::GameContext`; 176→89 on 2026-09-17 with the eighty-seven per-item-class `g_p<Class>InfoManager` globals `ItemInfoManager` created, which it now reaches through the `m_InfoClassManagers` table it was already filling; 89→88 on 2026-09-17 with `g_pLuckInfoManager`, whose class, global and bodies all sat inside a comment block; 88→72 on 2026-09-17 with sixteen managers `ObjectManager` creates that at most five files read — thirteen of them registered on `de::GameContext`, three that nothing outside `ObjectManager.cpp` reads left as plain members; 72→63 on 2026-09-17 with nine more: `g_pItemLoaderManager`, `g_pCastleSkillInfoManager`, `g_pTimeChecker` and `g_pGameServerGroupInfoManager`, four managers `ObjectManager` creates that at most five files read; `g_pObjectManager`, `g_pThreadManager` and `g_pClientManager`, which `GameServer` creates and now holds as members; `g_pConnectionInfoManager`, which `IncomingPlayerManager` creates and holds; and `g_pGameServer`, which only its own `main()` read and which is now a local there; 63→61 on 2026-09-17 with `g_pLoginServer` and `g_pSharedServer`, each read only by its own `main()` and now a local there; 61→49 on 2026-09-18 with the twelve managers a subsystem now takes from `de::GameContext`: the eleven `ObjectManager` creates that at most nine files read (`g_pItemMineInfoManager`, `g_pTimeManager`, `g_pPriceManager`, `g_pEffectLoaderManager`, `g_pAlignmentManager`, `g_pGlobalPartyManager`, `g_pCombatInfoManager`, `g_pMasterLairInfoManager`, `g_pBloodBibleBonusManager`, `g_pCoupleManager`, `g_pDynamicZoneManager`), now its members, and `g_pIncomingPlayerManager`, which `ClientManager` creates and holds; 49→48 on 2026-09-18 with `g_pLogClient`, which all three `main()`s assigned and only commented-out bodies read, now a `LogClient.cpp` static behind `openLogClient()`/`logClient()`; 48→38 on 2026-09-18 with ten globals the login and shared servers retired: the seven the loginserver's `LoginServer`, `ClientManager` and `LoginPlayerManager` now hold as members and register on `de::LoginContext` (`g_pGameServerGroupInfoManager`, `g_pGameServerManager`, `g_pLoginPlayerManager`, `g_pReconnectLoginInfoManager`, `g_pUserInfoManager`, `g_pZoneGroupInfoManager` and `g_pZoneInfoManager`; the first, second and last are the loginserver's own copies of names the shared server or the gameserver declares separately), and three nothing outside their creator reads, left as plain members: `g_pClientManager` and `g_pItemDestroyer` on `LoginServer`, `g_pHeartbeatManager` on `SharedServer`; 38→36 on 2026-09-22 with `g_pItemInfoManager` and `g_pFlagManager`, the two managers `ObjectManager` creates that the rest of the gameserver now takes from `de::GameContext` — the item info manager 130 files read, the flag war manager 14; 36→35 on 2026-09-22 with `g_pSkillInfoManager`, the skill info manager `ObjectManager` creates that the rest of the gameserver now takes from `de::GameContext` — 237 files read it, 221 of them under `skill/`; 35→34 on 2026-09-22 with `g_pZoneInfoManager`, which `ObjectManager` already registered on `de::GameContext` and now holds as a member: its twenty-nine remaining readers take it from the context) |
| R2 | Files with inline SQL in gameserver root | 0 | `grep -lE 'executeQuery' src/server/gameserver/*.cpp src/server/gameserver/*.h \| wc -l` (non-recursive on purpose: a `repository/` MySQL impl does not count — R2 measures SQL *leaving the game logic*. Textual, so a commented-out `executeQuery` still counts. Baseline 104 on 2026-08-29; 7→0 on 2026-09-10, the last two live sites into `PlayRecordRepository::logPlayerTrade` and the new `SMSMessageRepository`, `CreatureUtil.cpp`'s commented-out `addOlympicStat` body deleted, and four never-built stale copies deleted with it. The root is clean; new SQL there fails the ratchet.) |
| R3 | Files with inline SQL outside `database/` and any `repository/` | 0 | `grep -rlE 'executeQuery' src --include='*.cpp' \| grep -v 'server/database' \| grep -v '/repository/' \| wc -l` (18→11 on 2026-09-10 with the seven gameserver-root files R2 counted; 11→0 the same day with the never-built `EventBall.cpp`, the `*notice` command that held the last live statement, and the nine files whose only `executeQuery` sat inside a comment block. `gameserver/repository/` joined the exclusion on 2026-09-01, 317→314: a seam that quarantines four tables from two files would otherwise *raise* a shrink-only ratchet; the loginserver's, sharedserver's and ServerCore's `repository/` directories were admitted on 2026-09-07 before they existed, so the count did not move. Textual — see the comment policy under 3.2. Counts unbuilt files and the other binaries' game logic too.) |
| R4 | Packet headers with `execute()` still on the packet | 0 | `grep -rlE 'void execute\(Player' src/Core --include='*.h' \| wc -l` |
| R5 | `__BEGIN_TRY` control-flow macro sites in de-core candidates | 5,170 | `grep -rE '__BEGIN_TRY' src/server/gameserver --include='*.cpp' \| grep -vE 'gameserver/(gm\|handler\|packetfill)/' \| wc -l` (handler/ and packetfill/ hold 2.4-moved sources from `src/Core`, never counted while they lived there; `gm/` joined them with task 4.1, holding the GM command bodies that moved out of `handler/CGSayHandler.cpp` — the 33 macro pairs in them are the same handler bodies at a new address, so the number did not move. Fold them in with a re-baseline when they become 3.x extraction targets. 5,984→5,980 on 2026-09-02: the four macros inside the guild trio's deleted dead __SHARED_SERVER__ blocks. 5,980→5,899 on 2026-09-02, textual: ItemIDRegistry.cpp's 81 hand-expanded initItemIDRegistry bodies collapsed onto one macro, so the grep sees one #define line instead of 82 matched lines — 81 expansions plus the old macro's own; each method still has its try block. 5,897→5,790 on 2026-09-05: the never-built `gameserver/test/`, `testAlone/`, `mofus/testserver/` and `quest/Squest/` trees were deleted. 5,790→5,788 on 2026-09-08: the never-built `skill/Restore2.cpp`, a stale duplicate of `skill/Restore.cpp`, was deleted. 5,788→5,755 on 2026-09-08: the never-built `Vampire_backup.cpp`, a stale copy of `Vampire.cpp`, was deleted. 5,755→5,737 on 2026-09-10: the never-built `EventMonsterNameManager.cpp` (4), `GameServerInfoManager.cpp` (7) and `GameWorldInfoManager.cpp` (7) were deleted. 5,737→5,719 on 2026-09-10: the never-built `EventBall.cpp` (10) and `EventQuestRewardManager.cpp` (1) were deleted, and seven more sat in commented-out or empty bodies deleted from `mission/`, `skill/` and `war/`. 5,719→5,701 on 2026-09-13: the never-built `item/SubInventory.cpp` (10) and `war/SubInventoryItemPosition.cpp` (8) were deleted. 5,701→5,685 with the 4.3 hoist: 24 sites left `Slayer.cpp`/`Vampire.cpp`/`Ousters.cpp` with the bodies that moved to `PlayerCreature.cpp`, which carries 8 of them now that the three copies are one. 5,685→5,677 with the second 4.3 hoist: 12 sites left the three race files with `setGoldEx`, `getExtraInfo`, `getInventoryInfo`, `canPlayFree` and `isPayPlayAvaiable`, and `PlayerCreature.cpp` gained 4 of them — its own `isPayPlayAvaiable` already had one. 5,677→5,673 with the never-defined region macros: the two in EventShutdown.cpp's deleted branch. 5,673→5,483 on 2026-09-17: the thirty-nine sources no target compiled were deleted, and 190 of the sites sat in them. 5,482 → 5,474 with the never-defined feature macros: eight sat in the commented-out `NPC.cpp` SimpleQuest and `PlayerCreature.cpp` quest bodies that went with them. 5,473→5,437 with the dead billing module (32 sites) and the bodies that fed it: `GamePlayer::sendBillingLogin` (1), `PlayerCreature::isBillingPlayAvaiable` and `canPlayFree` (2), and `SkillUtil.cpp`'s empty `checkFreeLevelLimit` (1). 5,437→5,434 on 2026-09-17 with the deleted `LuckInfo.cpp`, whose three sites sat in its commented-out body, which this textual measure counts. 5,434→5,211 on 2026-09-17: 223 sites sat inside the commented-out bodies the R18 pass removed. 5,211→5,204 on 2026-09-18: seven more sat in the commented-out bodies deleted from `couple/`, `gm/`, `mission/` and `war/`. 5,204→5,201 on 2026-09-22: the vampire and ousters slot constructors, destructor and run-time bodies are three on their shared `skill/RaceSkillSlot.cpp` where they were six across the two race slot files. 5,201→5,193 on 2026-09-22 with the slot-table hoist: 12 sites left the three race files with the bodies that moved to `PlayerCreature.cpp`, which carries 4 of them now that the three copies are one; 5,193→5,170 on 2026-09-22: twenty-three more sat in the commented-out bodies deleted from the gameserver's top-level files) |
| R6 | Line count of god files (each tracked separately) | see table below | `wc -l <file>` |
| R7 | Files using parenthesized `throw(...)` syntax — dynamic specifications plus expressions, see 5.4 | 0 | `grep -rlE 'throw[[:space:]]*\(' src --include='*.h' --include='*.cpp' \| wc -l` (real throw expressions were normalized to `throw expr`, making every future match unambiguously forbidden legacy syntax) |
| R8 | Non-comment lines using `__PRETTY_FUNCTION__` | 0 | `grep -rh '__PRETTY_FUNCTION__' src --include='*.h' --include='*.cpp' \| grep -vcE '^[[:space:]]*//'` (call-site diagnostics take the enclosing function from a defaulted `std::source_location` — see docs/TOOLCHAIN.md, "Diagnostics without location macros". Line-based: a line whose first non-blank text is `//` is a comment, so the comments that explain the equivalence may still name the macro) |
| R9 | Hand-written length-prefixed string reads left in `src/Core` | 0 | `grep -rhE 'iStream\.read\([A-Za-z_][A-Za-z0-9_]*, sz[A-Za-z0-9_]*\);' src/Core --include='*.h' --include='*.cpp' \| grep -vcE '^[[:space:]]*//'` (a string field is a BYTE length then that many bytes; `de::wire::readString`/`writeString` in `src/Core/WireString.h` carry it with the bounds stated once. The read may land in a member or in a local, so any identifier counts. Line-based with R8's comment rule, so `WireString.h`'s own example of the shape it replaces does not count itself) |
| R10 | Throws of a pointer into a local string, and the handlers that caught one | 0 (R10a), 0 (R10b) | R10a: `grep -rnE 'throw [A-Za-z_]+\.c_str\(\)' src \| wc -l` — a `throw x.c_str()` hands the handler storage that dies with the clause it came from; `END_DB`/`END_DB_EX` answer a failed statement with a `DatabaseError` (`src/server/database/DatabaseError.h`) that owns its message, so nothing in the tree has that shape. R10b: `grep -rn 'catch (const char\*' src \| wc -l` — the receiving end. All 34 handlers name `DatabaseError`, so the count is 0 and one left behind would either be dead or be reaching for one of the ~160 bare `throw "literal"` sites, which are a separate defect and are not answered this way. Textual, so a commented-out clause counts (the one inside `CGPortCheckHandler.cpp`'s commented-out retry moved with the live code) |
| R11 | Bare string-literal throws left in `src` | 0 | `grep -rh 'throw "' src --include='*.h' --include='*.cpp' \| grep -vcE '^[[:space:]]*//'` — a `throw "text"` puts a `const char*` on the stack, a type nothing catches (R10b) and neither `__END_CATCH` nor the swallowing `__END_CATCH_NO_RETHROW` matches, so it walks past every handler the surrounding code wrote and lands in a `catch (...)` backstop — or, thrown out of a destructor, in `std::terminate`. `throw Error("text")` reaches the handler written for it. 149→0 over two rounds (see `docs/FIXES.md`): the 62 sites outside `src/server/gameserver/item`, then the 87 item constructors that answered a failed `isPossibleItem` check with the identical `Invalid item type or optionType` literal, one per item class. Line-based with R8's comment rule, so a commented-out throw does not count |
| R12 | Throw messages carrying non-ASCII text | 0 | ``LC_ALL=C grep -rhE $'throw[[:space:]]*[A-Za-z_][A-Za-z0-9_]*[[:space:]]*\\([[:space:]]*"[^"]*[^\x01-\x7f]' src --include='*.h' --include='*.cpp' | grep -vcE '^[[:space:]]*//'`` — a throw's message is a diagnostic read in a log or on a console, and the tree's code language is English; the legacy messages were code-page bytes that survived the migration as Korean, mojibake or U+FFFD runs. The class is spelled as the bytes it excludes so that the CR of a CRLF working tree is not counted, the way `[^[:print:]]` would; line-based with R11's comment rule, so a literal that starts on a continuation line is not counted |
| R13 | Duplicated include guard names across `src/**/*.h` | 0 | Each header's **first** `#ifndef` is its guard; the ratchet takes that name from every header (`find -exec awk`, stripping the CR of the CRLF tree) and counts `sort \| uniq -d`. Two headers sharing a guard means whichever one a translation unit reaches first silently swallows the other: the second `#include` expands to nothing and its declarations surface as an unrelated "undeclared identifier" far from the cause, which is how `EffectCallMotorcycle.h` reusing `EffectDecayItem.h`'s guard bit the `Zone` split. 16→0: eighteen headers took a name derived from their path, per-server twins (`sharedserver/GameServerInfo.h` → `__SHARED_SERVER_GAME_SERVER_INFO_H__`) included, because which headers share a binary changes. Not measured on every `^#ifndef` line — ordinary macros are tested that way too (`Core/Types.h` tests `__XMAS_EVENT_CODE__` before defining it), so that count could never reach zero |
| R14 | Mentions of macros nothing defines | 0 | `LC_ALL=C grep -rhE '__((THAILAND\|THIALAND\|CHINA\|CHAINA\|INTERNATIONAL\|NETMARBLE\|TEST)_SERVER\|OLD_GUILD_WAR\|CONNECT_BILLING_SYSTEM\|COUT_BILLING_SYSTEM\|PAY_SYSTEM_(ZONE\|LOGIN\|FREE_LIMIT)\|UNDERWORLD\|ACTIVE_QUEST\|ACTIVE_SERVICE_DEADLINE\|WINDOWS)__' src --include='*.h' --include='*.cpp' \| wc -l` — a macro no build defines makes the block behind it dead text: it never reached the compiler, so the `#else`/`#ifndef` branch beside it was the only code the servers ran, while the block kept reading as live code. A translation-unit-local `#define` arms one again, and one did, giving that TU a `SystemAvailabilitiesManager` with a layout no other TU shared. The region builds went first (`__THAILAND_SERVER__`, `__CHINA_SERVER__`, the misspellings `__CHAINA_SERVER__`/`__THIALAND_SERVER__`, `__INTERNATIONAL_SERVER__`, `__NETMARBLE_SERVER__`, `__TEST_SERVER__`); the feature macros followed — `__OLD_GUILD_WAR__` (guild union/tax handlers that only answered "not supported yet", a one-attacker war schedule where the live read takes five), `__CONNECT_BILLING_SYSTEM__`/`__COUT_BILLING_SYSTEM__` (the external billing link and its console trace), `__PAY_SYSTEM_ZONE__`/`__PAY_SYSTEM_LOGIN__`/`__PAY_SYSTEM_FREE_LIMIT__` (with none defined `GamePlayer::isPayPlaying()` answers true for every player and each gate passes), `__UNDERWORLD__`, `__ACTIVE_QUEST__`, `__ACTIVE_SERVICE_DEADLINE__` and `__WINDOWS__`, whose arm of the platform switch was the only dead one (`__LINUX__` comes from the top-level `CMakeLists.txt`, `__APPLE__` from the compiler). The instrumentation toggles a developer switches on by hand (`__PROFILE_*`, `__FULL_PROFILE__`, `__DEBUG_OUTPUT__`, `__OUTPUT_INIT__`) are deliberately not counted. Comments count too, so a comment about one of these branches states the behaviour instead |
| R15 | `src/**/*.cpp` that no target compiles | 0 | A source no target names is never compiled, so nothing it says is true of a running server: it drifts out of sync with the headers it includes while still reading as live code. Built means a `CMakeLists.txt` names it, `tests/arch/kernel_files.txt` lists it, or another source `#include`s it. A CMake reference is relative to its own `CMakeLists.txt`, so each is resolved against that directory — a basename match would call `gameserver/SocketImpl.cpp` built because the kernel list carries `Core/SocketImpl.cpp` — and comments are stripped first, since two quest sources sat behind a `#` in a source list. The scan is scoped to `src` plus the top-level `CMakeLists.txt` because the container configures its build tree inside the source root, and a `file(GLOB ...)` in the build files fails the check outright, a name-based measure being blind to a globbed target. 39→0 on 2026-09-17 |
| R16 | Headers under `src/` that nothing includes | 0 | A header no translation unit reaches is not part of any build, so nothing it declares is ever checked against the code it describes. Included means some `#include "..."` text under `src/` or `tests/` equals the header's path or the path ends with `/` plus that text. That approximates resolving each include against the including file's own directory and the CMake `-I` list, and it errs only toward calling a header used, never toward calling a live one dead; comments are not stripped, for the same direction. The include-text list is materialised first, as R13 does, so a broken grep fails loudly instead of reading as zero orphans. 5→0 on 2026-09-17 |
| R17 | Source lines carrying a non-ASCII byte | 578 | `LC_ALL=C grep -rhE $'[^\x01-\x7f]' src --include='*.h' --include='*.cpp' \| wc -l` - the tree's code language is English, and these are the lines a reader cannot read. The legacy text survived in three states: readable Korean, mojibake (EUC-KR/CP949 bytes decoded as Latin-1 and re-encoded as UTF-8), and U+FFFD runs where the text is gone and only the adjacent code says what it meant. Comments are translated tree by tree, and the last of them -- the gameserver's `skill`, `quest` and `item` trees -- is done, so no comment carries legacy text any more. String literals are a pass of their own: changing one changes what the server says, not how the source reads, so every one of the 578 lines this count still holds is a string literal, 88 of them in the gameserver's `skill` (14), `quest` (72) and `item` (2) trees and 490 in the rest of the tree. The byte class is R12's, so the CR of a CRLF working tree stays out of the count |
| R18 | Commented-out code lines inside multi-line `/* */` blocks | 5 | A character-level perl scan (see `tests/ratchet/ratchets.sh`) over every `src/**/*.{cpp,h}`: lines inside a multi-line `/* */` comment that read as a statement — ending in `;`, `{` or `}`, a preprocessor directive, opening with a control keyword or type name, or an identifier followed by `(`. Switched-off code names deleted globals and signatures that changed, and reads as if it described the code beside it. The same test over `//` lines called 13 of 50 sampled prose lines code — section banners, end-of-block markers, sentences opening with `delete` — so only `/* */` blocks are counted; `//` commented-out code is removed without being measured. 10639→5281 on 2026-09-17. 5,279→3,301 on 2026-09-18, with `src/Core`, `src/server/database`, the login and shared servers, the files directly under `src/server` and the gameserver’s `handler/`, `gm/`, `war/`, `mofus/`, `mission/`, `couple/`, `ctf/`, `packetfill/`, `repository/` and `exchange/` trees. 3,301→5 on 2026-09-22 with the gameserver's top-level files, `src/server/LogClient.cpp`, `src/domain/SkillOutputFormulas.cpp` and three scattered files; the five lines left are the two block comments in `handler/CGWhisperHandler.cpp` that describe the whisper relay field by field, documentation that only reads as code |

God-file baselines (R6):

All rows re-measured 2026-08-31 post-clang-format-18 (the 08-29 numbers
predated that pass) and again 2026-09-05; only the rows `ratchets.sh` names
are enforced so far.

| File | Baseline lines |
|------|---------------:|
| `src/server/gameserver/Zone.cpp` | 1,274 (was 9,350 before the 4.2 extractions; enforced by `ratchets.sh` R6g) |
| `src/server/gameserver/skill/SkillUtil.cpp` | 685 (was 6,626 before the split by concern into `SkillDamage.cpp` / `SkillExperience.cpp` / `SkillGeometry.cpp`, leaving the mana and HP costs, the slot run-time and zone-level gates, the skill-failure packets and the elemental lookups; under the 2,000-line phase exit criterion, so R6a pins it rather than baselining a god file; enforced by `ratchets.sh` R6a) |
| `src/server/gameserver/InitAllStat.cpp` | 230 (was 4,787 before the split by race into `SlayerStat.cpp` / `VampireStat.cpp` / `OustersStat.cpp`, leaving `PlayerCreature::applyBloodBibleSign` and `Monster::initAllStat`; under the 2,000-line phase exit criterion, so R6b pins it rather than baselining a god file; enforced by `ratchets.sh` R6b) |
| `src/server/gameserver/handler/CGSayHandler.cpp` (moved from `src/Core` in 2.4) | 114 (was 4,720 before the 4.1 command extraction; enforced by `ratchets.sh` R6e) |
| `src/server/gameserver/gm/ConsoleCommands.cpp` | 1,575 (the 61 `*command` sub-command bodies, one function per name; enforced by `ratchets.sh` R6f) |
| `src/server/gameserver/Slayer.cpp` | 3,087 (was 4,046 before the 4.3 hoists, 3,516 before the commented-out code went; enforced by `ratchets.sh` R6h) |
| `src/server/gameserver/Vampire.cpp` | 2,047 (was 2,783 before the 4.3 hoists, 2,235 before the commented-out code went; enforced by `ratchets.sh` R6i) |
| `src/server/gameserver/Ousters.cpp` | 1,954 (1,959 before an empty sight override left by the commented-out code went; was 2,548 before the 4.3 hoists, 2,117 before the commented-out code went; enforced by `ratchets.sh` R6j) |
| `src/server/gameserver/skill/SkillFormula.cpp` | 818 (was 3,081 before the 3.3 computeOutput extraction — now thin adapters + the 11 dice-roll formulas; enforced by `ratchets.sh` R6d) |
| `src/server/gameserver/skill/HitRoll.cpp` | 643 (not a god file — an extraction-target pin, locked in with its 3.3 extraction; enforced by `ratchets.sh` R6c) |

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

- [x] **1.2 Packet round-trip tests with golden byte fixtures.** For each
  packet direction (start GC/CG, then LC/CL, then inter-server), a test that
  constructs a canonical instance, `write()`s it, compares against a
  checked-in golden byte file, then `read()`s it back and compares fields.
  Must cover the `__USE_ENCRYPTER__` path for every `code % 3` shuffle branch
  of `SHUFFLE_STATEMENT_*` (`src/Core/EncryptUtility.h`) — the shuffle *is*
  part of the wire format.
  > **Status:** done — every registered packet factory has a checked-in
  > golden byte fixture, a loopback round trip comparing every getter,
  > and a pin of `getPacketSize()` against the bytes `write()` emits and
  > against the factory's `getPacketMaxSize()`. Coverage is proved
  > mechanically rather than claimed: the factory names in
  > `tests/ratchet/factory_registrations.txt`, 449 unique across the
  > three servers, minus the packet names under `tests/golden/`, is
  > empty, and no packet in it is pinned only through another packet's
  > variant golden.
  >
  > The conventions a new packet must follow: fixture values distinct
  > per field and >= 128 in every byte the width allows, with every
  > exception named at the point of use (enumerators, bools, text,
  > range-limited fields — the Debug toolchain traps an out-of-range
  > enum load); a golden per `write()` branch a single fixture cannot
  > reach; goldens recorded at encrypt code 0 for a packet that does not
  > touch the encrypter, with the test also asserting the bytes do not
  > vary with the code, so adopting the encrypter fails loudly instead
  > of silently voiding the pin; per-code goldens at 0..5 for one that
  > does, which `tests/ratchet/ratchets.sh` demands of any new encrypter
  > user (`tests/ratchet/encrypter_exceptions.txt` holds the abstract
  > base); a poisoned-storage constructor pin per file, splitting the
  > set into the packets that initialise every member they write and the
  > packets that do not; and every write/read disagreement stated as a
  > test that FAILS once the packet is fixed, never worked around in the
  > fixture. Goldens are re-recorded deliberately with
  > `UPDATE_GOLDENS=1 ./bin/wire_tests`
  > (`bash tools/devbuild.sh test --record`); a changed golden is a
  > protocol change the client repo must ship identically.
  >
  > One file per family, each with its own header listing the packets,
  > the exclusions and the findings: the encrypter shuffle paths at
  > codes 0..5, all 19 users, which reach every `code % N` case of every
  > `SHUFFLE_STATEMENT_N` (`tests/packet_encrypter_test.cpp`); the CL/LC
  > login phase, 33 (`tests/packet_login_test.cpp`); the gameserver
  > handshake, 12 (`tests/packet_gameserver_handshake_test.cpp`); the
  > zone population scan, 18 (`tests/packet_zone_scan_test.cpp`); both
  > inter-server links, 30, the datagram half through a real `Datagram`
  > (`tests/packet_interserver_test.cpp`, with the socket hop in
  > `tests/datagram_frame_test.cpp`); the party, guild and trade
  > protocols, 47 (`tests/packet_party_test.cpp`,
  > `tests/packet_guild_test.cpp`, `tests/packet_trade_test.cpp`);
  > combat feedback, 38 (`tests/packet_combat_test.cpp`); movement,
  > effect lifecycle and NPC dialogue, 26
  > (`tests/packet_movement_test.cpp`); inventory and item handling, 37
  > (`tests/packet_inventory_test.cpp`); store, shop and stash, 36
  > (`tests/packet_store_test.cpp`); character progression, 29
  > (`tests/packet_skill_test.cpp`); quest, war and zone selection, 27
  > (`tests/packet_quest_war_test.cpp`); chat, notice, nickname, union
  > and SMS, 31 (`tests/packet_chat_test.cpp`); creature state, 42
  > (`tests/packet_creature_test.cpp`); the session and take-out set, 18
  > (`tests/packet_session_test.cpp`). The four packets pinned first
  > (`GCMoveOK`, `CGMove`, `CGSay`, `CGWhisper`), the framed-bytes
  > golden and the header-width assertions stay in
  > `tests/packet_roundtrip_test.cpp`.
  >
  > Every family turned up write/read disagreements. All are fixed and
  > pinned as the behaviour the packets now produce; the one that cannot
  > be fixed — `CGPortCheck`'s registration, which the game server's UDP
  > path needs — is recorded in its file's header. The fixes moved
  > `tests/wire-layout.txt` lines in most families, every one a
  > server-side read-buffer budget rather than a field on the wire, and
  > one golden pair (`GCMakeItemOK`, one byte shorter: a duplicated
  > option count the client never read). `GCExecuteElement`'s golden was
  > re-recorded for a fixture that carried an out-of-range condition
  > byte.
  >
  > The four fixtures pinned first carry high-bit coordinates and
  > colours with the direction an enumerator, and a test reads their
  > recorded goldens back so they stay that way.
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
  > `m_Agree` byte sat behind `__NETMARBLE_SERVER__`, which nothing
  > defines; the byte and the macro are both gone now (R14); the
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
  > mofus/exchange) must not include MySQL, Lua, or
  > socket-transport headers. K rules have no baseline (the list is
  > defined as what complies); C1's remaining pre-existing violations are
  > frozen shrink-only in `tests/arch/baseline.txt` (the two `mofus/`
  > Socket.h users — Player-transport classes, 2.3's problem).
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
  > The handler classes themselves are still defined in the packet headers:
  > 196 `src/Core/*.h` carry a `class XHandler { ... };`, and for 193 of them
  > that is the only definition anywhere — no `src/server/*/handler/`
  > directory holds a header at all, so the 194 `handler/*.cpp` define their
  > members against the Core declaration (`CGConnectSetKeyHandler` twice, in
  > the gameserver and the loginserver). The other three, `CGDialUpHandler`,
  > `CGPhoneDisconnectHandler` and `CGPhoneSayHandler`, have no
  > implementation left. The client repo mirrors 167 of the 196 headers and
  > 42 of its copies carry a handler class of their own, so moving the
  > definitions out is a two-repo change.
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
  > 9. **The dead phone exchange is deleted**: `CGDialUp`,
  >    `CGPhoneDisconnect` and `CGPhoneSay` have no factory in any of
  >    `PacketFactoryManager::init()`'s lists, so `createPacket` answers
  >    their ids with an `InvalidProtocolException` and the dispatch
  >    entries the gameserver registered for them could never run; their
  >    three handler sources, the registrations and the CMake entries are
  >    gone, and with them the only senders of `GCRing`,
  >    `GCPhoneConnected`, `GCPhoneConnectionFailed`, `GCPhoneDisconnected`
  >    and `GCPhoneSay`. The packet classes stay in `src/Core`, which the
  >    client repo mirrors.
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

- [x] **3.2 Repository extraction.** Pull inline `executeQuery` SQL out of
  game logic into `*Repository` classes behind interfaces (game logic takes
  the interface; the app wires the MySQL implementation). Sidecar convention
  adopted verbatim: **legacy schema quirks live in the repository on
  purpose** — zero-date columns, denormalized tables, encoding oddities are
  quarantined and documented *there*, never leaked into domain types. Order
  of attack: `PlayerCreature`/`Slayer`/`Vampire`/`Ousters` persistence first
  (biggest testability win), then Zone, then the long tail. Ratchets R2/R3.
  > **Status:** done (2026-09-22) — R2 104→0 and R3 317→0. No `executeQuery`
  > is left anywhere in `src` outside `src/server/database/` and the
  > `repository/` directories, so the two ratchets now hold the seam shut
  > rather than track it shrinking: a statement written back into game logic
  > fails `tests/ratchet/ratchets.sh`. 44 seams — 36 under
  > `src/server/gameserver/repository/`, two in ServerCore
  > (`src/server/repository/`, compiled into all three binaries), four in the
  > loginserver and two in the sharedserver — each an interface
  > `*Repository.h` with a `MySQL*Repository.cpp` implementation, reached
  > through `default*Repository()` accessors, never `g_p*` externs.
  >
  > **Extending a seam: the header is the authority.** Each repository
  > header carries its tables' quirks and an explicit **"not enclosed"**
  > list of the SQL on the same tables the seam does not cover. Read that
  > header before adding a method, and grep the whole tree (loginserver/
  > and sharedserver/ included) before rewriting the list.
  >
  > **Seams:** `BalanceInfo`, `GameInfo`, `ContentInfo`, `ZoneInfo`,
  > `QuestInfo` — the read-only boot-time catalogues (exp/attr ladders,
  > skill/monster/NPC/script/option info, zone config incl. ZoneEffectInfo,
  > quest catalogues); `Character` (race-table loads, saves, tinysave),
  > `Gold`, `Stash`, `SkillSave`, `RankBonus`, `BloodBibleSign` (read-only),
  > `FlagSet`, `EffectSave` (the persisted effects), `SMSAddress`,
  > `QuestItem`, `PlayRecord`, `Goods`, `Nickname` — per-character state;
  > `ItemObject` (every `*Object` table's create/save/load/destroy behind a
  > per-table spec row with object-shape and info-shape enums — a loader
  > refuses a table of the wrong shape) and `Item` (trace logs, counters,
  > UniqueItemInfo, TimeLimitItems, the ItemID registry probes); `Guild`,
  > `Couple`, `Friend`, `Message`, `Session` (login/logout bookkeeping on the
  > account and GuildMember rows), `WarInfo` (shrines, castles, sweepers,
  > schedules, histories, reinforcement, race-war limits), `FlagWar`,
  > `RegenZone`, `BulletinBoard`, `ComebackEvent`, `MofusPoint`,
  > `SystemAvailability`, `SpecialEvent` (the one seam on the world-default
  > connection — `getConnection(int)`, see its header), `SMSMessage` (the SMS
  > relay's three statements, on the `SMS_DB_*` connection it owns itself
  > because the relay is not one of DatabaseManager's servers),
  > `CharacterPurge` (deletePC's 109-statement character-deletion list, one
  > method, one Statement, in the original order — every table on it is
  > another seam's, and those headers say so); `Exchange` (the Exchange
  > feature's own access class relocated under the convention:
  > ExchangeListing, ExchangeOrder, the AccountPoint / PointLedger
  > statements that ask for a USERINFO connection and reach DARKEDEN, and
  > the transaction pair — the header says what each does against the
  > shipped schema). In ServerCore, under `src/server/repository/`:
  > `PayPlay` (PaySystem's Player pay-play columns and the PC-room tables,
  > on the dist connection) and `ServerInfo` (GameServerInfo with its
  > NonPKServerList and CastleStatInfo flags, and WorldInfo). In the
  > loginserver, under `src/server/loginserver/repository/` with a `Login`
  > prefix (the integration binary links every impl, so names must not
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
  > **Conventions a new method follows** (each one cost a review finding):
  > - Statements move **byte-for-byte**, quirks included (backticked
  >   `Rank`, mixed-case keywords, copy-paste whitespace). Moving SQL is
  >   not fixing SQL. A bug found in a statement is recorded in the
  >   header, pinned by a test, and gets its own PR.
  > - Repository SQL uses the parameterized `executeQuery` form, never
  >   string concatenation; StringStream chains become format strings
  >   with the same bytes. `executeQueryString` survives only where the
  >   statement takes no arguments, must be logged verbatim, or can
  >   outrun the 2,048-byte format buffer in `Statement::executeQuery` —
  >   the one case is `PlayRecordRepository::logPlayerTrade`, whose
  >   TradeLog row is as long as the two traded inventories make it, and
  >   the MySQL tier pins it past that width.
  > - Every `executeQuery` conversion is the argument's own width: `%u`
  >   for DWORD/WORD/BYTE, `%d` for int, `%ld` for `time_t`. A 32-bit
  >   argument read through `%ld` takes whatever the upper half of the
  >   register holds under the pinned Clang toolchain — a Fame of 777
  >   landing as 4294967295, an `ItemID=%ld` UPDATE matching no row.
  >   `Statement::executeQuery` carries
  >   `__attribute__((format(printf, 2, 3)))` and the build compiles with
  >   `-Wformat` (top-level `CMakeLists.txt`), so the compiler now says
  >   so — except for a format reached through a pointer (the per-table
  >   spec rows in the ItemObject, EffectSave and ComebackEvent seams),
  >   which only the tier covers.
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
  > - The Statement is freed on every path, success or failure:
  >   `END_DB`, `END_DB_EX` and `MySQLSMSMessageRepository`'s
  >   `END_DB_RETHROW` carry a catch-all that deletes it and rethrows
  >   unchanged. A failed statement crosses the seam as a
  >   `DatabaseError` (`src/server/database/DatabaseError.h`) that owns
  >   its `DBError.log` line, and that line names the repository method,
  >   not the caller.
  > - **Integration tier over fakes**: `mysql_repository_tests`
  >   (`tests/integration/`, `make integration-test`, needs docker) runs
  >   the real impls against MySQL 5.7 loaded with `initdb/` and the
  >   production sql_mode. A quirk is replayed there before it is
  >   written down — the first rounds' fakes documented three behaviours
  >   the server refuted. Only the six pilot-era seams keep a fake
  >   (`tests/support/`). A seam whose callers are compiled out still
  >   gets a test: the compiler will never check them.
  > - R2/R3 are **textual** greps, so a commented-out `executeQuery`
  >   counts. A commented-out block that names code the conversion
  >   deleted is rewritten to name the seam method (it is otherwise
  >   wrong); a self-contained one is deleted, with the dead function
  >   around it when nothing calls it.
  >
  > **Pre-existing bugs found, pinned by the tier, deliberately not
  > fixed** (each named in its header):
  > - `FriendList`/`FriendHistory` are not in `initdb/`; opening the
  >   friend list disconnects the client.
  > - The FlagWar roll-up's GROUP BY is refused under ONLY_FULL_GROUP_BY
  >   (1055), so the first flag war to end takes the process down. Dead
  >   by config: `ActiveFlagWar` is 0 in both shipped confs.
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
  > - GuildUnionOffer's PK is OwnerGuildID alone, so an ESCAPE insert
  >   over a standing JOIN/QUIT row throws out of CGQuitUnionHandler.
  > - ActionShowGuildDialog gates guild creation on a hardcoded seven
  >   days ahead of the handler's QUIT_GUILD_PENALTY_TERM.
  >
  > `MySQLSMSMessageRepository` is the one seam the tier cannot pin:
  > `uds_msg` and `msg_queue` are not in `initdb/`, and its thread is
  > live but dormant (`GameServer::start()` does not start
  > `SMSServiceThread`).
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
  > and live-state gates stay in the adapters; the China-build
  > variants went with their never-defined macro; `isCriticalHit`'s additive
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
  > override; the China table is selected by no build now that
  > its macro is gone), Sniping's divide-first percents, the four slayer
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
  > etc. — no computation). The China liveness adapter went with
  > its never-defined macro; it was hand-compiled clean in the
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
  > **Status:** in progress — `src/server/gameserver/GameContext.h` is a
  > registry of non-owning pointers to fifty-one managers, each registered by
  > the code that creates it (`GameServer`, `ObjectManager`, `ClientManager`,
  > `IncomingPlayerManager`), read back through an accessor that asserts it is
  > there, a null one being a startup-order bug, not a condition to branch on.
  > Ownership is untouched: the same `new` and `SAFE_DELETE` sites; a manager no
  > global names is a member of its creator, registered only if something
  > outside that file reads it. `src/server/loginserver/LoginContext.h` is the
  > same for the loginserver's seven, registered by `LoginServer`,
  > `ClientManager` and `LoginPlayerManager`. `ctf/` and `quest/` are the
  > converted subsystems: `FlagManager`, `ActionFactoryManager`, `Trigger` and
  > `TriggerParser` take the context in their constructors and every `Action`
  > gets it from its factory; none of those calls `de::gameContext()`, the shim creation
  > sites and unconverted callers use. `game_context_tests` and
  > `login_context_tests` build a context over stand-in pointers with no server
  > linked, which the forward-declaration-only headers are for. R1: 325 → 34.
  > An `extern` line goes when nothing creates the global, when its owner
  > reaches its objects through a table it already fills (`EffectLoaderManager`,
  > `ItemInfoManager`), when its defining module owns it behind an open/read
  > pair (`LogClient`), or when its last caller is converted — for a server
  > object, its own `main()`, holding it as a local. Next: the rest of the
  > gameserver's own set, largest first — `g_pStringPool` (79 files) and
  > `g_pGuildManager` (79), each a name the sharedserver declares separately,
  > `g_pVariableManager` (79), `g_pItemFactoryManager` (52),
  > `g_pZoneGroupManager` (42), `g_pPCFinder` (44), `g_pCastleInfoManager` (43),
  > `g_pWarSystem` (36), then the fourteen smaller ones, the sharedserver's own
  > set and the `src/Core` externs; `g_pPacketValidator` and the
  > `g_pGameServer*Manager` twins fit none: each server creates one of its own.
  - Owner: R1 ratchet test.


**Phase exit criteria:** no hard gate — this phase *is* the ratchets trending
down. The 3.2 checkpoint is passed: R2 reached 0, 3.2 is closed, and R3 is
re-baselined at 0 with it. Both hold rather than shrink now — an
`executeQuery` anywhere outside `src/server/database/` and the `repository/`
directories fails `tests/ratchet/ratchets.sh`. R1 and R5 are the phase's
remaining trend lines.

---

## Phase 4 — God files behind routers

- [x] **4.1 GM-command router for `CGSayHandler.cpp`** (4,904 lines). A
  `CommandRouter` with one class/function per GM command and **gating
  declared at registration** — sidecar's `on(name, limiter, handler)` /
  `onGated(...)` insight: hand-kept permission maps drift silently, so the
  registration site is the single source of truth. A test enumerates
  registered commands and asserts each declares a permission level.
  > **Status:** done — `src/server/gameserver/gm/`. `CommandRouter` matches a
  > name as a prefix of the chat message and passes over a command whose
  > level the caller is below, leaving a later matching name its turn;
  > `CommandRegistration.cpp` is the one place a name, the number of
  > characters it is compared over and its permission are written down (34
  > operator rows, four broadcast prefixes; `*billing disconnect` went with
  > the billing module, so that name is now an unknown command rather than a
  > silent no-op). The command bodies are free functions in `de::gm`,
  > grouped by what they touch across
  > `ServerCommands` / `PlayerCommands` / `ZoneCommands` / `ItemCommands` /
  > `GuildCommands` / `ConsoleCommands.cpp`; `CGSayHandler::execute` keeps
  > the leading-`*` test and hands the message to the two tables.
  > `tests/gm_command_router_test.cpp` links the real registration over
  > stubbed bodies and pins every row — name, length, gate and order — so a
  > gate change has to be written twice. The levels are God / DM / Helper /
  > Everyone, ordered as `Competence` is. Three alias names left mojibake
  > by the code-page move are registered unreachable, each longer than the
  > length it is compared over. `*command` carries a second table of its
  > own: `SubcommandTable` matches the word after it whole rather than as a
  > prefix, `ConsoleCommandRegistration.cpp` is the one place its 61 rows and
  > their gates are written down, the bodies are one function per name in
  > `ConsoleCommands.cpp`, and `tests/gm_console_command_test.cpp` pins the
  > rows the same way. A third table, `RelayCommandRegistration.cpp`, names
  > the twelve commands another game server may relay - the only place
  > `opmodifyunioninfo` and `oprefreshguildunion` are named - and
  > `GGCommandHandler::execute` keeps the host/port logging and hands the
  > relayed message to it. A relayed message carries no player, so each
  > sub-command row declares whether its body is correct without one and
  > `SubcommandTable::dispatch` passes over the 40 that are not, the way it
  > passes over a gate the caller is below.
  - Owner: router registration + the enumeration tests; R6e and R6f
    ratchets.

- [x] **4.2 Split `Zone.cpp`** (9,263 lines) by concern: movement, broadcast,
  spawn/despawn, scan/visibility, persistence (→ 3.2 repository). Mechanical,
  many small commits, each verified by build + smoke.
  > **Status:** the zone is six translation units. `ZoneBroadcast.cpp` holds
  > the three `broadcastPacket` overloads, `broadcastDarkLightPacket`,
  > `broadcastSayPacket`, `broadcastLevelWarBonusPacket`,
  > `broadcastSkillPacket`, `movePCBroadcast` and `moveCreatureBroadcast`;
  > `ZoneScan.cpp` holds `scan`, `scanPC`, `monsterScan`, `updateScan`, the
  > four `update*Scan` refreshes and `getWatcherList`; `ZoneMove.cpp` holds
  > `pushPC`, `movePC`, `moveCreature`, `moveFastPC` and `moveFastMonster`
  > with the `g_FastMoveSearch` step tables; `ZoneLoad.cpp` holds `init`,
  > `load`, `reload`, `loadItem`, `loadTriggeredPortal`, `initSpriteCount`,
  > `loadNPCs` and `loadEffect`; `ZoneSpawn.cpp` holds the gated gateways
  > (`addPC` both overloads, `replacePC`, `addCreature`, `deleteCreature`,
  > `addCreatureToTile`, `deleteCreatureFromTile`) with `deletePC`,
  > `deleteQueuePC`, `deleteObject`, `createMonsterAddPacket`, `deleteNPC`,
  > `deleteNPCs`, `killAllMonsters`, `killAllMonsters_UNLOCK` and
  > `killAllPCs`; `ZoneItem.cpp` holds the ground-item tables — `addItem`,
  > `deleteItem`, `getItem`, `addToItemList`, `deleteFromItemList`, the
  > delayed and transport variants, `addRelicItem`, `deleteRelicItem`,
  > `addVampirePortal`, `deleteMotorcycle` and the commented-out
  > `decayMotorcycle` — with the master-lair decay constants, which nothing
  > else uses. They are still `Zone::` members with unchanged bodies — only
  > the translation unit differs — and the file-scope helpers more than one
  > unit calls (`isPotentialEnemy`, `sendRelicEffect`, `strlwr`) are declared
  > in `ZoneInternal.h`.
  > `Zone.cpp` 9,350 → 1,472, pinned by `ratchets.sh` R6g: the
  > constructors, the tile/sector/level accessors, the effect managers,
  > `getCreature`, the NPC info registry, `heartbeat`, `toString`, the
  > safe-zone and dark-light resets, the war and pay tails and the load
  > value. That is under the 2,000-line phase exit criterion, so the
  > `Zone.cpp` half of it holds.
  - Owner: R6 ratchet per extracted file.

- [ ] **4.3 Race-class cleanup.** `Slayer.cpp`/`Vampire.cpp`/`Ousters.cpp`
  share large duplicated blocks; factor shared behavior toward
  `PlayerCreature` or free functions as formulas from 3.3 make the
  differences explicit.
  > **Status:** in progress — fifteen members are defined once on
  > `PlayerCreature`: `tinysave`, `setGold`, `setGoldEx`, `increaseGoldEx`,
  > `decreaseGoldEx`, `checkGoldIntegrity`, `checkStashGoldIntegrity`,
  > `setResurrectZoneIDEx`, `saveAlignment`, `getIP`, `getItemShapeColor`,
  > `getExtraInfo`, `getInventoryInfo`, `canPlayFree` and
  > `isPayPlayAvaiable`, with `m_Gold`/`getGold()` beside `m_StashGold`. Two
  > seams carry what those bodies differed by: `characterRaceOf(getRace())`
  > (`PlayerRace.h`, pinned by `player_race_tests`) names the race table a
  > persistence body writes to, and `isWithinFreePlayLimit()` is the free-play
  > measure — one line per class, a level for Vampire and Ousters, the
  > skill-domain sum for Slayer. `Slayer::setGoldEx` stays as an override
  > because it writes `Gold = %u` where the shared body writes `Gold=%u`.
  > The skill-slot type is the first of the per-race types to be reconciled:
  > `VampireSkillSlot` and `OustersSkillSlot` now derive from
  > `skill/RaceSkillSlot.h`, which holds what they spelled identically — the
  > name, skill type, interval, casting time and run time with their
  > accessors, `getRemainTurn` and both `setRunTime` overloads. Each race
  > class keeps only what its own table needs: the `create`/`save` pair, plus
  > `destroy` and the `ExpLevel` for Ousters. On the back of that, four
  > slot-table bodies are defined once on `PlayerCreature` as member
  > templates over the race's map — `findSkillSlot`,
  > `removeCastleSkillSlot`, `removeAllCastleSkillSlots` and
  > `saveSkillSlots` — and all three races, Slayer included, delegate to
  > them in one line. Slayer's `SkillSlot` is deliberately not under
  > `RaceSkillSlot`: it carries exp, exp level and an enable flag, its
  > `getSkillType()` is const and its `setRunTime(Turn_t, bool)` takes a
  > second argument, so deriving would hide three base members across the
  > ~500 handlers that take a `SkillSlot*`.
  > What is still written three times is each race's wear, persistence-record
  > and load code, plus the two `addSkill` overloads, whose bodies differ for
  > real: Vampire alone does not assert on `SKILL_HOWL`, Ousters seeds
  > `ExpLevel` 1 and Slayer `Exp` 1 with `ExpLevel` 0, each logs its own
  > error file, and Slayer refuses to delete a duplicate that is already the
  > mapped slot. The rest are character-for-character identical in Vampire
  > and Ousters, but only after substituting a type the two do not share — a
  > class-scoped `WearPart` enum whose members differ in name and value, or a
  > `VampireExpsRecord`/`OustersExpsRecord` with its own repository method —
  > so the next shrink is a reconciliation of one of those two types, not
  > another hoist. Line counts pinned by `ratchets.sh` R6h/R6i/R6j,
  > `__BEGIN_TRY` sites by R5.
  - Owner: R6h/R6i/R6j ratchets; `player_race_tests`; `race_skill_slot_tests`.

**Phase exit criteria:** every GM command behind the router with declared
gating; `Zone.cpp` under 2,000 lines.

---

## Phase 5 — Process scaffolding (runs alongside all phases)

- [x] **5.1 Upgrade server CLAUDE.md to sidecar format.** From build manual to
  "non-obvious rules and gotchas that cost time to rediscover", each rule
  pointing at the test that owns it. Add the `make test` loop, the ratchet
  rules, and this document's status conventions.
  > **Status:** done (2026-09-22) — CLAUDE.md opens with a "Rules and the
  > tests that own them" table (fourteen rules, each with its owning test or
  > ratchet and the message it prints) and a "Working in this repository"
  > section carrying the `make dev-test` / `make test` loop, the ratchet
  > discipline, this document's `> **Status:**` conventions and
  > `docs/FIXES.md`. The build, container, database and run-the-servers
  > manuals stay; task and phase numbers are out of that file, because it is
  > read by sessions that never open this one. Same length as before (538
  > lines). Every claim was re-checked against the tree: the corrections were
  > the handler header that does not exist (`handler/` holds only
  > `*Handler.cpp`; the class is declared in the packet's Core header), the
  > four `FactoryList`s that are never four per server, the missing
  > directories (`gm/`, `guild/`, `party/`, `trade/`, the three
  > `repository/` trees outside the gameserver, `third_party/argon2`), the
  > thread backends that are one (`ManagedThread`), and the clang-format file
  > count nothing measures. Keep it that way: a sentence in CLAUDE.md that no
  > file or command backs is the failure mode this task exists to fix.

- [x] **5.2 `.claude/skills/add-packet` skill.** Modeled on sidecar's
  `add-sidecar-domain`: the checklist for adding/changing a packet — layout
  inventory regenerated in *both* repos, golden fixture added, shuffle
  branches covered, handler registered at the composition root (never on the
  packet), client-repo counterpart commit linked.
  > **Status:** done (2026-09-22) — `.claude/skills/add-packet/SKILL.md`, ten
  > steps derived from what the tests already enforce rather than from
  > habit: the `Packet.h` enumerator (appending is cheap, inserting shifts
  > every later id in both repos), the factory's `kPacketID`/`kName`/
  > `kMaxSize` and the `de::PacketFactoryType` concept, kernel membership
  > in `tests/arch/kernel_files.txt` with K1/K2, the per-server
  > `FactoryList` plus regenerating `tests/ratchet/factory_registrations.txt`,
  > the handler at the composition root through
  > `DE_REGISTER_PACKET_HANDLER` (and the `PacketValidator` set, needed only
  > before `GPS_NORMAL`), `tests/tools/gen_factory_list.sh`, the golden
  > fixture and its recording rule, the per-code goldens a shuffled packet
  > needs, the inventory re-record in this repo and in the client's, and the
  > `wire_inventory_diff.sh` cross-check with the counterpart commit linked
  > in the PR. Every step names the file or command that proves it; steps
  > that no check enforces say so (a plain packet's missing golden is the
  > one). `.claude/skills/` is out of `.gitignore` so the skill is checked
  > in, like the scripts under `tests/`.

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
  > `PaySystem` remains intact.
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
