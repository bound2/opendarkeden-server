# Fix log

Real bugs uncovered by the restructuring work (task 5.3 in
`docs/RESTRUCTURING.md`), recorded instead of fixed silently. Sidecar's
convention: every entry has a `> **Status:**` line updated in the same
commit as the fix.

The Exchange-reconcile defect set (SQL injection, string size/body
desync, `StringStream` stack overflows, unbounded listing counts, …) is
recorded inline in `docs/RESTRUCTURING.md` task 1.4, where it was found.
Entries below are newest first; the oldest is the 1.4 max-size reconcile
that followed it.

## Gameserver handshake write/read disagreements (2026-09-08)

The findings task 1.2 stated as flip-tests in
`tests/packet_gameserver_handshake_test.cpp`, plus the ones its review
reported that no test could state while they were open. Each is now
pinned as the behaviour it produces. No valid packet's bytes moved and
no golden changed; the effect-list max below is the set's only
`tests/wire-layout.txt` movement.

- **`PCSlayerInfo2` and `SubItemInfo` swallowed the exceptions their
  `read`/`write` raised.** Both wrapped their whole body in
  `try { ... } catch (Throwable& t) { cout ... }`, the shape
  `PCSlayerInfo` had on the login side. `PCSlayerInfo2::write()` raises
  on an empty PC name and on a guild name past 30; the object id was
  already on the wire when the throw happened, so the record stopped
  after four bytes while `GCUpdateInfo::getPacketSize()` — which
  `writePacket()` puts on the wire ahead of the body — still counted the
  whole record, and every packet after it was misframed.
  `PCVampireInfo2` and `PCOustersInfo2` never had the wrapper. The
  exceptions now leave both functions in both records.
  > **Status:** fixed (wire/update-info-disagreements)

- **`setGuildName` did not truncate.** All three `PCInfo2` records
  budget 30 in `getMaxSize()` and refuse more in `write()`, but the
  setter took any length, so a guild renamed past the cap made every
  `GCUpdateInfo` carrying one of its members unsendable. The setter now
  truncates to 30, the way the name setters in the same family do.
  > **Status:** fixed (wire/update-info-disagreements)

- **`NPCInfo::getSize()` counted fields `write()` omits.** `write()`
  emits the id and the two coordinates only behind a non-empty name;
  `getSize()` always added them, so one nameless NPC in a zone made
  `GCUpdateInfo` declare six bytes it never sent. `getSize()` now stops
  at the length byte for a nameless record, which is what `read()`
  expects.
  > **Status:** fixed (wire/update-info-disagreements)

- **`EffectInfo::getMaxSize()` was a flat 255.** `getSize()` is
  `szBYTE + 4 * ListNum` and `ListNum` is a BYTE, so a character
  carrying the maximum 255 effects needs 1021 bytes. The shortfall was
  budgeted into the factory max of every packet that embeds an effect
  list, which is what the receiver sizes its read buffer from. The max
  is now the full list, so nine factory maxima grow by 766 bytes in
  `tests/wire-layout.txt` (`GCAddMonster`,
  `GCAddMonsterFromBurrowing`, `GCAddMonsterFromTransformation`,
  `GCAddOusters`, `GCAddSlayer`, `GCAddVampire`,
  `GCAddVampireFromBurrowing`, `GCAddVampireFromTransformation`,
  `GCUpdateInfo`). Bytes on the wire are unchanged; this is the read
  buffer the server budgets, and the client sizes its own from its own
  copy of the max.
  > **Status:** fixed (wire/update-info-disagreements)

- **A settable `m_ListNum` that `addListElement` did not maintain.**
  `InventoryInfo`, `GearInfo`, `ExtraInfo` and `RideMotorcycleInfo` put
  `m_ListNum` on the wire and then wrote every element they held, so a
  count that disagreed with the list left the reader parsing fewer
  records than the body carried — and `getSize()`, which counts the
  list rather than the field, still matched the byte count, so the
  packet size looked correct all the way to the receiver. All four now
  maintain the count in `addListElement()` like `PCItemInfo` and
  `EffectInfo`, and `setListNum` is gone; every caller set it to the
  number it had just added, so no count on the wire changes.
  > **Status:** fixed (wire/update-info-disagreements)

- **`GCUpdateInfo` left most of its members indeterminate.** The
  constructor initialised ten of them. Among the rest was
  `m_pBloodBibleSign`, which `read()` wrote through without allocating —
  a write through an uninitialised pointer on every received packet.
  Every member is initialised now, `read()` allocates the sign record
  the way it allocates the other sub-records, and `write()` /
  `getPacketSize()` fall back to an empty record when none is installed,
  the way they already did for the nickname.
  > **Status:** fixed (wire/update-info-disagreements)

- **`NicknameInfo` had no constructor.** The NULL-nickname branch of
  `GCUpdateInfo::write()` builds one on the stack and emits it, so two
  indeterminate bytes of nickname id went on the wire for every player
  without a nickname record. The fields are zeroed now, and
  `setNickname` truncates to the 22 the max size budgets.
  > **Status:** fixed (wire/update-info-disagreements)

- **`GCPetInfo::read()` never restored the summon flag.** `write()`
  copies the packet's flag into the `PetInfo` it emits, so the wire
  carries it; `read()` left the packet's own copy at whatever the
  allocation held. The constructor zeroes it and `read()` takes it back
  from the `PetInfo` it read.
  > **Status:** fixed (wire/update-info-disagreements)

- **`CGSetVampireHotKey::toString()` read one past its array.** The
  eight hot keys live at indices 0..7; the debug string printed indices
  1..8, so it never showed the first key and read one element past the
  end. It now walks the eight it owns.
  > **Status:** fixed (wire/update-info-disagreements)

- **Three lists were unbounded against the widths their max sizes
  budget.** `GCUpdateInfo` writes its NPC record count as a BYTE while
  `getPacketSize()` counted the whole list, and its factory max budgets
  255 records; `BloodBibleSignInfo` budgets six signs and capped
  neither `write()` nor `getSize()`, so a seventh both wrapped the
  count byte and outgrew the max; the nickname was bounded on neither
  side. `addNPCInfo` now refuses the record past the count byte,
  `BloodBibleSignInfo` writes and counts at most the budgeted slots,
  and the nickname is capped in the setter and refused on read.
  > **Status:** fixed (wire/update-info-disagreements)

## Login-phase framing disagreements the CL/LC goldens found (2026-09-08)

The three write/read disagreements task 1.2 stated as flip-tests in
`tests/packet_login_test.cpp`. Each is now pinned as the refusal it
produces; no valid packet's bytes moved and no golden changed.

- **`PCSlayerInfo::read`/`write` swallowed the exceptions they raised.**
  Both wrapped their whole body in `try { … } catch (Throwable& t) { cout … }`,
  so the empty-name refusal that `PCVampireInfo` and `PCOustersInfo` let
  escape was printed and discarded here. `write()` carried on and emitted
  only the trailing advancement level while `getPacketSize()` still
  counted the full record — and `writePacket()` had already put that
  count on the wire, so the client read a body dozens of bytes short and
  every packet after it in the stream was misframed. The exception now
  leaves both functions.
  > **Status:** fixed (wire/login-packet-disagreements)

- **`LCRegisterPlayerOK`'s group name was bounded on neither side.** The
  setter took any length, `write()` narrowed it to a BYTE prefix with no
  check and `read()` accepted whatever length byte arrived, so an empty
  name wrote a zero prefix and a 256-byte name wrapped its prefix to
  zero — both undeliverable, because the stream's own string read rejects
  a zero length. The setter now truncates to `maxNameLength`, matching
  the width the factory max already budgeted, and both `read()` and
  `write()` refuse an empty or over-long name.
  > **Status:** fixed (wire/login-packet-disagreements)

- **`LCWorldList` / `LCServerList` accepted more entries than the
  factory max budgets.** Both budget 37 records of a 20-character name,
  and nothing capped the list at fill time, so the 38th world or server
  group in the login server's configuration made `getPacketSize()` exceed
  the max the client sizes its read buffer from — a truncated packet, not
  a caught error. `addListElement` now refuses the entry past the budget,
  and `WorldInfo` / `ServerGroupInfo` truncate their name to the width
  the budget allows, so the declared size can no longer outgrow the max.
  > **Status:** fixed (wire/login-packet-disagreements)

## Critical sections leaked their lock on non-Throwable exits (2026-09-05)

Found while making `__ENTER_CRITICAL_SECTION`/`__LEAVE_CRITICAL_SECTION` RAII.
The pair expanded to a bare `mutex.lock()` and a
`catch (Throwable&) { mutex.unlock(); throw; }` tail, so the lock survived any
exit the tail did not cover. Three defect shapes, all in the 463 sections under
`src/`:

- **`return` inside a section with no hand-written unlock.** Three live sites:
  `PCFinder::addNPC` (duplicate NPC name) and `PCFinder::deleteNPC` (unknown
  name) return holding `PCFinder::m_Mutex`, the lock every cross-thread SG/LG/GG
  handler takes through `getCreature_LOCKED`; `MPlayerManager::processResult`
  returns holding it from the `default:` branch of its error-code switch, on the
  mofus thread. `Mutex::lock()` detects a same-thread relock and throws `Error`,
  so the first symptom is a `SELF DEAD LOCK` line in `MutexError.log`; another
  thread blocks instead. The other 107 early returns unlocked by hand.
- **Non-`Throwable` exceptions.** `std::bad_alloc`, `std::out_of_range` and the
  `const char*` that `END_DB` rethrows do not match `catch (Throwable&)`, so any
  of them crossing a section boundary left the mutex held. The `const char*`
  case was already recorded in a comment in `SGDeleteGuildOKHandler.cpp`, which
  said the general fix belonged in `__LEAVE_CRITICAL_SECTION`. (`END_DB` throwing
  `msg.c_str()` from a local `string` — a dangling pointer — is a separate
  defect and is still open.)
- **Double unlock.** `Guild::addMember` and its sharedserver twin do
  `m_Mutex.unlock(); throw DuplicatedException();` inside a section; the
  `Throwable` tail then unlocked the same non-recursive pthread mutex a second
  time, which is undefined behaviour.

The section is now a block guarded by a scoped `CriticalSection` object, so the
lock is released on every exit — end of block, `return`, `goto`/`continue` out
of it, and any thrown type. The 120 hand-written unlocks inside sections were
removed (114) or routed through the guard (6, where work deliberately ran
unlocked before the return). `tests/critical_section_tests` pins the exits and
`tests/tools/critical_section_audit.pl` fails the class on sight.
> **Status:** fixed (cpp20/raii-critical-sections)

## loginserver and sharedserver could not be shut down (2026-09-05)

Found while extending PR #89's cooperative lifecycle to the other two server
processes. Neither `main()` installed a signal handler, so SIGTERM took the
default disposition and killed the process outright — no worker join, no
socket drain, no chance for the loop to finish its turn. There was no
graceful path either: `LoginServer::stop()` and `SharedServer::stop()` both
began with `throw UnsupportedError()` (the sub-manager stops after it were
dead code), `ClientManager::stop()`/`HeartbeatManager::stop()` threw the
same, and the `GameServerManager` workers derived from the legacy pthread
`Thread`, whose `stop()` also throws — their `while (true)` loops had no exit
at all. The loginserver's UDP listener compounded it: the socket was left
blocking, so the worker sat inside `recvfrom` for as long as the game servers
stayed quiet. The container supervisor's unbounded final `wait` matched the
old behaviour (login/shared died instantly), so it too had to be bounded once
they started draining. Every worker now uses `ManagedThread`, both mains
install the SIGTERM/SIGINT handler and the named 30-second failed-exit
watchdog, `stop()` is idempotent and joins before dependencies are released,
and `docker/start.sh` gives login/shared 8 seconds before SIGKILL.
> **Status:** fixed (cpp20/login-shared-managed-threads)

## sharedserver keep-alive query runs every tick, not hourly (2026-09-05)

Found while migrating `sharedserver/GameServerManager::run()`. Its
connection keep-alive reschedules with `dummyQueryTime.tv_sec = (60 + rand()
% 30) * 60;` — a plain assignment where the gameserver's `LoginServerManager`,
`SharedServerManager`, `SMSServiceThread` and the loginserver's
`ClientManager` all use `+=`. The absolute deadline therefore lands about an
hour after the epoch, is always in the past, and `executeDummyQuery()` fires
on every pass of a loop that now turns roughly every millisecond. Left alone
here deliberately: the lifecycle migration changed no thread's actual work.
> **Status:** open

## Self-initialised pointer in sharedserver processOutputs() (2026-09-05)

`GameServerManager::processOutputs()` writes `GameServerPlayer*
pGameServerPlayer = pGameServerPlayer;` inside the socket-error branch,
shadowing the live outer pointer with one initialised from itself, and then
`delete`s it. Clang reports it (`-Wuninitialized`) on every build. The same
file never initialises `m_pGameServerPlayers[nMaxGameServers]`, so its
unwritten slots are indeterminate rather than NULL — the `m_MinFD`/`m_MaxFD`
window is what keeps the loops off them today.
> **Status:** open

## Cooperative lifecycle and process shutdown gaps (2026-09-05)

Adversarial review of PR #89 found unsynchronized start/stop publication,
pool startup leaving its mutex locked after thread-construction failures,
uncaught worker exceptions, and a container SIGTERM path that bypassed joins.
The gameserver auxiliary workers could also outlive zone/database dependencies.
`ManagedThread` now serializes lifecycle operations, retains worker failures,
and requests process shutdown. The pool uses RAII and rolls back failed starts.
All gameserver workers are stopped/joined before dependencies may be released;
SIGTERM/SIGINT drain the main loop, and the supervisor keeps peer services alive
until gameserver finishes. Idle UDP reception is nonblocking. MySQL options
bound individual network operations; a 30-second watchdog gives stuck work a
failed process exit, without destroying live worker state. Normal main exit
reclaims the legacy singleton graph through the OS after joins instead of
invoking its unaudited destructor ordering. This adds no world-save guarantee.
Lifecycle, signal, supervisor and silent-MySQL-peer regression tests accompany
the fixes. CMake checks actual C++20 library support; CI runs the pinned toolchain.
> **Status:** fixed (codex/cpp20-jthread-lifecycle)

## Zone workers could outlive their zone state during teardown (2026-09-04)

Found while migrating `ZoneGroupThread` to `std::jthread`: the gameserver
destructor deleted `g_pObjectManager` before `g_pThreadManager`, even though
the zone workers continuously dereference zone state owned below the object
manager. The zone pool's `stop()` was also an `UnsupportedError`, so teardown
could either trip its exit-state assertion or let a worker observe freed state.
The pool now requests cancellation for every zone, joins every worker, and is
destroyed before the client, object, and database managers.
> **Status:** fixed (codex/cpp20-jthread-lifecycle)

## SGModifyGuildMemberOK was never handled: `#ifdef __GAME_SERER__` (2026-08-31)

Found while migrating the SG direction onto the dispatch table (task
2.3): `SGModifyGuildMemberOK::execute()` wrapped its handler call in
`#ifdef __GAME_SERER__` — a misspelling of `__GAME_SERVER__` that no
build defines — so the shared server's acknowledgement of a guild-member
rank change was silently dropped by every game server, leaving the
in-memory `Guild` stale until reload. The other nine SG packets spell
the guard correctly. The dispatch table now registers the handler like
its siblings, which both fixes the bug and makes the class of bug
impossible: registration is plain code at the composition root, not a
per-file macro spelling.
> **Status:** fixed (restructuring/dispatch-cg)

## Types.h include order broke the container build (2026-08-31)

The Phase 2 scaffolding PR (#14) clang-formatted `Types.h` *after* its
verification build had already synced sources into the container volume:
the formatter sorted `#include "Utility.h"` above the `types/` block, but
`Utility.h` uses `BYTE`/`WORD`/`sz*` from those headers and its own
`#include "Types.h"` is an empty no-op mid-expansion (the guard is
already set) — so every TU failed on a fresh build while the stale
volume kept passing. Fixed by pinning `Utility.h` below the `types/`
block behind `// clang-format off`. Lesson recorded: **re-run the build
after formatting**, not before.
> **Status:** fixed (restructuring/dispatch-cg)

## Wire max-size reconcile (both repos, 2026-08-31)

The cross-repo inventory diff (`tests/tools/wire_inventory_diff.sh`)
exits 0 as of this set. "Server" = this repo, "client" = the `client`
repo's hand-copied packet classes. None of these changed live wire
*bytes*; they changed the size bookkeeping (`getPacketSize()` /
`getPacketMaxSize()`) that frames and validates them — except where
noted.

- **`CGUseItemFromInventory` / `CGSkillToInventory` phantom field
  (server).** `getPacketSize()`/`getPacketMaxSize()` counted
  `m_InventoryItemObjectID`, which `read()`/`write()` skip (commented
  out). Dormant only because the server never writes CG packets. Found
  by the 1.2 encrypter pins; stated as a fact by the
  `..._WITH_SIZE_DRIFT` tests until fixed.
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`LCPCList::getPacketSize()` under-reported by SLOT_MAX (server).**
  `write()` emits one PC-type char per slot before the info bodies; the
  size never counted them, so every character-list packet's size header
  was 3 bytes short. Worked only because TCP usually delivers the whole
  packet at once, so the client's length check passed anyway; a
  fragmented delivery could throw `InsufficientDataException` mid-parse.
  Separately, the trailing `m_Agree` byte is written only under
  `__NETMARBLE_SERVER__` (never defined by the build) but was counted
  unconditionally in `getPacketMaxSize()`; the accounting is now guarded
  the same way as the write.
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`GCUseOK` client cap dropped large use results (client).** The
  client's factory hardcoded `getPacketMaxSize() = 255` while the body
  is one `ModifyInfo` (max 2042). A use result with more than ~36
  modify entries was rejected by `Player.cpp`'s size guard (bug report +
  disconnect). Now returns `ModifyInfo::getPacketMaxSize()`.
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`PCSlayerInfo2` dead statements after `return` (client).** Both
  `getSize()` and `getMaxSize()` ended `+ szLevel; + szExp; + szBonus;`
  — the last two are discarded expression statements, so the sums missed
  6 bytes that `read()`/`write()` do transfer (`m_AdvancementGoalExp`,
  `m_AttrBonus`). Undercounted the max of every packet embedding it
  (`GCUpdateInfo`, `GCMorph1`, …).
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`InventoryInfo::getMaxSize()` phantom Width/Height (client).**
  Counted two `szCoordInven` for fields commented out of
  `read()`/`write()`.
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`GCAddMonsterCorpse` client max missed the `hasHead` byte.**
  `read()`/`write()` transfer `m_bhasHead`; the client's max summed one
  `szBYTE` too few, rejecting only a maximal (128-char-name) packet.
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`GCExecuteElement` server max 3 for a 7-byte body.** The factory
  summed `szBYTE + szWORD`, omitting the leading `DWORD` quest id.
  Harmless live (server only sends it) but wrong as contract.
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`GCNPCResponse` server max counted `szBYTE` for a `WORD` code.**
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`CLLogin` server max was the dead netmarble sso formula.**
  `szint + 2048 + …` = 2090, while `read()` accepts at most
  1+30+1+30+6+1 = 69 and throws on anything longer. Client's copy also
  drifted (counted a 20-byte password cap; its own `write()` clamp stays
  20, the server accepts up to 30). Both now state 69.
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`CGSMSSend` message cap disagreed (80 server / 40 client).** The
  client's `write()` asserts `size < 40`, so the server's extra 40 bytes
  of acceptance were unreachable. Aligned both at 40.
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`GLIncomingConnectionError` max wrong in both repos.** The body is
  two length-prefixed strings each capped at 127 by `read()`/`write()`
  (max 256); the server counted one string (129), the client counted
  `szBYTE + 80` (81). Datagram-only (game→login), so never framed on the
  TCP wire.
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`LCServerList` / `LCWorldList` server max missed the list-count
  byte** that `write()` emits between the id and the infos (their
  dynamic `getPacketSize()` counted it correctly).
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`CGBloodDrain` stale client factory max.** Flagged in 1.4 as a
  layout mismatch, but the client's X/Y/Dir reads/writes were already
  commented out to match the server — only the factory max (7) still
  described the old layout. Now 4 (`szObjectID`) in both repos. The
  client's only send site remains commented out (`MPlayer.cpp:3457`).
  > **Status:** fixed (restructuring/wire-maxsize-reconcile)

- **`getDBString` wrote up to two bytes past its `char[100]` with
  client-controlled input.** The legacy SQL escaper (formerly in
  `CGModifyNicknameHandler.cpp`, now single-sourced in
  `MySQLNicknameRepository.cpp`) appended the escape byte and the NUL
  terminator past the buffer whenever an escaped character landed on the
  boundary — and `CGModifyNickname` feeds it a client-supplied string of
  up to 255 bytes with only an empty-string check in front. Rewritten as
  a bounded `std::string` accumulator: byte-identical output for every
  input the old code survived, clean truncation at the same ~100-byte
  horizon for the rest (the column is `varchar(22)` regardless). Found
  by the 3.2 adversarial review.
  > **Status:** fixed (restructuring/repository-pilot)
