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
| R2 | Files with inline SQL in gameserver root | 10 | `grep -lE 'executeQuery' src/server/gameserver/*.cpp src/server/gameserver/*.h \| wc -l` (glob is deliberately non-recursive: a `repository/` MySQL impl doesn't count here — R2 measures SQL *leaving the game logic*. 101→98 on 2026-09-01: the three race files. The grep is textual, so a commented-out `executeQuery` still counts — the character-load round deleted the dead comment blocks that would otherwise have held the number. 98→85 the same day: the eight persisted-effect files, FlagSet, SMSAddressBook, GQuestInventory and the two quest-item elements. 85→75 the same day, the Zone milestone: Zone, ZoneGroupManager, ZoneUtil, ZoneInfo, ZoneInfoManager, ZonePlayerManager, RegenZoneManager, ResurrectLocationManager, WayPoint, ThreadManager. 75→61 the same day, the balance/info loaders: AttrBalanceInfo, VampEXPInfo, OustersEXPInfo, RankEXPInfo, SkillDomainInfoManager, FameLimitInfo, PetExpInfo, PetAttrInfo, SkillParentInfo, RankBonusInfo, PetTypeInfo, GameServerGroupInfoManager, BloodBibleBonusManager, MonsterNameManager. 61→44 the same day, the config loaders: WeatherInfo, StringPool, ShopTemplate, PKZoneInfoManager, LevelWarZoneInfoManager, LevelNickInfoManager, ItemMineInfo, ItemGradeManager, GoodsInfoManager, EventZoneInfo, DefaultOptionSetInfo, DarkLightInfo, CastleSkillInfo, CastleShrineInfoManager, EffectOnBridge, MonsterManager, LogNameManager — not gameserver/GameWorldInfoManager.cpp, an unbuilt stale fork of ServerCore's live loader, which R2 keeps counting. 44→37 on 2026-09-02, the race-war cluster: ShrineInfoManager, CastleInfoManager, SweeperBonusManager, SweeperBonus, SweeperSet, LevelWarManager, MasterLairInfoManager. 37→30 on 2026-09-02, the item cluster: ItemUtil, UniqueItemManager, TimeLimitItemManager, EventItemUtil, Item, GlobalItemPositionLoader, OptionInfo. 30→23 on 2026-09-02, the content-info cluster: MonsterInfo, SkillInfo, NPCManager, ScriptManager, Directive, VariableManager, EffectShutDown. 23→19 on 2026-09-02, the play-record cluster: GQuestManager, GQuestStatus, EventHeadCount, PacketUtil. 19→14 on 2026-09-02, the session cluster: GamePlayer, IncomingPlayerManager, ZoneGroupThread, EventMorph, ConnectionInfoManager. 14→13 on 2026-09-02: SomethingGrowingUp.h, the ExpTable template — a header, so R3 is unchanged. 13→10 on 2026-09-02, the guild trio: Guild, GuildManager, GuildUnion) |
| R3 | Files with inline SQL outside `database/` and `gameserver/repository/` | 193 | `grep -rlE 'executeQuery' src --include='*.cpp' \| grep -v 'server/database' \| grep -v 'server/gameserver/repository/' \| wc -l` (repository/ joined the exclusion 2026-09-01, baseline 317→314 — two files cleansed, one pilot impl no longer counted. This reverses the pilot's "R3 still counts the impl files" note: that held only while an extraction cleansed at least as many files as it created; the PlayerCreature round — 4 tables from 2 files — would have RAISED a shrink-only ratchet for sanctioned quarantining. 314→308 on 2026-09-01: the three race files and the three skill-slot files; 308→295 the same day: the thirteen files of the effect/flag/address-book/quest-item round; 295→285 the same day: the ten files of the Zone milestone; 285→271 the same day: the fourteen balance/info loaders; 271→254 the same day: the seventeen config loaders; 254→247 on 2026-09-02: the seven race-war files; 247→240 on 2026-09-02: the seven item files; 240→233 on 2026-09-02: the seven content-info files; 233→229 on 2026-09-02: the four play-record files; 229→224 on 2026-09-02: the five session files; 224→221 on 2026-09-02: the guild trio; 221→220 on 2026-09-02: item/ItemIDRegistry.cpp; 220→211 on 2026-09-02: the nine gear item classes; 211→203 on 2026-09-02: the eight vampire/ousters gear classes; 203→197 on 2026-09-02: the six gear classes with their own Info shapes; 197→193 on 2026-09-02: the four silver weapons) |
| R4 | Packet headers with `execute()` still on the packet | 0 | `grep -rlE 'void execute\(Player' src/Core --include='*.h' \| wc -l` |
| R5 | `__BEGIN_TRY` control-flow macro sites in de-core candidates | 5,899 | `grep -rE '__BEGIN_TRY' src/server/gameserver --include='*.cpp' \| grep -vE 'gameserver/(handler\|packetfill)/' \| wc -l` (handler/ and packetfill/ hold 2.4-moved sources from `src/Core`, never counted while they lived there; fold in with a re-baseline when they become 3.x extraction targets. 5,984→5,980 on 2026-09-02: the four macros inside the guild trio's deleted dead __SHARED_SERVER__ blocks. 5,980→5,899 on 2026-09-02, textual: ItemIDRegistry.cpp's 81 hand-expanded initItemIDRegistry bodies collapsed onto one macro, so the grep sees one #define line instead of 82 matched lines — 81 expansions plus the old macro's own; each method still has its try block) |
| R6 | Line count of god files (each tracked separately) | see table below | `wc -l <file>` |
| R7 | Files declaring dynamic exception specifications (`throw(...)`) — added 2026-08-30, see 5.4 | 867 | `grep -rlE 'throw\s*\([^)]*\)\s*(const\s*)?(;|\{|=)' src --include='*.h' --include='*.cpp' \| wc -l` (867 since the never-compiled `SlotInfo.cpp`'s stale specs left with the file, 2.4 review) |

God-file baselines (R6):

All rows re-measured 2026-08-31 post-clang-format-18 (the 08-29 numbers
predated that pass); only the rows `ratchets.sh` names are enforced so far.

| File | Baseline lines |
|------|---------------:|
| `src/server/gameserver/Zone.cpp` | 9,297 |
| `src/server/gameserver/skill/SkillUtil.cpp` | 6,745 (enforced by `ratchets.sh` R6a) |
| `src/server/gameserver/InitAllStat.cpp` | 4,803 (was 4,949 before the 3.3 bonus-formula extraction; enforced by `ratchets.sh` R6b) |
| `src/server/gameserver/handler/CGSayHandler.cpp` (moved from `src/Core` in 2.4) | 4,905 |
| `src/server/gameserver/Slayer.cpp` | 4,375 |
| `src/server/gameserver/skill/SkillFormula.cpp` | 820 (was 3,081 before the 3.3 computeOutput extraction — now thin adapters + the 11 dice-roll formulas; enforced by `ratchets.sh` R6d) |
| `src/server/gameserver/skill/HitRoll.cpp` | 774 (not a god file — an extraction-target pin, locked in with its 3.3 extraction; enforced by `ratchets.sh` R6c) |

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
  > golden and header-width assertions. **2026-08-30: every encrypter
  > path is pinned** — `tests/packet_encrypter_test.cpp` covers all 19
  > `readEncrypt`/`writeEncrypt` users (17 shuffle packets, with the
  > abstract `GCAddItemToZone` pinned through its three concrete
  > subclasses, plus the unshuffled `CGAddMouseToZone`/`CGDropMoney`) at
  > encrypt codes 0..5, which reach every `code % N` case of every
  > `SHUFFLE_STATEMENT_N`; the shuffle tables themselves are asserted as
  > literals, and `ratchets.sh` fails on any new encrypter user without
  > goldens (`tests/ratchet/encrypter_exceptions.txt` for the base
  > class). Remaining non-encrypter GC/CG coverage outstanding.
  > **Adversarial review (2026-08-29) named the specific gaps, in
  > priority order:**
  > 1. ~~Only 2 of the 17 encrypter-using packets are pinned~~ — closed
  >    2026-08-30 as above. Pinning them surfaced three read/write/size
  >    disagreements, each stated as a fact by a test that flips when it
  >    is fixed (record the fix in `docs/FIXES.md`, task 5.3):
  >    - `GCDropItemToZone::read()` consumes a leading `BYTE flag` that
  >      `write()` no longer emits (commented out) and `getPacketSize()`
  >      does not count. The server only writes this packet, so write()
  >      is the pinned contract; no round-trip until read() is fixed or
  >      removed. Check the client copy in 1.4.
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
  >    zero entries). `src/server/updateserver/` and `theoneserver/`
  >    still reference the deleted headers but are built by no target;
  >    deleting those dead server trees is an open decision.
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

- [ ] **3.1 `Outcome<Events, Rejection>` result type.** C++11 template in
  `de-kernel` mirroring sidecar's `kernel.domain.Outcome`: gameplay mutations
  return `Ok(events)` or `Rejected(reason)`; exceptions reserved for
  programming/config errors. New/refactored domain code uses it; the
  `__BEGIN_TRY/__END_CATCH` macros stop being control flow (ratchet R5).
  > **Status:** in progress (2026-08-31) — `src/Core/Outcome.h` exists in the
  > kernel with unit tests (tests/outcome_test.cpp: factories, accessors,
  > throw-on-wrong-side, value semantics); adoption by domain code pending.
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
  > **Status:** pilot landed (2026-08-31) — `NicknameRepository`
  > (`gameserver/repository/`): interface + MySQL impl carrying the
  > NicknameBook table's quirks, fake + `repository_tests` in ctest;
  > `NicknameBook.cpp` and `CGModifyNicknameHandler.cpp` no longer touch
  > SQL (R2 104→103, R3 318→317). CGSayHandler's two NicknameBook
  > queries wait for the god-file work. Pattern to repeat table-by-table.
  > **PlayerCreature round (2026-09-01)**: four more repositories —
  > `RankBonusRepository` (keyless RankBonusData: duplicate rows are
  > storable and surfaced; `loadTypes` comes back Type-ascending via the
  > covering index, not insertion order; the char-deletion sweeps in
  > CreatureUtil.cpp and the loginserver still DELETE inline as part of
  > their multi-table purge), `StashRepository` (no stash table —
  > StashNum/StashGold are columns on the three race tables; the Slayer
  > row is written UNCONDITIONALLY, then Vampire xor Ousters; gold is
  > streamed through `(int)`, and an over-2^31 balance would be clamped
  > to 0 by the UNSIGNED column, not stored negative; the
  > `checkStashGoldIntegrity` reads in the three race classes route
  > through `loadStashGold` on the same seam), read-only
  > `BloodBibleSignRepository` (nothing in the server writes that table —
  > no write method on purpose; the char-deletion purges skip it too, so
  > a name-reuser inherits the old signs), and `GoodsRepository`
  > (`getDistConnection("PLAYER_DB")` IGNORES its name argument — it is
  > the thread's second connection, same server and DARKEDEN schema in
  > the shipped stack; `takeOne`'s single UPDATE leans on MySQL's
  > left-to-right SET evaluation — the `IF()` sees the already-
  > decremented Num — and taking a Num=0 row raises ER_DATA_OUT_OF_RANGE,
  > leaving the purchase stuck: a pre-existing bug, documented not fixed).
  > `PlayerCreature.cpp` and `GoodsInventory.cpp` no longer touch SQL
  > (R2 103→101; R3 re-defined to exclude `gameserver/repository/` and
  > re-baselined 317→314, see the ratchet table). Wiring is via the
  > `default*Repository()` accessors at the call sites — PlayerCreature
  > is not unit-constructible anyway. The repo methods add the
  > `SAFE_DELETE(pStmt)` three of the original success paths lacked (a
  > per-call Statement leak, fixed knowingly), and DBError.log
  > attribution moved from the game-flow function to the repository
  > method (__PRETTY_FUNCTION__ in END_DB).
  > **The adversarial review round (2x xhigh, 2026-09-01, both NO-SHIP as
  > written)** proved the extraction byte-faithful but falsified three
  > fake-pinned quirk claims by replaying the SQL on a real MySQL 5.7
  > (the Num=0 "unsigned clamp" — actually error 1690; "insertion order"
  > — actually index order; "stored negative" gold — actually clamped to
  > 0) and the "web-shop database" description of the dist connection.
  > The cure is the **MySQL-backed integration tier this task always
  > promised**: `mysql_repository_tests` (tests/integration/) runs the
  > REAL impls against a throwaway MySQL 5.7 loaded with the initdb/
  > schema and production sql_mode — fixture lifecycle handrolled in
  > `tests/integration/mysql_test.sh` (`make integration-test`; not in
  > default ctest, it needs docker). Character rows come from
  > `PlayerFixtures.h`: low/mid/high-level profiles per race, persisted
  > the way CLCreatePCHandler does it (Slayer row for every race), with
  > each test deciding what to persist. The fakes were corrected to
  > match the observed semantics and now mirror, not define, the
  > contract (fake-tier +18, integration tier 14 tests incl. the pilot's
  > NicknameBook nID-ascending order, which its fake had also
  > mis-documented).
  > **Gold + character-row round (2026-09-01)**: `GoldRepository` — the
  > carried-gold column on the three race tables, from
  > `increaseGoldEx`/`decreaseGoldEx`/`checkGoldIntegrity` in
  > Slayer/Vampire/Ousters.cpp. The writes are RELATIVE
  > (`Gold = Gold ± %u`, database-side arithmetic) and target ONLY the
  > character's own table (no Slayer fan-out, unlike stash); the gameplay
  > clamps (MAX_MONEY up, zero down) stay in the creature against its
  > in-memory balance, so a decrease can still hit
  > ER_DATA_OUT_OF_RANGE (1690) when the ROW holds less than memory says
  > (integrity drift) — pinned by the integration tier, with the row
  > verified untouched. **`CharacterRepository`** takes the rest of the
  > race files' non-load writes: the `save()` vitals/position UPDATEs
  > (per-race records; the emitted SQL keeps each race's original
  > spacing byte-for-byte — Slayer's printf `CurrentHP=%d`,
  > Vampire/Ousters' `CurrentHP = 12`; Vampire has no MP columns and
  > carries SilverDamage instead), the `saveExps()` tails (Vampire's
  > conditional `,SilverDamage = %d` fragment preserved — a zero value
  > leaves the column untouched, while Ousters always writes it), and
  > `tinysave` (caller-composed raw SET fragments — quarantined, not
  > yet retired; the WHERE's NAME-vs-Name casing is cosmetic, kept for
  > byte-fidelity, and the `Rank` backticks are load-bearing on MySQL 8
  > where RANK is reserved). The adversarial round (2x xhigh,
  > 2026-09-01, both NO-SHIP as written; the extraction itself was
  > machine-verified byte-identical, 17/17 literals, every record field
  > type traced) corrected two things the first draft got wrong: the
  > reward-reset flow is DEAD in BOTH races (Slayer's call site sits
  > inside a ~100-line `/* */` block, which the draft edited without
  > noticing — that edit was reverted and no reward method ships), and
  > the DWORD-vs-%lu varargs mismatch in the exp saves is a LATENT BUG
  > masked by GCC codegen, not an ABI guarantee (clang -O0 demonstrably
  > reads garbage for stack-passed %lu/%ld fields; the extraction
  > preserves it bit-for-bit, the %u fix is deliberate follow-up).
  > Repository SQL now uses the parameterized `executeQuery` form
  > exclusively — never string concatenation (maintainer rule; the
  > vitals and stash StringStreams were converted with byte-identical
  > output; true server-side prepared statements don't exist in this DB
  > layer and remain the upgrade that would also retire the raw-name
  > interpolation). CharacterRepository is write-only and has NO fake
  > tier — its 10 integration tests assert EVERY written column with
  > distinct sentinels and exercise every tinysave dispatch branch, by
  > the maintainer's integration-over-fakes call; known blind spots,
  > documented in the suite: the `Rank` backticks are untestable on the
  > 5.7 tier, a seam signature break surfaces in the gameserver build
  > rather than default ctest, and DBError.log now attributes failures
  > to repository methods instead of game-flow functions. The shared
  > race enum moved to `repository/CharacterRace.h` (`CharacterRace`,
  > née StashRace). Ratchets unchanged: the race files keep exactly
  > their `load()` SELECT + skill-save load (the 53+ positional-column
  > transplant is the NEXT round); the dead comment blocks (Slayer's
  > `getIP` UserIPInfo, Slayer's AND Vampire's reward flows) are
  > comments, not debt. Gold's seam does not enclose every Gold writer:
  > `setGoldEx` writes the column absolutely via tinysave,
  > SGAddGuildMemberOKHandler inline, and the sharedserver's
  > GSQuitGuildHandler from another process — named in
  > GoldRepository.h, to be extracted with their own flows.
  > **Character-load round (2026-09-01)**: the race files' login-time
  > `load()` SELECTs moved into `CharacterRepository` as
  > `loadSlayer/loadVampire/loadOusters` returning per-race load
  > records (54/33/34 columns, in SELECT order). The transplant was
  > mechanical — a script mapped every positional `pResult->getX(++i)`
  > read, in order, to a record field and left comment lines alone, so
  > the diff is checkable rather than transcribed; every record field is
  > typed to the driver getter the inline code used (int/BYTE/string), so
  > each narrowing the race class performed at its setter still happens
  > there on the same value. Quirks quarantined in the impl: the column
  > lists are the positional contract and are verbatim (the only byte
  > change is the whitespace the backslash-continued literals had
  > leaked into the SQL — a space plus indentation tabs — each run now
  > a single space); `Active = 'ACTIVE'` filters
  > an INACTIVE (deleted-in-handover) row into "no row"; the vampire/
  > ousters Competence pair is read through getBYTE where the slayer
  > uses getInt (tinyint columns, so nothing is lost); the SLAYER loader
  > overrides the Sight it just applied (to 13 — vampire/ousters apply
  > theirs as loaded), and the Slayer/Vampire Reward column is selected
  > and surfaced but consumed by nothing (the reward flow is dead in
  > both races that have the column; Ousters never selected it). The
  > setters now run OUTSIDE the `BEGIN_DB`/`END_DB` try — inert, since
  > none of them can raise the SQLQueryException that catch handles —
  > and the Statement is freed BEFORE the setters run, so a setter
  > throwing (setSex's InvalidProtocolException, the skill loop's
  > Assert) no longer leaks it; the one real delta is that a mid-read
  > driver exception now applies nothing instead of the columns read so
  > far (unreachable with a fixed column count). **`SkillSaveRepository`**
  > is new and encloses the three learned-skill tables on the compiled
  > gameserver side: the loads from the race files AND the
  > insert/update/delete from `SkillSlot`/`VampireSkillSlot`/
  > `OustersSkillSlot` (the per-character purges in CreatureUtil.cpp and
  > the loginserver's CLDeletePCHandler stay inline with their
  > multi-table deletion flow; the unbuilt `Vampire_backup.cpp` still
  > carries the old VampireSkillSave SELECT and is one of R2's counted
  > files). Two record
  > families on purpose — driver-typed `*Row` for loads, member-typed
  > `*Record` for writes — so the varargs bytes reaching the format
  > strings are unchanged (incl. the stack-passed time_t NextTime
  > through `%d` — the low 32 bits, whole until 2038 — and the DWORD
  > exp/delay through `%d`; the WORD type/level members promote to int
  > exactly; preserved not fixed). The
  > tables are keyless: duplicates store, an UPDATE/DELETE hits every
  > row of the type. Row order of the ORDER-BY-less loads was checked
  > against the real server BEFORE being written down — and the server
  > falsified the first draft's "SkillType ascending via the secondary
  > index" claim: on the tier's near-empty table the uncovered SELECT
  > comes back in INSERTION order (a scan in hidden-row-id order), and a
  > production-sized table may take the index and reorder. Pinned as
  > observed and documented as the optimizer's choice, not a contract;
  > the vampire/ousters loaders keep the first row of a duplicated type,
  > so which duplicate wins is plan-dependent — the order test asserts
  > the multiset as the contract and records the 5.7 tier's insertion
  > order separately as an observation. No fake tier for either seam,
  > by the standing integration-over-fakes call; +13 integration tests
  > (every load column asserted with a sentinel unique within the test
  > — its SELECT position for the numeric columns, the negated position
  > for the signed ones, distinct enum/varchar values for the three text
  > columns — so a transposed pair fails; every skill-save write
  > asserted for both the columns it writes and the ones it must leave
  > alone). Slayer.cpp, Vampire.cpp and Ousters.cpp no longer execute
  > any SQL (R2 101→98; Vampire.cpp's saveExps still carries a
  > pre-existing commented-out StringStream UPDATE with no
  > `executeQuery` token), and neither do the three slot files (R3
  > 314→308). Because both ratchets grep textually, the dead
  > commented-out blocks that still spelled `executeQuery` were deleted
  > rather than edited: both races' `if (reward != 0)` reward flows
  > (already proven dead in the previous round), Slayer::getIP's
  > UserIPInfo lookup, and the `/* StringStream ... executeQueryString
  > */` blocks inside the slot files' replaced DB blocks (load-bearing
  > for R3) — git history keeps them. The `#include "DB.h"` left each
  > cleansed file, which decouples only the slot files: the race files
  > still reach DB.h transitively through ConcreteItem.h. Korean
  > comments on the re-indented lines were translated.
  > **Effect / flag / address-book / quest-item round (2026-09-01,
  > stacked on the character-load round)**: four more seams, thirteen
  > files cleansed (R2 98→85, R3 308→295). **`EffectSaveRepository`**
  > encloses the eight persisted-effect tables — the create()/destroy()/
  > save() overrides and the *Loader::load() of EffectAftermath,
  > EffectKillAftermath, EffectMute, EffectCanEnterGDRLair (DEADLINE
  > tables: an absolute expiry plus a never-read YearTime), the three
  > force scrolls (REMAIN tables: the turns left at save time, so
  > logged-out time does not count) and EffectEnemyErase (a deadline
  > table with an EnemyName). The eight classes never agreed on SQL
  > spacing, so the format strings are per-table data, verbatim
  > (EffectMute's "YearTime=%ld", the force scrolls' "SELECt" typo and
  > "(OwnerID, RemainTime )", CanEnterGDRLair's "VALUES ("); the Turn_t
  > year time / remain turn still ride %ld/%lu (register-passed, the
  > documented latent-bug family, preserved). Structural quirks pinned
  > against the real server: seven tables are keyless and accumulate a
  > duplicate row on a second create(), EffectKillAftermath alone has
  > OwnerID as PRIMARY KEY and raises ER_DUP_ENTRY; EnemyErase's DELETE
  > keys on (OwnerID, EnemyName) but its UPDATE on OwnerID alone, so
  > saving one enemy-erase effect rewrites EVERY EnemyErase row of the
  > owner. The transplant was again scripted (the eight files share one
  > shape; the script anchors on it and dies on any deviation), the
  > loaders keep every gate and per-race branch and now iterate a
  > vector instead of a live result. **`FlagSetRepository`** (the
  > character's 24 flag bits as a '0'/'1' text; the StringStream-built
  > INSERT/UPDATE/DELETE became parameterized format strings with the
  > same bytes, the INSERT's two trailing spaces included; OwnerID is
  > the PK — insert() refuses a second row, the load-path fallback is
  > the INSERT IGNORE of an empty text, both pinned),
  > **`SMSAddressRepository`** (the phone book; (eID, OwnerID) PK
  > pinned; a datetime Time column nothing on the server touches) and
  > **`QuestItemRepository`** (GQuestItemObject, one row per item
  > instance — removeOne's LIMIT 1 is load-bearing and pinned; encloses
  > GQuestInventory AND the two GQuestGive*Element inserts; the
  > original load/removeOne leaked their Statement on success, fixed
  > knowingly). No fake tier for any of the four (integration over
  > fakes); +11 integration tests. The per-character purges
  > (CreatureUtil.cpp, CLDeletePCHandler) still DELETE inline from
  > FIVE of the eleven tables — EffectAftermath, EffectMute, EnemyErase,
  > FlagSet, GQuestItemObject; nothing purges EffectKillAftermath,
  > CanEnterGDRLair, the three force scrolls or SMSAddressBook, whose
  > rows outlive the character and are inherited by a name-reuser (the
  > adversarial round caught the first draft claiming "all of these
  > tables", and a fabricated "negative remain turn clamps to the
  > column maximum" quirk: Timeval's timediff returns the absolute
  > difference and a DWORD cannot exceed the column, so the real quirk
  > is that a scroll saved past its deadline would store the elapsed
  > time as remaining — unreached, now documented as such).
  > **Zone milestone (2026-09-01, stacked on the effect round)**: every
  > SQL statement the ten zone-layer files ran — against the zone
  > config tables, Messages, Event200501, BulletinBoardObject and
  > RegenZonePosition — is behind a seam; ten files cleansed (R2 85→75,
  > R3 295→285). Zone-named neighbours on OTHER tables (ZoneGroupThread's
  > PCRoomDBInfo read, the PKZone/EventZone/LevelWarZone info loaders)
  > are not in this round. **`ZoneInfoRepository`** is the
  > read-only zone CONFIGURATION seam: the group list (with and without
  > ORDER BY — two statements, kept as two, the caller choosing;
  > ZoneGroupManager::load, makeDefaultLoadInfo, ThreadManager::init),
  > the zones of a group (same pair), the 17-column ZoneInfo row
  > (ZoneInfoManager; the SELECT's SMPFilename/SSIFilename spelling
  > differs from the schema's SmpFileName — case-insensitive, kept), the
  > per-zone resurrect positions (ResurrectLocationManager, its "table
  > does not exist" throw on an empty result kept), the ZoneTriggers
  > rectangles (Zone::loadTriggeredPortal), the EffectPKZoneRegen
  > rectangles and the per-zone/per-race WayPointInfo points
  > (Zone::loadEffect) and every way point (WayPointManager).
  > **`MessageRepository`** — the keyless Messages queue the zone drains
  > on entry (load, delete; the pay-zone eviction INSERT in the
  > `__PAY_SYSTEM_ZONE__` branch of ZonePlayerManager, which no build
  > config compiles — converted by hand, not compile-checked).
  > **`ComebackEventRepository`** — the three Event200501 zero-date
  > predicates on the dist connection (Zone.cpp ran them on one
  > never-freed statement; three calls now, each freeing its own).
  > **`BulletinBoardRepository`** — the notice corpses: the column-less
  > `VALUES (0, ...)` INSERT that leans on the table's column order and
  > the auto-increment, returning the affected count the caller logs;
  > the per-zone load; the expired-row DELETE that used a second, never-
  > freed statement (fixed knowingly). **`RegenZoneRepository`** — the
  > race-war towers (load and reload issued the identical SELECT and
  > neither freed its statement; fixed knowingly). Two dead blocks that
  > still spelled `executeQuery` were deleted rather than edited:
  > ZoneGroupManager's `/* */` copy of load() and ZoneInfo::load's
  > commented-out body (the live function is `Assert(false)`). No fake
  > tier; +9 integration tests, seeded config rows kept clear of the
  > shipped data by using ids from 31000 up. Not enclosed: the
  > loginserver/sharedserver's own ZoneGroupInfo/ZoneInfo reads, the
  > MAX(ZoneGroupID) probes (ConnectionInfoManager, EffectShutDown,
  > CGSayHandler), the Messages users in the guild handlers
  > (CGQuitUnion*, CGAcceptUnion, CGDenyUnion, CGExpelGuild INSERT; the
  > SG*GuildOK handlers SELECT+DELETE — they drain, never insert) and
  > the sharedserver's GS*Guild* inserts, CGGetEventItemHandler/
  > CLLoginHandler's event stamps, and two dead files R3 still counts
  > (`src/server/ZoneUtil.cpp`, an unbuilt byte-identical sibling of the
  > old bulletin-board code, never-freed statement included, and
  > `src/server/theoneserver/ThreadManager.cpp`, plus the sharedserver's
  > two-race variant of the resurrect SELECT in its own
  > ResurrectLocationManager.cpp) — each its own extraction. Two fixes
  > the round made without saying so, named by the fidelity review:
  > RegenZoneManager declared its `Statement* pStmt` uninitialised while
  > END_DB deletes it (a throwing createStatement would have deleted an
  > indeterminate pointer — the seam initialises), and
  > ResurrectLocationManager's "table does not exist" throw used to
  > escape BEGIN_DB with the statement still allocated. The
  > bulletin-board insert returns the affected count as the driver's
  > uint. The adversarial round also caught the R2/R3 baseline
  > COLUMNS of the ratchet table lagging their prose since the effect
  > round (the prose said 295/285 while the column still read 308) —
  > fixed here; the number in the column is the contract.
  > **Balance / game-info loaders (2026-09-01, stacked on the Zone
  > round)**: fourteen read-only loader files cleansed (R2 75→61,
  > R3 285→271), two seams. Ten of their sixteen managers run at boot;
  > SIX are dead — ObjectManager has the STR/DEX/INT balance,
  > SkillParent, RankEXP and FameLimit managers' construction and load
  > commented out, and their surviving getter call sites sit inside
  > comment blocks (Slayer.cpp, skill/SkillUtil.cpp) — so those
  > extractions are for the ratchet, not for a running server.
  > **`BalanceInfoRepository`** — the level/exp
  > ladders: STR/DEX/INT, VampEXP, OustersEXP (per-table statement
  > pairs kept as data: mixed-case "Select ... from", the trailing
  > space after the three attribute table names, SkillPointBonus only
  > for ousters), RankEXPInfo per RankType, SkillDomainInfo per
  > DomainType, FameLimitInfo per DomainType, and the pet ladders
  > (PetExpInfo, PetAttrBalanceInfo, PetAttrInfo).
  > **`GameInfoRepository`** — SkillTreeInfo (keyless; the loader relies
  > on same-type rows arriving adjacent — the optimizer's choice, not a
  > contract), RankBonusInfo (`Rank` backticks, MySQL-8-load-bearing),
  > PetTypeInfo, GameServerGroupInfo (the +2 array sizing stays at the
  > caller), BloodBibleBonusInfo, and the four monster-name lists
  > (FirstNameInfo/MiddleNameInfo/LastNameInfo with the 'BASIC'/'EVENT'
  > filter inline in the literal — four statements, kept as four).
  > **One knowing behavior change, disclosed and pinned:** the eleven
  > level-indexed loaders (all but PetExpInfo, PetAttrInfo and
  > MonsterNameManager, which read their tables whole) size an array
  > from a `SELECT MAX(...)` probe and guarded the "empty table" case
  > with `getRowCount()==0` or `!next()` ON THAT PROBE — guards that
  > can never fire, because MySQL answers MAX() over nothing with ONE
  > row holding NULL; the inline code then called `getInt(1)`, i.e.
  > `atoi(NULL)`, and an empty table crashed the boot instead of raising
  > the intended "There is no data" Error. The seams' `loadMax*` return
  > false on the NULL, so the callers' throws fire as written. Pinned
  > by the tier through a RankType/DomainType no ladder has.
  > MonsterNameManager's four `getRowCount()==0` guards are on real row
  > queries, DO fire on an empty list, and are kept as `size()==0`. (The
  > first draft said "every loader" — the claims audit counted.) Other
  > disclosures: GameServerGroupInfoManager's hand-written try turned a
  > SQLQueryException into an Error and swallowed other Throwables with
  > a cout — the SQL failure now takes every repository's path
  > (DBError.log + thrown const char*), the Throwable arm is kept;
  > FameLimitInfoManager is never constructed and its table is absent
  > from the shipped schema (dead, extracted for the ratchet, the tier
  > pins that its first query throws); EventMonsterNameManager.cpp is
  > an unbuilt OLDER VARIANT of MonsterNameManager (a second definition
  > of `MonsterNameManager::init` with three unfiltered `SELECT *`
  > lists) and stays as is (still one of R2's counted files). **The
  > fidelity review caught what the first draft missed: THREE of the
  > fourteen files — AttrBalanceInfo.cpp, RankEXPInfo.cpp and
  > FameLimitInfo.cpp — are in no CMakeLists (they still `#include
  > <algo.h>`, an SGI header that no longer exists), so those
  > conversions, including the three STR/DEX/INT blocks, are text-only
  > and no compiler has seen them; the other eleven are built and
  > linked.** Also disclosed on review: PetTypeInfo's original declared
  > its Statement uninitialised and never freed it on success (one leak
  > per boot; END_DB would have deleted an indeterminate pointer on a
  > SQL failure), PetAttrInfo's was uninitialised too, and
  > MonsterNameManager/GameServerGroupInfoManager leaked theirs on
  > their throw paths — the seams initialise and free; two Korean
  > message strings were translated (PetTypeInfo's throw text,
  > PetAttrInfo's cout), an observable-output delta CLAUDE.md sanctions;
  > the two per-domain loaders (SkillDomainInfoManager, FameLimitInfo)
  > now create a Statement per query instead of one for the loop —
  > same queries, same order, boot-time only; and the attribute
  > ladders' bigint AccumExp EXCEEDS int range in the shipped data
  > (2431521747, 3344798380) — read through atoi as before, lossless
  > through the int→DWORD round trip below 2^32, the first draft's
  > "stays within range" retracted. No fake tier; +7 integration tests
  > asserting the SHAPE of the shipped data (a maximum exists, rows
  > stay within it, no list empty — what the boot requires). One field
  > the seam reads that the inline code never fetched: PetAttrBalanceInfo's
  > AddAttr (column 3) rides along in the row; the caller still uses
  > AccumAttr (column 4) exactly as before. Not enclosed: the
  > loginserver's CLCreatePCHandler single-row reads of the ladders,
  > the loginserver/sharedserver GameServerGroupInfo loads, and the
  > loginserver's UserInfoManager — a byte-level clone of the
  > GameServerGroupInfo MAX-probe-plus-rows pattern, same never-firing
  > guard, same +2.
  > **Config loaders (2026-09-01, stacked on the balance round)**:
  > seventeen more read-only boot-time loaders cleansed (R2 61→44, R3
  > 271→254) by extending the two existing seams rather than adding a
  > third. `ZoneInfoRepository` gains the zone-shaped config reads:
  > ZoneEffectInfo rectangles by zone and effect (EffectOnBridge; of
  > the nine skill/Effect*.cpp that name the same table, six live
  > loaders — AcidSwamp, ContinualBloodyWall, GreenPoison, IceField,
  > Prominence, YellowPoison — use this exact 7-column literal and are
  > the method's next callers, EffectDarkness reads a 4-column `%u`
  > variant that needs its own method, and EffectBloodyWall /
  > EffectGrayDarkness only mention it inside commented-out loaders —
  > two of R2's textually counted files), ZoneInfo's
  > MonsterList/EventMonsterList texts for a zone (MonsterManager,
  > dynamic zones reading their template's), and the PKZoneInfo /
  > EventZoneInfo / LevelWarZoneInfo tables. `GameInfoRepository` gains
  > WeatherInfo, GSStringPool, ShopTemplate, NicknameIndex's 'LEVEL'
  > rows, ItemMineInfo, ItemGradeRatioInfo, GoodsListInfo (on the dist
  > connection, "Limited+0" enum ordinal, "Kind<>'SET'" filter — as
  > GoodsInfoManager read it), DefaultOptionSetInfo, DarkLightInfo (its
  > " , "-spaced column list kept), CastleSkillInfo (mixed-case
  > "Select ... from"), CastleShrineInfo and LogUserInfo. Every row
  > field is the driver getter's type; the callers keep their narrowings
  > and their Asserts on the shipped shape (WeatherInfo's 12 rows,
  > ItemGradeRatioInfo's 10, DarkLightInfo's month/hour/minute ranges).
  > Not converted: `gameserver/GameWorldInfoManager.cpp` is an unbuilt
  > stale fork — no CMake target lists it, there is no sibling header,
  > and the live WorldInfo loader is `src/server/GameWorldInfoManager.cpp`
  > in ServerCore (reached from ObjectManager), which cannot include a
  > gameserver repository header and keeps its `executeQueryString`
  > read and hand-written SQLQueryException→Error / Throwable→cout try.
  > The review caught a first cut that had converted the fork, added a
  > caller-less `loadWorlds()` and claimed eighteen; both are reverted
  > and R2 keeps counting the fork. Disclosures: seven of the originals
  > never freed their Statement on the success path (StringPool,
  > LevelWarZoneInfoManager, ItemMineInfo, EventZoneInfo,
  > DefaultOptionSetInfo, CastleShrineInfoManager, EffectOnBridge — that
  > one once per zone at boot) and three more (WeatherInfo,
  > ItemGradeManager, DarkLightInfo) Asserted on the row shape before
  > freeing it, so a failed Assert leaked — the seams free before
  > returning, fixed knowingly; LevelNickInfoManager declared its
  > `Statement* pStmt` uninitialised while END_DB deletes it (the
  > RegenZoneManager quirk again) — the seam initialises it;
  > SystemAvailabilitiesManager's SELECT sits under
  > `__CHINA_SERVER__`/`__THAILAND_SERVER__` (uncompiled) and stays. No
  > fake tier; +5 integration tests (fixed row counts, non-empty lists,
  > the dist-connection goods read against inserted 'SET' and 'FOREVER'
  > rows — the shipped seed has neither, so without them the filter and
  > the ordinal's upper bound would pass unexercised — and the two
  > per-zone reads against inserted rows). Remaining SQL under
  > gameserver/: the long tail (R2 = 44 files) — biggest remaining
  > clusters are CreatureUtil.cpp's 128-statement character deletion
  > and the guild trio (Guild/GuildManager/GuildUnion, 65 statements).
  > **Race-war cluster (2026-09-02, stacked on the config round)**: the
  > seven war files (ShrineInfoManager, CastleInfoManager,
  > SweeperBonusManager, SweeperBonus, SweeperSet, LevelWarManager,
  > MasterLairInfoManager) move to a new `WarInfoRepository` — the first
  > seam in this stack that mixes boot-time reads with runtime writes:
  > ShrineInfo (load, owner reload, owner save), CastleInfo (load by
  > ServerID, the six-column state save, and `tinysave`'s caller-composed
  > SET fragment — the same raw-SQL quarantine as
  > CharacterRepository::tinysave), SweeperBonusInfo (MAX probe as a
  > bool, rows, owners by level, owner save), SweeperSetInfo and
  > SweeperOwnerInfo (both per zone; LevelWarManager's two-column read
  > of the owners has its own method; the owner UPDATE keys on
  > SweeperType alone, the table's PK), LevelWarHistory (no primary or
  > unique key; INSERT
  > of the "Old" sweeper columns at war start, UPDATE of the "new" ones
  > at war end keyed on Level + a caller-formatted start time) and
  > MasterLairInfo's 25-column row. R2 44→37, R3 254→247. Every literal
  > byte-for-byte, including the SweeperOwnerInfo UPDATE's `%ld` for an
  > int OwnerRace and `%d` for a uint SweeperType, and the castle save's
  > unspaced `GuildID=%d` list against the sweeper writes' spaced ones.
  > Disclosures: LevelWarManager's two recorders and SweeperSet's load
  > (both of its Statements) never freed on the success path — the seam
  > does, fixed knowingly; ShrineInfoManager::saveBloodBibleOwner opened
  > a BEGIN_DB block and created a Statement it never used around a loop
  > whose body already went through the seam — the block is gone, the
  > loop stays; SweeperSet's commented-out first-generation loader
  > (owner-per-set, "보관대" names) is deleted rather than carried;
  > SweeperBonusManager's two "There is no data" throws now fire on the
  > MAX probe's false where the originals' `getRowCount() == 0` guard
  > never could (one NULL row — the original atoi(NULL)'d and crashed on
  > an empty table, the seam fails the boot with the intended Error);
  > the row structs read every column of every row where four inline
  > loops read the tail columns only inside a branch (MasterLairInfo's
  > 24 non-key columns behind `getMasterLairInfo() != NULL` in reload
  > and behind `isAvailable()` in the Thailand/China build of load,
  > SweeperBonusInfo's OwnerRace behind the type lookup,
  > SweeperOwnerInfo's SweeperSafeType behind `Assert(race < 4)`) —
  > harmless against the schema, where every int column is NOT NULL;
  > the Korean comments inside MasterLairInfoManager's load/reload (five
  > lines) and one in ShrineInfoManager::load are translated, seven
  > others in MasterLairInfoManager stay, and its thrown Error strings
  > are untouched. Not enclosed:
  > war/RaceWar.cpp's ShrineInfo owner reads and the gameserver /
  > sharedserver GuildManager castle counts and lists. No fake tier; +7
  > integration tests (seeded reads non-empty and the MAX probe against
  > the table, shrine-owner save scoped to one shrine, castles scoped to
  > the server with save and `tinysave` keyed on server+zone, sweeper
  > bonus owners by level and save by type, sweeper sets/owners scoped
  > to the zone with the PK-only owner save, level-war history insert
  > then update filling only that war's "new" columns, and a 25-column
  > master-lair row read back). Remaining SQL under gameserver/: R2 = 37
  > files.
  > **Item cluster (2026-09-02, stacked on the race-war round)**: seven
  > item files (ItemUtil, UniqueItemManager, TimeLimitItemManager,
  > EventItemUtil, Item, GlobalItemPositionLoader, OptionInfo) — R2
  > 37→30, R3 247→240. A new `ItemRepository` takes the bookkeeping
  > tables: the ItemTraceLog / MoneyTraceLog inserts (SQL-side now()),
  > the EventQuestRewardSchedule decrement (bWinPrize; true when a row
  > changed), the ResurrectItemCount / CardCount / LuckyBagCount /
  > GiftBoxCount / EventItemCount increments, UniqueItemInfo (list, the
  > per-item limit/current read as a bool, +1 / −1), TimeLimitItems
  > (load by owner and status, insert, and the status update reporting
  > whether a row changed — one literal that served two callers), and
  > the two per-class item-object operations that take the table NAME
  > as data (Item::destroy's DELETE, GlobalItemPositionLoader's position
  > read; the caller still picks the name from ItemObjectTableName[]).
  > `GameInfoRepository` gains OptionInfoManager's four loads (OptionInfo
  > 19 columns, OptionClassInfo, RareEnchantInfo,
  > PetEnchantOptionRatioInfo); OptionInfo's SELECT was assembled from
  > StringStream pieces and run through executeQueryString — the joined
  > bytes are the literal — and its three `getRowCount() == 0` "There is
  > no data" throws (plain SELECTs, so they could fire) become
  > `rows.empty()` checks at the caller, same Error, same outer
  > catch/rethrow. Every literal byte-for-byte, including
  > TimeLimitItems' lower-case from/where/and and the varargs mismatches
  > kept as they were: an ItemID_t (DWORD) through "%lu" in the delete
  > and "%d" in the position read, bWinPrize's two DWORDs through "%d".
  > Disclosures: UniqueItemManager::isPossibleCreate returned from inside
  > its BEGIN_DB block on the found path and so never freed its
  > Statement, and four loaders (UniqueItemManager::init,
  > OptionInfoManager::load, TimeLimitItemManager::load,
  > GlobalItemPositionLoader::load) did their row processing inside the
  > block that owned the Statement, so any non-SQL Throwable from that
  > processing leaked it — the seam frees before the caller processes,
  > fixed knowingly; OptionInfo's four queries shared one Statement where
  > the seam uses one per call, and its SELECT now runs through
  > executeQuery's 2048-byte format buffer instead of executeQueryString
  > (the literal is ~250 bytes); remainTraceLogNew's
  > commented-out ItemTrace2Log block (dead since it was written) is
  > deleted because R2's grep is textual; the Korean comments inside the
  > replaced blocks (UniqueItemManager, Item) went with them or are
  > translated, the rest of those files' comments stay. Not enclosed: the
  > item-object tables' own create/save/load SQL in gameserver/item/ (90
  > files, R3 territory); the two remaining raw TimeLimitItems deletes,
  > CreatureUtil.cpp's (the character-deletion flow) and the
  > loginserver's CLDeletePCHandler.cpp's; and MoonCardUtil.cpp's copy of
  > the CardCount UPDATE (in no build target). No fake tier; +7 integration tests
  > (trace logs with their enum texts and a server-side time;
  > unique-item numbers read and counted per class+type, the no-row
  > false, and the UNSIGNED decrement at 0 erroring (ER 1690) with the
  > row untouched; time-limited items by owner+status with the
  > owner+class+id status update; the four keyed counters touching only
  > their row and the keyless ResurrectItemCount touching every row (a
  > second row is inserted — the seed has one, so before+1 == after
  > alone would not tell the two apart); the event-quest reward taken
  > once per Count and only when due; the PotionObject position read and
  > delete by table name; OptionInfo read in SELECT order and the other
  > three option tables non-empty). Remaining SQL under gameserver/: R2 =
  > 30 files.
  > **Content-info cluster (2026-09-02, stacked on the item round)**:
  > seven more files (MonsterInfo, SkillInfo, NPCManager, ScriptManager,
  > Directive, VariableManager, EffectShutDown) — R2 30→23, R3 240→233.
  > A new `ContentInfoRepository` takes the content tables the server
  > loads at boot: MonsterInfo (MAX probe as a bool, the 35-column load,
  > the second-pass MonsterSummonInfo read, and reload's own 32-column
  > SELECT in both shapes — every row, or the one row the caller's
  > StringStream used to append " WHERE MType=<n>" for), SkillBalance
  > (MAX probe, the 26-column mixed-case "Select ... from" with its
  > `RequireSkill`/`Condition` backticks, Domain/MagicDomain through
  > getBYTE), NPC (by zone, or by zone and race), Script (ORDER BY
  > ScriptID — the one ordered load here), DirectiveSet (MAX probe,
  > rows) and AttrInfo (MAX probe, rows, and the attr1 UPDATE
  > VariableManager::setVariable fires on every call — the GM `opset`
  > path, and the defaults written at init()/load()).
  > `ZoneInfoRepository` gains `loadMaxZoneGroupID` for EffectShutDown's
  > two MAX(ZoneGroupID) probes (three stay inline: ConnectionInfoManager's
  > one and CGSayHandler's two). Two literals change bytes: the
  > backslash-continued MonsterInfo and SkillBalance SELECTs drop their
  > leaked indentation tabs for single spaces — the same whitespace-run
  > collapse the character-load round made, and the only byte change.
  > Kept: the reload SELECT's double space before "FROM" (a StringStream
  > joined "NormalRegen " to " FROM MonsterInfo"). Disclosures: the MAX
  > probes' dead guards — `getRowCount() == 0` in MonsterInfo,
  > SkillBalance and DirectiveSet (whose `throw(const char*)` also
  > leaked the Statement on that dead path; the seam frees first), `<= 0`
  > in VariableManager — now fire on an empty table where the originals
  > atoi(NULL)'d; VariableManager's second `<= 0` guard, on the AttrInfo
  > rows, was live and is kept as `rows.empty()`, now unreachable behind
  > the probe's false; EffectShutDown had no guard at all and now throws
  > a new Error("Critical Error : ZoneGroupInfo table is empty.") where
  > it crashed; MonsterInfoManager::reload ran a MAX(MType) probe whose
  > result it never read — dropped; the row structs read every column of
  > every row where three inline loops read the tail columns only inside
  > a branch (NPCManager behind `getCreature(Name) == NULL`, reload
  > behind `getMonsterInfo() != NULL`, SkillBalance's last six behind the
  > Ousters-domain check) — NPC's columns and SkillBalance's last six are
  > NOT NULL and NULL text reads as "", but 21 of the 32 columns reload
  > reads are nullable ints (Level, STR, DEX, INTE, BSize, Align, AOrder,
  > Moral, Delay, ADelay, Sight, AIType DEFAULT NULL; nine more nullable
  > with a default) and Result::getInt is atoi(getField()): load()
  > already reads every one of them unconditionally for every row, so a
  > NULL in MonsterInfo crashes the boot before reload could see it, and
  > the newly reachable case is a row inserted after boot with a NULL for
  > a MType not yet loaded (the shipped seed has no NULL there); four
  > dead comment blocks that mentioned executeQuery are deleted
  > (MonsterInfo::load's older StringStream SELECT and its GROUP BY
  > treasure loader, SkillInfo's SkillInfo-table probe, ScriptManager's
  > per-owner SELECT) because R2's grep is textual, along with
  > NPCManager's commented-out cout and `// StringStream sql;` and
  > MonsterInfo's commented-out setMonsterSummonInfo marker (the row
  > structs' comments carry that column-set difference); Korean comments
  > inside the replaced blocks are translated (MonsterInfo, SkillInfo,
  > NPCManager, Directive), the thrown Korean Error strings and the
  > NPC.log format stay. Not enclosed: EventMonsterNameManager.cpp and
  > GameServerInfoManager.cpp (in no build target) and the
  > MAX(ZoneGroupID) probes named above. No fake tier; +6 integration
  > tests (seeded reads non-empty with every MAX probe checked against
  > its table; a 35-column monster row read back and reload's one-row
  > and every-row shapes; a 26-column skill row incl. the BYTE domains;
  > NPCs scoped to zone and race; scripts ordered by ScriptID; a
  > directive set and a variable inserted, read and the variable saved).
  > Remaining SQL under gameserver/: R2 = 23 files — CreatureUtil.cpp
  > (128), the guild trio (65), EventShutdown (18), GamePlayer (11) and
  > a tail of ≤6.
  > **Play-record cluster (2026-09-02, stacked on the content-info
  > round)**: four files (GQuestManager, GQuestStatus, EventHeadCount,
  > PacketUtil) — R2 23→19, R3 233→229. A new `PlayRecordRepository`
  > takes a player's saved quest states (GQuestSave: the login load with
  > its SQL-side save age `unix_timestamp(now()) - unix_timestamp(Time)`,
  > the REPLACE on every status change, and the DELETE whose numeric key
  > is quoted — `QuestID='%u'`), the half-hourly HeadCount insert, and
  > the minigame score read (`LIMIT 1` with no ORDER BY — whichever row
  > the optimizer hands back, not a top score; false when none). Every
  > literal byte-for-byte; the DWORD quest id, BYTE status, BYTE levels
  > and uint count stream through "%u" as before. Disclosures:
  > EventHeadCount::activate never freed its Statement (at most once per
  > player every half hour, for the level bands it counts) — the seam
  > does, fixed knowingly; GQuestManager::load did its row processing
  > (new GQuestStatus, addEffect, filelog) inside the block that owned
  > the Statement, so any non-SQL Throwable from it leaked the Statement
  > — the seam frees before the caller loops; GQuestManager's loop read
  > the save-age column only for the three event quests (1001/2001/3001)
  > in COMPLETE state — the row reads it for every save (harmless: Time
  > is NOT NULL, so the unix_timestamp subtraction is never NULL);
  > PacketUtil's commented-out second score read (the player's own) and
  > its Korean note on getAffectedRowCount are deleted because R2's grep
  > is textual. **Deferred with a reason**: TradeManager's TradeLog INSERT
  > concatenates a trade summary of unbounded length (every traded
  > item's toString) and runs it through executeQueryString; the
  > repository convention is parameterized executeQuery, whose 2048-byte
  > vsnprintf buffer throws on longer statements — converting it would
  > turn a large trade's log into a new failure after the gold has
  > already moved. It waits for an uncapped parameterized path in the
  > DB layer. (CGBuyStoreItemHandler's TradeLog INSERT is a different
  > case: already parameterized, one item's toString — it stays inline
  > only because it is a handler-directory file.) Not enclosed: the
  > character-deletion sweeps of GQuestSave (CreatureUtil.cpp, the
  > loginserver's CLDeletePCHandler.cpp) and CGSubmitScoreHandler's
  > MiniGameScores UPDATE; CGSayHandler's and
  > mission/MiniGameQuestStatus.cpp's MiniGameScores reads are
  > commented out (the latter calls sendGCMiniGameScores, now a caller
  > of the seam). No fake tier; +3 integration tests
  > (saved quests replaced, loaded per owner with a sane save age,
  > re-replaced in place and deleted; the head-count row with its
  > server-side time; the score read's found/none paths). Remaining SQL
  > under gameserver/: R2 = 19 files.
  > **Session cluster (2026-09-02, stacked on the play-record round)**:
  > five files (GamePlayer, IncomingPlayerManager, ZoneGroupThread,
  > EventMorph, ConnectionInfoManager) — R2 19→14, R3 229→224. A new
  > `SessionRepository` takes the session-END bookkeeping: GuildMember's
  > LogOn = 0 (GamePlayer's three races and EventMorph's slayer→vampire
  > morph — the four sites in these files — shared one literal, one
  > method; a fifth, built copy in skill/Restore.cpp stays inline, see
  > below), the account row on the dist connection (LogOn='LOGOFF' +
  > LastLogoutDate=now() for a row still in 'GAME'; the boot-time sweep
  > that lists this server's 'GAME' accounts, clears their
  > PCRoomUserInfo rows and flips them to LOGOFF; the SpecialEventCount
  > read through getDWORD and its "%d" write of the uint member), the
  > PC-room lotto row (SELECT / UPDATE / positional INSERT with the
  > AUTO_INCREMENT 0 and the first Amount 1), UserIPInfo's two deletes,
  > and the NetMarble user count on the USERINFO connection (UPDATE,
  > then INSERT IGNORE when no row changed). ConnectionInfoManager's
  > MAX(ZoneGroupID) goes through
  > `ZoneInfoRepository::loadMaxZoneGroupID` (CGSayHandler's two probes
  > are the last inline ones). Every literal byte-for-byte — the sweep's
  > lower-case "from", "LogOn='GAME'" against "LogOn = 'LOGOFF'", the
  > UserStatus INSERT's "Values". The tier gains the USERINFO database
  > (initdb/USERINFO.sql mounted next to DARKEDEN.sql;
  > `DatabaseManager::setUserInfoConnection` added for processes that
  > never run init()). Disclosures: GamePlayer's three GuildMember
  > blocks, EventMorph's and giveLotto never freed their Statement on
  > the success path, and IncomingPlayerManager's sweep leaked its
  > second Statement on the error path — the seam frees, fixed
  > knowingly; giveLotto's SELECT and its UPDATE/INSERT shared one
  > Statement and the sweep's per-player PCRoomUserInfo deletes reused
  > one — the seam uses one per call; loadSpecialEventCount's
  > `getRowCount() != 0` became `next()` (equivalent under
  > mysql_store_result); the `#if __PAY_SYSTEM_*` block in the session
  > end used to sit inside the BEGIN_DB block that converted its
  > SQLQueryException and now sits between two repository calls (dead in
  > every shipped build: all three macros are commented out in
  > PaySystem.h); the empty `if (getAffectedRowCount() == 0) {}` after
  > the LOGOFF update is gone; loadSpecialEventCount's `throw(const
  > char*)` now follows a bool (its unreachable `return;` is gone);
  > ConnectionInfoManager had no guard on its MAX probe and now throws
  > Error("Critical Error : ZoneGroupInfo table is empty.") where it
  > crashed; the gameserver's `addLogoutPlayerData` — its only call
  > commented out, the live copy in the loginserver's LoginPlayer.cpp —
  > is deleted rather than extracted, and with it the only
  > USERINFO.LogoutPlayerData write on this side; the dead PCRoomDBInfo
  > comment blocks in IncomingPlayerManager and ZoneGroupThread (the
  > latter's only mention of executeQuery) and IncomingPlayerManager's
  > commented-out duplicate UserIPInfo delete are deleted because R2's
  > grep is textual; the mojibake comments inside the replaced
  > session-end, event-count and lotto blocks are replaced by English
  > ones (the three GuildMember blocks keep theirs; EventMorph's Korean
  > comment is translated and IncomingPlayerManager's two are merged
  > into one English one), the GuildMissing.log format strings stay;
  > `DatabaseManager::setUserInfoConnection` frees a previously set
  > connection (init() is the only other assigning writer);
  > USERINFO.UserStatus has no primary or unique key, so its INSERT
  > IGNORE ignores nothing (faithful). Not enclosed: skill/Restore.cpp's
  > GuildMember LogOn = 0 write (built; the same unfreed Statement); the
  > session START side — CGConnectHandler's Player LogOn='GAME' and
  > GuildMember LogOn = 1 writes, CGPortCheckHandler's UserIPInfo
  > upsert, CGRequestIPHandler's and CGSayHandler's UserIPInfo reads
  > (handler/, R3); CGSayHandler's and billing/CommonBillingPacket.cpp's
  > Player LogOn / LastLogoutDate reads; src/server/PaySystem.cpp's
  > PCRoomUserInfo statements (ServerCore, every caller under the
  > disabled __PAY_SYSTEM_* macros); the loginserver's
  > LoginPlayerManager sweep and addLogoutPlayerData; and the unbuilt
  > src/server/IncomingPlayerManager.cpp fork that still carries the
  > whole sweep. No fake tier; +5 integration tests (the guild flag
  > scoped to one member; session end flipping only a 'GAME' row with a
  > server-side logout time and the boot sweep listing, clearing and
  > flipping only this server's accounts; the event counter read/saved
  > and false for an unknown account; the lotto row inserted with its
  > positional columns, read and counted; UserStatus update→false /
  > insert / update→true on USERINFO, plus the CurrentUser tinyint's
  > clamp at 127 pinned as observed). Remaining SQL under gameserver/:
  > R2 = 14 files — CreatureUtil.cpp (128, the character-deletion flow),
  > the guild trio (65), EventShutdown (18), TradeManager (deferred
  > above), SMSServiceThread (a thread that is never started),
  > SomethingGrowingUp.h, SystemAvailabilitiesManager (under two ORed
  > undefined macros) and five files no build target compiles
  > (Vampire_backup, GameServerInfoManager, EventMonsterNameManager,
  > GameWorldInfoManager, MoonCardUtil).
  > **ExpTable template (2026-09-02, stacked on the session round)**:
  > the one header R2 counted, SomethingGrowingUp.h — R2 14→13, R3
  > unchanged (it counts .cpp files). `ExpTable<...>::load` formatted
  > "SELECT %s, %s, %s FROM %s %s" from its subclasses' column and table
  > names (RankExpTable's "where RankType=<n>" condition, or "" — which
  > leaves the original's trailing space) into a heap buffer and handed
  > that string to executeQuery as a FORMAT — a second printf pass that
  > was harmless only because every '%' had already been expanded.
  > `BalanceInfoRepository::loadExpTable` takes the five identifiers as
  > data and formats once; the bytes on the wire are the same (the
  > template is 28 bytes and its five "%s" expand to nothing, so the
  > output is 18 + pieces against a buffer of 28 + pieces — nine bytes
  > of headroom for every input — and the original snprintf never
  > truncated). Its callers are the three ExpTable subclasses
  > (AdvancementClassExpTable, RankExpTable, SlayerAttrExpTable's
  > STR/DEX/INT tables). Disclosures: the original declared `Statement*
  > pStmt` uninitialised while END_DB deletes it (the RegenZoneManager
  > quirk again), and freed its `new char[]` query buffer with
  > SAFE_DELETE (delete, not delete[]) — both gone with the buffer; the
  > seam initialises its Statement; the original leaked that buffer on
  > every throwing path (END_DB rethrew before the trailing SAFE_DELETE,
  > and an AssertionError bypassed END_DB altogether, taking the
  > Statement with it), and the level Asserts now fire after the
  > statement is closed rather than mid-cursor (the partial fill of
  > m_Records is the same). STRBalanceInfo.AccumExp exceeds INT_MAX in
  > its top rows and truncates through getInt as it always did. No fake
  > tier; +1 integration test (RankEXPInfo with and without the
  > condition against COUNT(*) and per-level GoalExp,
  > AdvancementClassEXPInfo and STRBalanceInfo non-empty). Remaining SQL
  > under gameserver/: R2 = 13 files — CreatureUtil.cpp (128, the
  > character-deletion flow), the guild trio (65), TradeManager
  > (deferred above), and eight files whose SQL never runs:
  > EventShutdown (its two blocks sit in the `#else` of `#if
  > !defined(__THAILAND_SERVER__) && !defined(__CHINA_SERVER__)`) and
  > SystemAvailabilitiesManager (under the same two macros, which no
  > build file defines — ItemUtil.cpp's TU-local `#define
  > __THAILAND_SERVER__` reaches neither), SMSServiceThread (a thread
  > GameServer.cpp never starts, and its own connection), and the five
  > no build target compiles (Vampire_backup, GameServerInfoManager,
  > EventMonsterNameManager, GameWorldInfoManager, MoonCardUtil).
  > **Guild cluster (2026-09-02, stacked on the ExpTable round)**: the
  > last live cluster in the gameserver root — Guild.cpp,
  > GuildManager.cpp and GuildUnion.cpp — R2 13→10, R3 224→221, R5
  > 5,984→5,980 (the four __BEGIN_TRY macros inside the deleted dead
  > blocks). A new `GuildRepository` takes GuildMember (exists / insert
  > and re-join in the plain and waiting variants / load / save / delete
  > / the rank-plus-ExpireDate write behind expire() and leave() / intro
  > read and write / the boot-time Rank IN (0,1,2,3) list), GuildInfo
  > (insert with the corrected intro / load / save without the intro /
  > delete together with the guild's GuildUnionMember rows / the
  > GuildState IN (%d, %d) list / the name-and-master lookup the offer
  > list makes), the guild-scoped war reads (castle count and lookup,
  > the five-slot WarScheduleInfo count in WAIT/START, the
  > ReinforceRegisterInfo join, the two Status='START' counts), the
  > unions (insert returning the AUTO_INCREMENT id, member insert /
  > delete-reporting-a-hit / union delete, the two loads, the
  > guild→union and union→master lookups, the quoted-key member count)
  > and the union offers (the ten-day ESCAPE penalty count, the
  > stale-offer delete, JOIN and QUIT inserts, the OfferType+0 /
  > DATE_FORMAT list, the per-type UnionID lookups, delete and count).
  > Every literal byte-for-byte: the `Rank` backticks, the spaced
  > "GuildMember( ... ) VALUES ( ... )" against the unspaced union
  > inserts, the quoted numeric keys, the lower-case "and", "count(*)"
  > against "COUNT(*)", the doubled %% in the DATE_FORMAT literal; the
  > WORD guild ids and BYTE rank/type/race/state stream through "%d" or
  > "%u" exactly where they did. Dead code deleted: the gameserver's
  > SQL-bearing `#ifdef __SHARED_SERVER__` blocks, which no build
  > compiles (the sharedserver builds its own Guild.cpp /
  > GuildManager.cpp; nine SQL-free blocks and Guild.h's declarations
  > remain) — Guild::saveIntro / tinysave / saveCount,
  > GuildManager::init's whole body (the MaxGuildID and per-race
  > MaxZoneID probes and the trailing load() call), and deleteGuild's DB
  > purge — and hasWarSchedule's `#else` branch for the
  > __OLD_GUILD_WAR__ macro that Types.h comments out. Disclosures:
  > Statements that leaked and now do not — GuildUnion::destroy and
  > clearOffer; removeMasterGuild's two branches on their success paths
  > (the early returns freed; END_DB frees only on the exception path)
  > and the inner pStmt2; makeOfferList's pStmt2 on every loop iteration
  > and on the early return; acceptJoin's early returns; acceptQuit,
  > denyJoin and denyQuit on every path; hasOffer's false path — the
  > seam frees, fixed knowingly; GuildUnionManager::removeMasterGuild's
  > non-master branch ran its GuildUnionInfo lookup through the OUTER
  > statement (`pStmt`, not the `pStmt2` it had just created), which
  > freed the outer result it then kept reading (`pResult->next()`,
  > `pResult->getInt(1)`, `pResult->getInt(2)`) — a use-after-free:
  > Statement::executeQuery deletes its previous Result first, so
  > `pResult->getInt(2)` then read a one-column Result and threw
  > OutOfBoundException (after a ResultBug.log line), or freed memory —
  > removeGuild was never reached, and a non-master guild leaving a
  > union never completed. The seam reads the guild's UnionID and
  > OwnerGuildID before the master lookup, so that path works for the
  > first time (a knowing fix; the stray second `pResult->next()` is
  > gone with it); GuildManager::load read RequestDateTime only for
  > waiting members — the row reads it for every member (a nullable
  > datetime; getString gives "" for NULL); Korean comments inside the
  > replaced blocks are translated (the review found seven more in
  > removeMasterGuild's rewritten hunks; the fix commit translates
  > them), the GuildUnion.log format string and the comments outside the
  > replaced blocks stay; END_DB's DBError.log lines now name the
  > repository method rather than the caller; the boot-time member and
  > guild lists come back as row vectors where the originals streamed
  > the cursor (mysql_store_result had already buffered the rows). Not
  > enclosed: the sharedserver's own Guild.cpp / GuildManager.cpp
  > copies; the built duplicates of the union literals in handler/ —
  > CGDenyUnionHandler's and CGExpelGuildHandler's backticked count(*) /
  > DELETE GuildUnionInfo pair, CGQuitUnionAcceptHandler's and
  > CGQuitUnionHandler's unbackticked pair, and CGQuitUnionHandler's
  > ESCAPE offer INSERT (the write behind countRecentEscapes); the
  > GuildMember SQL in CGConnectHandler, CGJoinGuildHandler,
  > CGRegistGuildHandler, CGTryJoinGuildHandler,
  > quest/ActionShowGuildDialog.cpp and skill/Restore.cpp; war/'s
  > WarScheduleInfo reads and writes (War.cpp, WarScheduler.cpp) and
  > SiegeWar.cpp's ReinforceRegisterInfo statements; CGSayHandler's GM
  > commands (which touch the race tables, not the guild ones); and the
  > SG*Guild* handlers that mutate guild rows from the
  > SharedServerManager thread (SGAddGuildMemberOK, SGDeleteGuildOK,
  > SGModifyGuildMemberOK, SGModifyGuildOK). No fake tier; +5
  > integration tests (members created, re-joined in both variants,
  > saved, expired, intro round-tripped, the rank 0..3 list, deleted;
  > guilds created, loaded, saved without touching the intro, listed by
  > state, name/master looked up, deleted with their union rows; the
  > castle and war counts scoped to the guild and to WAIT/START; a union
  > created with its AUTO_INCREMENT id, members read, the guild→union
  > and union→master lookups, member delete reporting a hit then a miss,
  > union delete; offers inserted, read by type with the enum ordinal
  > and today's yymmdd, the ten-day ESCAPE penalty, stale offers aged
  > out, cleared). Remaining SQL under gameserver/: R2 = 10 files —
  > CreatureUtil.cpp (the character-deletion flow), TradeManager
  > (deferred above), and eight files whose SQL never runs
  > (EventShutdown, SystemAvailabilitiesManager, SMSServiceThread, and
  > the five in no build target).
  > **ItemIDRegistry (2026-09-02, stacked on the guild round; the first
  > step of the item milestone)**: item/ItemIDRegistry.cpp — R2
  > unchanged (item/ is not the gameserver root), R3 221→220, R5
  > 5,980→5,899 textually. The file held the 87 item classes'
  > `initItemIDRegistry` (EventBall and SubInventory declare one and
  > neither define nor call it): 81 hand-expanded copies of one body
  > plus six uses of a `%s` macro — 3,143 lines for two statements,
  > "SELECT COUNT(*) from <table>" and, only for a non-empty table,
  > "SELECT MAX(ItemID) FROM <table>". `ItemRepository` gains
  > `countItemRows(table)` / `loadMaxItemID(table)` (table-name-as-data
  > like `deleteItemRow`; the lower-case "from" and the upper-case
  > "FROM" are the originals'; getDWORD both, so a bigint ItemID above
  > 32 bits truncates as it always did), and the file collapses to one
  > helper (the count-then-MAX sequence, the successor/base rounding,
  > the boot cout) plus 87 one-line macro uses in the original
  > definition order, each naming its table and its cout label verbatim
  > — the class name for most, the table name for the six old macro uses
  > and for Relic / VampireWeapon / VampireAmulet, "GiftBox" for
  > EventGiftBox and "CoupleRing" for VampireCoupleRing. Each class
  > method keeps its own `__BEGIN_TRY` / `__END_CATCH` (the macro
  > expands them), so the stack annotation still names the class; R5's
  > drop is the grep counting one `#define` line where it counted 82
  > matched lines — 81 expansions plus the old macro's own. Disclosures:
  > END_DB's DBError.log line (and the const char* it throws) now names
  > the repository method rather than the class — and the class name was
  > the only thing there that identified the failing table, so a
  > boot-time failure now needs the Stmt line executeQuery prints to
  > cout; HolyWater's and BombMaterial's copies ran the same literals
  > through executeQueryString — the seam's `%s` route yields the same
  > bytes; Potion's two commented-out "DIST_DARKEDEN" connection lines
  > are gone; the `DB.h` include is gone with the SQL; the Korean header
  > comment is translated. Not enclosed: the 89 files under item/ that
  > still carry SQL — the 87 registry classes' own create / tinysave /
  > save / loader / info statements plus EventBall and SubInventory (the
  > rest of the item milestone). `ItemInfoManager.cpp` holds the 87
  > calls and no SQL. No fake tier; +1 integration test (PotionObject
  > counted before and after two inserts at 31005/31007, the maximum
  > pinned against the table's own MAX; the dump's rows sit at ItemID ≤
  > 8).
  > **Gear item classes (2026-09-02, stacked on the ItemIDRegistry
  > round; item milestone round 2)**: the nine slayer gear classes with
  > a Grade column — Ring, Bracelet, Necklace, Coat, Trouser, Shoes,
  > Glove, Helm, Shield — R3 220→211 (R2/R5 unchanged). Each class runs
  > the same seven statements: the create INSERT (a StringStream chain
  > through executeQueryString), the tinysave "SET %s", the save UPDATE,
  > the info manager's MAX(ItemType) and its 18-column SELECT, the
  > creature loader's owner SELECT (12 columns, Storage IN(0, 1, 2, 3,
  > 4, 9)) and the zone loader's zone SELECT (a StringStream chain, 11
  > columns — it never named Grade). The literals differ per class only
  > in the table name and in copy-paste whitespace ("Y,OptionType" in
  > Necklace / Trouser / Shoes, "ReqAbility,ItemLevel" in Bracelet, ", 
  > " before Coat's Grade value, "EnchantLevel = %d" in Glove), so a new
  > **`ItemObjectRepository`** has one method set — insertGear /
  > tinysaveGear / updateGear / loadMaxGearType / loadGearInfos /
  > loadGearOfOwner / loadGearInZone — and a `GearTable` enum that
  > selects the class's own seven literals from a spec table in the
  > MySQL impl, byte-for-byte quirks included. The two StringStream
  > chains are format strings there: every streamed expression maps to
  > the conversion StringStream used for its type (DWORD/WORD through
  > "%u" — uint/ushort overloads —, int through "%d", text as is), so
  > the bytes on the wire are the same; the tinysave and save literals
  > keep their "%ld" for the DWORD ids exactly as written, and every
  > write parameter is typed to what the caller streamed (Coat, Trouser,
  > Glove and Helm still pass `(int)getGrade()`, the others `getGrade()`
  > — Grade_t is int either way). Rows are typed to the getters: the
  > owner load's getDWORD ids / getBYTE x,y / getInt rest, the zone
  > load's getInt for every numeric column, the info load's
  > getInt/getString. The nine files were converted by one script
  > (scratchpad-only; the resulting text is what is reviewed), each
  > pattern required to match exactly once per file. Disclosures: the
  > commented-out StringStream blocks inside save() and the creature
  > loader — dead since the parameterized rewrite, each mentioning
  > executeQueryString — are gone with the blocks they sat in (R3's grep
  > is textual); `Statement* pStmt` declared uninitialised while END_DB
  > deletes it (create / save / info / both loaders; Trouser's zone
  > loader was the one initialised) — gone with the statements, the seam
  > initialises (the SQL-free third-loader stub keeps its own); the info
  > manager's MAX and SELECT shared one Statement, the seam uses one per
  > call; the loaders now read the whole result before placing the first
  > item where the originals interleaved (mysql_store_result had
  > buffered the rows already), so an item-placement throw no longer
  > leaks the Statement — the creature loader's UnsupportedError for a
  > Monster/NPC owner and the zone loader's for stash/corpse storage
  > did, and so did the zone loader's default-case Error; the creature
  > loader's default case freed first (`SAFE_DELETE(pStmt); // by sigi`,
  > gone with it); the create INSERT and the zone SELECT,
  > StringStream-built and run through the uncapped executeQueryString,
  > now go through executeQuery's 2048-byte format buffer — 188 and 161
  > bytes of format plus a varchar(10) owner and a varchar(30) option
  > field, so unreachable, but a new failure mode (Error("more buffer
  > size needed...")) all the same; END_DB's DBError.log lines now name
  > the repository method rather than the class. The third loader
  > overload, load(StorageID_t, Inventory*), keeps its empty `BEGIN_DB
  > {} END_DB(pStmt)` stub (no SQL; not touched) and so the DB.h include
  > stays. Not enclosed: the other 80 item files with SQL (their own
  > rounds: by live statement count 59 run these same seven with their
  > own column sets, 13 add destroy / saveBullet / hasPartnerItem /
  > setNewMotorcycle, PetItem adds savePetInfo, and seven —
  > CarryingReceiver, Dermis, Fascia, Mitten, Persona, ShoulderArmor,
  > WarItem — have an empty zone loader). ItemInfoManager.cpp holds only
  > the registry calls, no SQL. No fake tier; +2 integration tests
  > (every one of the nine tables: two rows of one owner inserted
  > through the class's own INSERT literal, the owner load returning
  > only the one in Storage IN(...) and the zone load only the one in
  > Storage 5 / the zone id, the full UPDATE read back, the tinysave
  > field write; every Info table's MAX against the table, its rows
  > counted, and the first row pinned by Name / NextItemType /
  > DowngradeRatio). Ids from 31000 up, cleaned in SetUp/TearDown.
  > **Vampire/ousters gear (2026-09-02, stacked on the gear round; item
  > milestone round 3)**: eight more classes of exactly the gear shape —
  > VampireRing, VampireBracelet, VampireNecklace, OustersRing,
  > OustersCoat, OustersCirclet, OustersPendent, OustersBoots — R3
  > 211→203 (R2/R5 unchanged). Same seven statements, same twelve /
  > eleven / eighteen columns, same getters and streamed expressions
  > (checked per class by a script against the pre-conversion text — the
  > script stays outside the repo; its output is what was reviewed — not
  > by eye: the previous round's function diff printed only the
  > neighbourhood of the first difference, which would have let two
  > impostors through — VampireCoat's Info SELECT has sixteen columns
  > and OustersStone's twenty, and both were converted and reverted
  > before this commit; VampireEarring guards an
  > `ifnull(MAX(ItemType),0)` — all three wait for a round that carries
  > their own Info rows). `GearTable` gains eight enumerators, the spec
  > table eight rows generated from the classes' original text (the nine
  > existing rows regenerate byte-identical; their label comments gained
  > the enumerator name), and the same transformer converted the eight
  > call sites, each pattern required to match exactly once. Literal
  > quirks kept: "Y,OptionType" in VampireRing's and VampireNecklace's
  > owner SELECT, the Ousters loaders' `Ousters* pOusters` placement
  > branches and the vampire loaders' wear-slot branches stay with the
  > classes untouched. Disclosures: the same as the gear round — the
  > seam initialises its Statement where the originals declared `pStmt`
  > uninitialised (in create / save / info / both loaders in all eight;
  > tinysave's `= NULL` is the family norm and Trouser's zone-loader one
  > has no counterpart here), one Statement per info statement, the
  > loaders read the whole result before placing the first item (an
  > item-placement throw no longer leaks the Statement; the creature
  > loader's `SAFE_DELETE(pStmt); // by sigi` inside the default case is
  > gone with the statement), END_DB's DBError.log lines name the
  > repository method, the create INSERT and the zone SELECT now pass
  > through executeQuery's 2048-byte format buffer (195–199 and 168–172
  > bytes of format plus a varchar(10) owner and a varchar(30) option
  > field — unreachable, but the new failure mode all the same), the
  > commented-out StringStream blocks in save() and the creature loader
  > are gone with their blocks, the third loader's empty `BEGIN_DB {}
  > END_DB(pStmt)` stub and the DB.h include stay. (The header counts
  > families of the seam; these paragraphs count item-milestone rounds,
  > ItemIDRegistry being round 1.) No new tests: the two ItemObjectMySQL
  > tests iterate a table array and now cover seventeen tables (two rows
  > through each class's own INSERT literal, the owner and zone loads,
  > the UPDATE read back, the tinysave field write; every Info table's
  > MAX and rows). Not enclosed: the other 72 item files with SQL;
  > ItemInfoManager.cpp holds only the registry calls. The spec table's
  > static_assert now reads GEAR_OUSTERS_BOOTS + 1.
  > **Gear with its own Info shapes (2026-09-02, stacked on the
  > vampire/ousters gear round; item milestone round 4)**: six classes
  > whose object statements are gear's but whose Info SELECT is not —
  > VampireCoat (16 columns: no UpgradeRatio / DowngradeRatio),
  > OustersStone (20: gear plus ElementalType, Elemental),
  > VampireEarring (gear's 18 behind `ifnull(MAX(ItemType),0)` and an
  > `if (next()) … else m_InfoCount = 0` guard), VampireWeapon and
  > OustersChakram (20 weapon columns: minDamage, maxDamage, Speed,
  > CriticalBonus where gear has Defense, Protection; the first thirteen
  > column names unspaced) and OustersWristlet (22: weapon plus
  > ElementalType, Elemental) — R3 203→197 (R2/R5 unchanged).
  > `ItemObjectRepository` gains a `GearInfoKind` and four Info rows and
  > loaders (`GearInfoNoRatioRow` / `loadGearInfosNoRatio`,
  > `GearInfoElementalRow` / `loadGearInfosElemental`, `WeaponInfoRow` /
  > `loadWeaponInfos`, `WeaponInfoElementalRow` /
  > `loadWeaponInfosElemental`); the spec row records each table's shape
  > and every info loader refuses a table of another shape with an
  > `Error`, so a mismatch throws instead of misreading the columns
  > silently (a longer row would drop its extra columns; a shorter one
  > would throw getField's OutOfBoundException with no hint which table)
  > — exactly the mistake the previous round's shape check caught.
  > GEAR_INFO_UNSET is the enum's zero, so a spec row that forgets its
  > kind is refused by every loader (the gear round's review asked for
  > the guard to fail closed); the kinds are named GEAR_INFO_*
  > throughout. The spec table's `static_assert` (the gear round's
  > review fix) now reads GEAR_OUSTERS_WRISTLET + 1. The transformer
  > feeds each class's setters from its own row (it checks the
  > original's setter sequence against the plan before rewriting) and
  > tolerates the `Statement* pStmt = NULL` the three weapon classes'
  > save() used. Literal quirks kept per class: "Y,OptionType" in
  > VampireEarring's owner SELECT, "Grade,EnchantLevel" in
  > OustersWristlet's, "OwnerID= '%s'" in the three weapon classes'
  > UPDATE, the unspaced weapon column names, VampireEarring's ifnull
  > literal. Disclosures: VampireEarring's else branch (`m_InfoCount =
  > 0` when the MAX row is absent) is gone — an aggregate without GROUP
  > BY always yields one row, ifnull only turns its NULL into 0, so the
  > branch was unreachable; the shared loader's `next(); getInt(1)`
  > reads the same 0 for an empty table; OustersStone's and
  > OustersWristlet's commented-out alternate Info SELECTs, which sat
  > inside the executeQuery argument list, are gone with the statement;
  > the rest as in the gear round (the seam initialises its Statement,
  > one Statement per info statement, whole-result reads before
  > placement, DBError.log names the repository method, the
  > commented-out StringStream blocks gone with their blocks, the third
  > loader stub and DB.h kept). No new object tests: the two
  > ItemObjectMySQL loops now cover 23 object tables and 18 standard
  > Info tables; +1 test for the four variant loaders (each pinned by
  > COUNT(*) and a column whose ordinal the shape moves or adds —
  > NextItemType, Elemental, minDamage / CriticalBonus, maxDamage /
  > ElementalType), VampireEarring's MAX, and the shape guard throwing
  > for three mismatched calls. Not enclosed: the other 66 item files
  > with SQL; ItemInfoManager.cpp holds only the registry calls.
  > **Silver weapons (2026-09-02, stacked on the Info-shapes round; item
  > milestone round 5)**: Sword, Blade, Cross, Mace — R3 197→193 (R2/R5
  > unchanged). Gear's INSERT, but a Silver column in the UPDATE and in
  > both loads — and the tail reorders rather than merely grows: the
  > UPDATE's and the owner SELECT's tail is EnchantLevel, Silver, Grade
  > where gear's is Grade, EnchantLevel, so Grade's ordinal moves too
  > (the save arguments run getDurability(), (int)getEnchantLevel(),
  > (int)getSilver(), (int)getGrade(); the owner load: 13 columns,
  > getters getDWORD ids / getBYTE x,y / getInt rest with Silver between
  > EnchantLevel and Grade); the zone SELECT is gear's plus Silver (12
  > columns, every numeric column through getInt, OptionType getString,
  > no Grade). So `ItemObjectRepository` gains a second object shape —
  > `GearObjectKind`, `SilverWeaponObjectRow` /
  > `SilverWeaponZoneObjectRow`, `updateSilverWeapon` /
  > `loadSilverWeaponOfOwner` / `loadSilverWeaponInZone` — and two more
  > Info shapes: SwordInfo / BladeInfo's 21 columns (the weapon shape
  > with MaxSilver after maxDamage; Blade's first fourteen names
  > unspaced) and CrossInfo / MaceInfo's 22 (an MPBonus before
  > MaxSilver) behind `SilverWeaponInfoRow` / `SilverWeaponMPInfoRow`.
  > The spec row now records the object shape as well as the Info shape,
  > and the update and load methods refuse a table of the other shape,
  > like the info loaders do (GEAR_OBJECT_UNSET is the object enum's
  > zero and every shape-checked method refuses it — insertGear,
  > tinysaveGear and loadMaxGearType never consult it — the same
  > fail-closed guard the Info kinds got in the previous round's review
  > fix); the spec table's `static_assert` now reads GEAR_MACE + 1.
  > insertGear, tinysaveGear and loadMaxGearType are shared: their
  > literals have gear's columns. The transformer (outside the repo; its
  > output is what was reviewed) gained the object shape: it checks the
  > original's save() argument list against the shape's expected list
  > before rewriting (Cross and Mace cast getGrade() in create; all four
  > in save). Literal quirks kept: "Y,OptionType" in Sword's owner
  > SELECT (the other three are spaced), Blade's unspaced Info columns
  > and its "OwnerID= '%s'" in the UPDATE, Cross's "StorageID=%d" (the
  > other three "%ld") in the UPDATE. Disclosures: the same as the gear
  > round — the seam initialises its Statement where the originals
  > declared pStmt uninitialised (create / save / info / both loaders in
  > all four, except Blade's save(), which had `= NULL`; tinysave's `=
  > NULL` is the family norm here too; the SQL-free third-loader stub
  > keeps its uninitialised pStmt), one Statement per info statement,
  > whole-result reads before placement (an item-placement throw no
  > longer leaks the Statement), DBError.log names the repository
  > method, the create INSERT and the zone SELECT now pass through
  > executeQuery's 2048-byte format buffer (189 and 170 bytes of format
  > — Mace's 188 and 169 — plus a varchar(10) owner and the option-list
  > text: unreachable), the commented-out StringStream blocks gone with
  > their blocks, the DB.h include kept. +1 integration test: for each
  > of the four tables, two rows of one owner through the class's
  > INSERT, the weapon UPDATE writing Silver, the owner load's thirteen
  > columns read back, the zone load's Silver at the column default and
  > empty for another StorageID, MAX(ItemType) against the seeded Info
  > table, the tinysave field write, the object-shape guard both ways,
  > and the two Info shapes pinned by COUNT(*) and MaxSilver / Speed /
  > MPBonus / DowngradeRatio with the guard refusing the other weapon
  > loaders. Not enclosed: the other 62 item files with SQL — next the
  > four guns (AR, SG, SMG, SR: BulletCount in the INSERT and UPDATE,
  > saveBullet) — and ItemInfoManager.cpp holds only the registry calls.
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
  > the original partial assignments; the one output-reusing caller sits
  > in the never-built legacy `gameserver/test/` dir).
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

- [ ] **3.4 Codify thread ownership.** Document (in CLAUDE.md) which state is
  owned by which thread: zone-group state mutated only on its
  `ZoneGroupThread`, cross-group communication via queues only. Add
  debug-build `assertOwnedByZoneThread()` checks on Zone/Creature mutation
  entry points (sidecar analog: "never block the registry mailbox" — the
  invariant is written down *and* asserted).
  > **Status:** contract documented in CLAUDE.md ("Thread ownership",
  > 2026-08-31): ownership is mutex-guarded, not thread-affine — the
  > `ZoneGroupThread` holds the group mutex for its whole tick and other
  > threads must take it. Debug-only `ZoneGroup::assertOwned()` guards
  > the five `Zone` mutation gateways `addPC`×2/`addCreature`/
  > `deleteCreature`/`moveCreature`. Hardened by the adversarial review:
  > the machinery rides `DE_OWNERSHIP_CHECKS` (Debug-only compile flag —
  > this repo never defines `NDEBUG`, so gating on it was a no-op and
  > the bookkeeping was live in release), a violation now `abort()`s
  > instead of throwing (an `AssertionError` is a `Throwable`, and the
  > `catch (Throwable&)` on these very paths swallowed it — e.g.
  > `GamePlayer::disconnect`'s empty catch would have skipped the
  > character save), `pthread_equal` + a valid flag replace the raw
  > compare/zero sentinel, and the review's main-thread hole is closed:
  > packets pipelined behind `CGReady` no longer drain on the main
  > thread after `GPS_NORMAL` opens the validator gate. Documented
  > violations (CLAUDE.md has the full list): SG/LG/GG handlers under
  > only the `PCFinder` lock; `EventMorph` tile writes below the
  > gateways; cross-group `DynamicZone` `addZone()`; three unlocked
  > `GDRLair*::start` loops.
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

- [x] **5.3 Fix log.** When restructuring uncovers real bugs (1.4 layout
  diffs, races, double-frees), record them in `docs/FIXES.md` with sidecar's
  `> **Status:**` convention rather than fixing silently.
  > **Status:** done — `docs/FIXES.md` created with the 1.4 max-size
  > reconcile entries (2026-08-31); the earlier Exchange defect set stays
  > recorded inline in 1.4 where it was written. Ongoing discipline, not a
  > one-shot: new finds keep landing there.

- [ ] **5.4 Language standard: C++11 → C++17 now, C++20 with the image bump.**
  Assessed 2026-08-30. No open task is *blocked* on the standard, but 3.1
  `Outcome` wants `std::variant` + `[[nodiscard]]` (an unchecked rejection
  becomes a compiler warning instead of a convention), the packet layer
  wants `string_view`/`optional`/`if constexpr`, and googletest is pinned at
  1.12.1 only because it is the last C++11 release. The Docker image is
  `ubuntu:20.04` → GCC 9.4: full C++17, only partial `-std=c++2a` (no
  `<ranges>`/`<format>`, incomplete concepts), so real C++20 is an infra
  change (22.04/GCC 11 or 24.04/GCC 13, which also moves `libmysqlclient`
  to the 8.0 client) and must not be bundled with the language switch.
  The only migration cost is pre-C++11 dynamic exception specifications;
  everything else removed by C++17/20 (`auto_ptr`, `bind1st`,
  `random_shuffle`, `hash_map`, `register`, …) is already absent:

  | Form | Measured | C++17 | C++20 |
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
  Not pursued: modules (4,271 TUs on GCC, no upside), coroutines (the
  thread-per-zone-group model stays, see non-goals), concepts (few templates).
  > **Status:** not started
  - Owner: ratchet R7, held at 0 once the sweep lands.

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
