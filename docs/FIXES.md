# Fix log

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

Real bugs uncovered by the restructuring work (task 5.3 in
`docs/RESTRUCTURING.md`), recorded instead of fixed silently. Sidecar's
convention: every entry has a `> **Status:**` line updated in the same
commit as the fix.

The Exchange-reconcile defect set (SQL injection, string size/body
desync, `StringStream` stack overflows, unbounded listing counts, …) is
recorded inline in `docs/RESTRUCTURING.md` task 1.4, where it was found;
entries below start with the 1.4 max-size reconcile that followed it.

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
