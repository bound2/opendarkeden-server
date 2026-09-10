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

## Store, shop and stash write/read disagreements (2026-09-10)

The nine findings task 1.2 stated as flip-tests in
`tests/packet_store_test.cpp`, the initialisation split the same file
pinned, and two entry counts the inventory round left near-missed. These
packets are client-facing, so `write()` and `getPacketSize()` are the
contract: every fix is a refusal, a cap at the width the factory max
already budgets, a size-accounting correction, a read-side correction or
an initialisation. **No golden changes.** Six `tests/wire-layout.txt`
lines move — `GCPetStashList` 1021 → 1020, `GCShopBought` 284 → 59,
`GCShopBuyFail` 4 → 9, `GCShopBuyOK` 287 → 62, `GCShopList` 5515 → 1015
and `GCStashList` 21006 → 7506 — all read-buffer budgets, not fields on
the wire.

- **`GCShopBuyFail::getPacketSize()` counted the NPC id alone while
  `write()` also emitted the fail code and the four-byte amount, and the
  factory max was the same four bytes.** Every refusal declared a body
  five bytes shorter than it sent, so `writePacket()` put a length on the
  wire that did not describe the body behind it and the stream never
  resynchronised; the receiver's read buffer, sized from the max, was
  overrun by the same five bytes. The size and the max count all three
  fields: `GCShopBuyFail` 4 → 9.
  > **Status:** fixed (wire/store-disagreements)

- **`CGStoreSign` derived the sign's length byte by hand and capped
  nothing.** A sign past the 80 bytes its factory max budgets was emitted
  whole, and at 256 bytes the length byte wrapped to zero while the text
  still followed it. The field goes through `de::wire::readString` /
  `writeString` at `MAX_SIGN_SIZE`, and the setter refuses a longer one,
  so the length always describes the bytes behind it. A sign of 0..80
  bytes travels unchanged.
  > **Status:** fixed (wire/store-disagreements)

- **`CGStoreSign::write()` emitted an empty sign as a bare zero length
  byte that `read()` could not take back.** The hand-written sequence
  handed the zero to `SocketInputStream::read(string&, uint)`, which
  refuses a zero length, so a stall named with an empty string was
  written and never read. The helper's bounds admit an empty value
  (`{0, MAX_SIGN_SIZE}`) and `write()` is byte-identical.
  > **Status:** fixed (wire/store-disagreements)

- **`StoreInfo::setSign` capped nothing and `StoreInfo::write` bounded
  the sign at 255 while `StoreInfo::getMaxSize` budgets 80.** A full
  stall whose sign was longer made `GCMyStoreInfo` and `GCOtherStoreInfo`
  outgrow their factory maxima by the overage. The setter cuts at
  `MAX_SIGN_SIZE`, the way `StoreOutlook::setSign` already did — the two
  carriers of the same player-typed field behave alike, and
  `tests/packet_zone_scan_test.cpp` pins the cut for both — and `read()`
  and `write()` bound the field there too, so a peer cannot smuggle a
  longer one past the record's budget.
  > **Status:** fixed (wire/store-disagreements)

- **`GCShopBought`, `GCShopBuyOK`, `GCShopList` and `GCStashList` bounded
  their option lists against nothing.** The count travels in a `BYTE`, so
  the 256th option wrapped it to zero while `write()` still emitted every
  option, and the four maxima budgeted 255 options per item where an item
  carries at most `MAX_ITEM_OPTION_NUM` (30) — a code sheet's stone grid.
  The lists are held to that bound in the adders, the setters, `write()`
  and `read()`, and the maxima budget it: `GCShopBought` 284 → 59,
  `GCShopBuyOK` 287 → 62, `GCShopList` 5515 → 1015 and `GCStashList`
  21006 → 7506, the last two twenty and sixty slots deep.
  > **Status:** fixed (wire/store-disagreements)

- **The same four read their option list into whatever the packet already
  held.** A reused packet grew by one listing per read, so it declared
  one listing and held two. `GCShopBought::read` and `GCShopBuyOK::read`
  clear the list first; `GCShopList::read` and `GCStashList::read` reset
  every slot, which is what a full listing replaces, and `GCStashList`
  destroys the sub-item records it drops instead of leaking them.
  > **Status:** fixed (wire/store-disagreements)

- **`GCPetStashList` never put its code byte on the wire although three
  senders set it and its factory max budgeted a byte for it.** The client
  decides which side is right and it reads no code:
  `Client/Packet/Gpackets/GCPetStashList.cpp` reads, per slot, an
  availability byte, the `PetInfo` and the keep days, and
  `GCPetStashListHandler` never calls `getCode()`. So the wire is right
  and the code was dead API: `m_Code` / `getCode` / `setCode` are gone,
  with the `setCode` calls in `ActionPetDeposit`, `ActionPetWithdraw` and
  `CGSayHandler`'s `PetStash` command, and the max no longer budgets a
  byte no body carries: `GCPetStashList` 1021 → 1020.
  > **Status:** fixed (wire/store-disagreements)

- **`GCStashList` kept each slot's sub-item count in an array of its
  own while the records came from a separate list.** A sub-item added
  through `getSubItems()` left the count behind, so `write()` emitted the
  stale count and then the whole list and `getPacketSize()` counted the
  stale one. The count is the list, refused past the eight belt pockets
  the max budgets in `write()` and in `read()`; the count array and the
  bookkeeping in `GCStashListFill.cpp` are gone, and a slot entry holding
  no record is refused rather than skipped, which would leave the body one
  record short of the count.
  > **Status:** fixed (wire/store-disagreements)

- **`GCMyStoreInfo` and `GCOtherStoreInfo` left their `StoreInfo`
  pointer uninitialised, and both `getPacketSize()` and `write()`
  dereferenced it.** A sender that skipped `setStoreInfo` followed an
  indeterminate value. The pointer starts empty and `getPacketSize()`,
  `write()` and `read()` throw `InvalidProtocolException` on it.
  > **Status:** fixed (wire/store-disagreements)

- **Twenty-five of the thirty-six left at least one member the default
  constructor never set**, so a packet sent without every setter called
  put whatever the allocation held on the wire. All thirty-six initialise
  every member now, pinned by constructing each over storage poisoned
  with two different bytes.
  > **Status:** fixed (wire/store-disagreements)

- **`GCTimeLimitItemInfo` and `CGTypeStringList` derived an entry count
  from a list they bounded against nothing.** Both counts travel in a
  `BYTE`; `GCTimeLimitItemInfo` only `Assert`ed the bound and
  `CGTypeStringList` did not check at all, so a listing past what the
  factory max budgets outgrew the receiver's read buffer. Each is held to
  the number its own max already budgets — 100 time-limit entries, 20
  strings of 50 bytes — in the adder, in `write()` and in `read()`. Both
  maxima were already right, so no wire-layout line moves.
  `CGTypeStringList::popString` called `front()` on a list that may be
  empty and throws now, and the packet initialises its type and its
  parameter.
  > **Status:** fixed (wire/store-disagreements)

## Inventory and item handling write/read disagreements (2026-09-10)

The fourteen findings task 1.2 stated as flip-tests in
`tests/packet_inventory_test.cpp`, plus the one its PR recorded rather
than tested. These packets are client-facing, so `write()` and
`getPacketSize()` are the contract: every fix is a refusal, a cap at the
width the factory max already budgets, a size-accounting correction, a
read-side correction or an initialisation. One golden pair moves
deliberately — `GCMakeItemOK` carried a count byte twice and the client
reads it once — and no other golden changes. The three
`tests/wire-layout.txt` movements are read-buffer budgets, not fields on
the wire.

- **`GCAddItemToInventory::write()` emitted the option count twice while
  `read()` consumed it once, and it wrote an item count
  `getPacketSize()` never budgeted for.** `GCMakeItemOK`, the only
  packet that puts the record on the wire, declared a body two bytes
  shorter than it sent, so the receiver took the second count as the
  first option, read every field behind the option list a byte early and
  ran the stat record off the end of the body. The client decides which
  side is right and it reads one count: its own
  `Client/Packet/Gpackets/GCAddItemToInventory.cpp` reads the object id,
  the two inventory coordinates, the class, the type, **one** option
  count, the options, the durability and the item count, and
  `GCMakeItemOKHandler` uses `getItemNum()`. So the duplicated write is
  the bug: `write()` emits one count and `getPacketSize()` counts the
  item count. `tests/golden/GCMakeItemOK.code0.hex` and
  `GCMakeItemOK.nooptions.code0.hex` are re-recorded one byte shorter —
  a deliberate wire fix, and the only golden change in the set.
  > **Status:** fixed (wire/inventory-disagreements)

- **The record's option list was bounded against nothing.** The count
  travels in a `BYTE`, so the 256th option wrapped it to zero while
  `write()` still emitted every option. The list is held to
  `MAX_ITEM_OPTION_NUM` (30) in the adder, in the setter, in `write()`
  and in `read()`. That is the widest option list an item carries: a
  code sheet keeps its stone grid there and holds exactly 30
  (`CodeSheet::CodeSheet` pads to 30, `CGAddItemToCodeSheetHandler`
  refuses a sheet with fewer), and every other item class holds at most
  three.
  > **Status:** fixed (wire/inventory-disagreements)

- **`GCAddItemToItemVerify::getPacketSize()` had no case for its
  THREE_ENCHANT_OK branch.** That result declared a bare code byte while
  `write()` gave it the code and two parameters, so the packet put a
  length on the wire eight bytes short of the body behind it. The size
  counts both parameters now.
  > **Status:** fixed (wire/inventory-disagreements)

- **`GCAddItemToItemVerify`'s second parameter was the one member the
  constructor left alone**, and the THREE_ENCHANT_OK branch put it on
  the wire, so a sender that skipped `setParameter2()` sent whatever the
  allocation held. It starts at zero.
  > **Status:** fixed (wire/inventory-disagreements)

- **`GCChangeInventoryItemNum` kept its material count in a `BYTE` it
  incremented per entry and bounded nowhere.** The 256th material
  wrapped the count to zero while `write()` still emitted every pair;
  `setChangedItemListNum()` let the count and the lists disagree, and
  the declared size then described neither; and
  `popFrontChangedItemListElement()` took an entry off the list without
  taking it off the count. The count is the list now, refused past
  `kMaxCount` — 255, what the count byte carries — in the adder and in
  `write()`; the setter is gone; popping an empty list is an
  `InvalidProtocolException` rather than a `front()` on nothing; and
  `read()` clears before it parses. The bound is the count byte's
  because the record has no live fill site to take one from:
  `CGMakeItemHandler::execute` is commented out in its entirety, no
  source calls `addChangedItemListElement`, and there is no recipe table
  in `initdb/` saying how many materials a craft consumes.
  > **Status:** fixed (wire/inventory-disagreements)

- **A full material list was five times what the two crafting factory
  maxima budgeted for it.** Both budgeted a flat 255 bytes for a record
  that reaches 1 + 255 * (`szObjectID` + `szItemNum`) = 1276, and
  `GCMakeItemOK` budgeted another flat 255 for an item record it can now
  measure. Both maxima are built from the two records' own
  `getPacketMaxSize()`: `GCMakeItemFail` 2297 -> 3318 and `GCMakeItemOK`
  2552 -> 3363, server-side read-buffer budgets and not fields on the
  wire.
  > **Status:** fixed (wire/inventory-disagreements)

- **`GCCreateItem` derived its option count from the list and bounded
  nothing**, so the 256th option wrapped the count and the body outgrew
  the 255 options its factory max budgeted. It is held to the same
  `MAX_ITEM_OPTION_NUM` in the adder, the setter, `write()` and
  `read()`, and its max budgets that: `GCCreateItem` 277 -> 52.
  > **Status:** fixed (wire/inventory-disagreements)

- **`GCGQuestInventory` counted its item list in a `BYTE` it never
  bounded, and `read()` appended to the list the packet already held.**
  A guild-quest inventory past 255 items wrapped the count, one past the
  100 the factory max budgets outgrew the receiver's read buffer, and a
  packet read into twice declared one listing and held two. The list is
  held to `MAX_GQUEST_INVENTORY_ITEM_NUM` in a new `addItem()`, in
  `write()` and in `read()`, and `read()` clears before it parses. The
  server's own adds go through `GQuestInventory::addOne`.
  > **Status:** fixed (wire/inventory-disagreements)

- **`GCTimeLimitItemInfo::getTimeLimit()` answered `0xffff` for an item
  it did not hold**, a value a real remaining time can equal, so a
  caller could not tell an absent item from one with 65535 seconds left.
  It returns `std::optional<DWORD>` now, with `hasTimeLimit()` beside
  it. The wire is untouched; the packet has no caller of the accessor in
  `src/server`.
  > **Status:** fixed (wire/inventory-disagreements)

- **Thirty of the thirty-seven packets left at least one member the
  default constructor never set**, so a packet sent without every setter
  called put whatever the allocation held on the wire. All thirty-seven
  initialise every member now, pinned by constructing each over storage
  poisoned with two different bytes and requiring the same body.
  > **Status:** fixed (wire/inventory-disagreements)

- **Three string fields still read a length into a local and handed it
  straight to `read(string&, uint)`** — `CGTypeStringList`'s list
  strings, `GCShowWaitGuildInfo`'s founding-member names and
  `GCSMSAddressList`'s three `AddressUnit` fields — a shape R9's grep did
  not match because it only looked for reads into a member. All go
  through `de::wire::readString`/`writeString` at the bounds their
  factory maxima budget, and R9's grep accepts any identifier and still
  measures 0.
  > **Status:** fixed (wire/inventory-disagreements)

- **Four `pop*()` methods called `front()` on a list that may be
  empty** — `GCNPCAskDynamic::popContent`,
  `GCRemoveEffect::popFrontListElement` and
  `ModifyInfo::popShortData`/`popLongData` — and so did the eleven
  `popCListElement()` copies in the tile, bomb and mine packets. All
  throw `InvalidProtocolException` on an empty list.
  > **Status:** fixed (wire/inventory-disagreements)

- **`Script::addContent` threw a raw string literal past
  `SCRIPT_MAX_CONTENTS`**, which no `__END_CATCH` can catch, so a script
  with a sixteenth content terminated the process instead of being
  reported. It throws `Error`.
  > **Status:** fixed (wire/inventory-disagreements)

## Movement, effect-lifecycle and NPC dialogue write/read disagreements (2026-09-10)

The ten findings task 1.2 stated as flip-tests in
`tests/packet_movement_test.cpp`, plus the three its PR recorded rather
than tested, and the blast-list bounds the combat round left open in the
bomb and mine packets. These packets are client-facing, so `write()` and
`getPacketSize()` are the contract: every fix is a refusal, a cap at the
width the length byte or the factory max already budgets, a
size-accounting correction or a read-side correction to what `write()`
emits. No golden changed; the seven maxima below are the set's only
`tests/wire-layout.txt` movements, and the five bomb and mine goldens are
new files.

- **`GCNPCAskDynamic::getPacketSize()` did not count the choice-count
  byte `write()` emits.** `writePacket()` puts that size on the wire
  before calling `write()`, so every question declared a body one byte
  shorter than the one that followed it and the stream never
  resynchronised. The size counts the byte now, and so does the factory
  maximum, which had left it out too.
  > **Status:** fixed (wire/movement-disagreements)

- **`GCNPCAskDynamic` counted its choices in a `BYTE` it incremented per
  entry and bounded the list against nothing.** The 256th choice wrapped
  the count to zero while `write()` still emitted every string. The count
  is the list now, bounded by `kMaxCount` — 15, refused in
  `addContent()`, in `write()` and in `read()`. The bound is the fill
  site's own: `ActionAskDynamic` adds one choice per `Script` content and
  `Script` holds at most `SCRIPT_MAX_CONTENTS` (15) of them, so the
  refusal is unreachable. The factory max budgeted ten, which the shipped
  data already passes — of 1155 `Script` rows two carry 11 and 12
  contents — so it budgets 15 now: `GCNPCAskDynamic` 11294 -> 16425, a
  read-buffer budget rather than a field on the wire.
  > **Status:** fixed (wire/movement-disagreements)

- **`GCNPCAskDynamic::write()` emitted a zero-length choice that `read()`
  counted and dropped.** The packet read back declared one more choice
  than it held, so forwarding it wrote one fewer field than its own count
  promised. `read()` keeps the empty choice, which is the side that
  matches `write()`: the field is a well-formed zero-length string, the
  size counts its length word, and `ScriptManager` trims each
  `**`-separated chunk into the script whatever it holds.
  > **Status:** fixed (wire/movement-disagreements)

- **`GCNPCSayDynamic` derived the length byte in front of its message
  from an unbounded string.** `ActionSayDynamic` sets the message to a
  script subject, and 208 of the 1155 shipped subjects are longer than
  255 bytes, so the length byte wrapped (846 becomes 78) and the client
  read a truncated line with the rest of the text left in its buffer. The
  message goes through `de::wire::readString`/`writeString` at the 255
  the byte can describe, on both sides. The seven live sends are
  `CGNPCTalkHandler`'s string-pool alerts, whose longest entry is 120
  bytes, so no live line is refused.
  > **Status:** fixed (wire/movement-disagreements)

- **`ScriptParameter::getSize()` measured its name and value through a
  `BYTE`.** `GCNPCAskVariable` therefore under-reported by 256 for every
  parameter longer than a byte can count, while `write()` emitted both
  strings whole. Both fields go through `de::wire::readString`/
  `writeString` at 255 and the size is `stringWireSize()` of each, so the
  declared size and the emitted bytes cannot drift again. The parameter
  count, a `BYTE` with the same wrap, is the map now, refused past the
  255 the factory max budgets in `addScriptParameter()`, in `write()` and
  by `read()`'s own count byte; a refused record is destroyed rather than
  leaked, as the duplicate-name refusal beside it now is too.
  > **Status:** fixed (wire/movement-disagreements)

- **`GCRemoveEffect` counted its effect list the same way, and
  `popFrontListElement()` left the count alone.** The count is the list
  now, refused past `kMaxCount` (255, what the count byte carries) in
  `addEffectList()`, in `write()` and in `read()`, `setListNum` is gone,
  and popping an id shortens what the packet declares. The factory max
  was a flat 255 bytes, which budgets 125 ids for a list the count byte
  lets reach 255; it is `szObjectID + szBYTE + 255 * szEffectID` now:
  `GCRemoveEffect` 255 -> 515.
  > **Status:** fixed (wire/movement-disagreements)

- **`GCRemoveEffect::read()` appended to the list the packet already
  held.** A packet read into twice declared one sweep's worth of ids and
  wrote two. It clears before it parses, the shape `ModifyInfo::read()`
  and the tile reads already have.
  > **Status:** fixed (wire/movement-disagreements)

- **`GCAddMonsterFromBurrowing` and `GCAddMonsterFromTransformation` did
  not hold the monster name to the 32 bytes their factory max budgets.**
  `GCAddMonster` carries the same record and refuses a longer name; these
  two let a long name outgrow the receiver's read buffer and a name past
  255 wrap the length byte in front of it. Both go through
  `de::wire::readString`/`writeString` at `kMaxNameSize`, the same 32.
  No maximum moves — it already budgeted 32.
  > **Status:** fixed (wire/movement-disagreements)

- **A direction outside the eight travelled unchecked in both
  directions.** `Dir_t` is a `BYTE` typedef, so this is a range gap
  rather than undefined behaviour, and `dir2String()` already prints an
  unknown value as its number. The two client requests refuse it:
  `CGUnburrow::read()` and `CGMove::read()` throw on a direction `>=
  DIR_MAX`. The server's own broadcasts keep it: `MonsterAI::moveNormal`
  computes `DIR_NONE` for a monster already standing on its destination
  and hands it to `Zone::moveCreature`, and `dirMoveMask` has a (0,0)
  entry at that index, so `GCMove`, `GCUnburrowOK` and `GCUntransformOK`
  carrying `DIR_NONE` is part of the contract and is pinned as one.
  > **Status:** fixed (wire/movement-disagreements)

- **Twenty-two of the twenty-six packets left at least one member
  uninitialised in the default constructor.** A packet sent without every
  setter called put whatever the allocation held on the wire. All of them
  initialise every member now — including `PCVampireInfo3`, the record
  the two vampire transition packets embed — pinned by constructing each
  over storage poisoned with two different bytes and requiring the same
  body.
  > **Status:** fixed (wire/movement-disagreements)

- **`~GCNPCInfo` cleared its record list without freeing what `read()`
  allocated into it.** The records a filler hands the packet belong to
  the `Zone` and the ones `read()` parses belong to nobody, so every
  packet read leaked one record per NPC in the zone. The packet tracks
  which it holds and destroys only its own; `read()` clears first, so a
  packet read into twice does not leak the first roster either.
  > **Status:** fixed (wire/movement-disagreements)

- **The four transition packets' `read()` allocated a fresh `EffectInfo`
  over the one the packet already held.** Each destructor frees the
  record, so reading into a packet that had one leaked it. All four free
  the held record before allocating.
  > **Status:** fixed (wire/movement-disagreements)

- **`GCThrowBombOK1`/`2`/`3` and `GCMineExplosionOK1`/`2` carried the
  tile packets' creature list with the tile packets' bugs.** The count
  wrapped at 256, `popCListElement()` left it alone, `setCListNum` let a
  caller declare a count the list did not hold, and every factory max
  budgeted one `ObjectID` for a list the count byte lets reach 255 — so a
  blast that caught more than one creature already wrote a body larger
  than the receiver's read buffer. All five take the `GCSkillToTileOK*`
  fix verbatim, and their maxima budget a full list: `GCThrowBombOK1`
  2055 -> 3071, `GCThrowBombOK2` 2058 -> 3074, `GCThrowBombOK3` 271 ->
  1287, `GCMineExplosionOK1` 2054 -> 3070, `GCMineExplosionOK2` 267 ->
  1283. None was pinned by a golden before; all five have one now.
  > **Status:** fixed (wire/movement-disagreements)

## Combat feedback write/read disagreements (2026-09-09)

The six findings task 1.2 stated as flip-tests in
`tests/packet_combat_test.cpp`, plus the two its review recorded rather
than tested, and three list bounds the social round left open. These
packets are client-facing, so `write()` and `getPacketSize()` are the
contract: every fix is a refusal, a cap at a width the factory max
budgets, a size-accounting correction or a read-side correction to what
`write()` emits. No golden changed; the five tile maxima below are the
set's only `tests/wire-layout.txt` movements.

- **`ModifyInfo` counted each of its two lists in a `BYTE` it incremented
  per entry.** Nothing capped either list, so the 256th entry wrapped the
  count to zero while `write()` still emitted every entry: the receiver
  stopped after the count byte and read the rest of the record as the next
  packet's header, and `getPacketSize()` counted the wrapped count too, so
  `writePacket()` put a length on the wire the body did not match either.
  Both counts are the list now (`ModifyInfo::kMaxCount`, the 255 the count
  byte carries and the maximum budgets), `addShortData`/`addLongData`
  refuse the entry past it and so does `write()`, and `popShortData` /
  `popLongData` no longer underflow a count of their own. Of the 485 fill
  sites in `src/server`, none adds in a loop: each adds one entry per
  changed field under its own `if`, so a record cannot legitimately hold
  more than the 73 `ModifyType` values and no refusal is reachable.
  > **Status:** fixed (wire/combat-disagreements)

- **The five tile packets that carry a creature list counted it the same
  way, and `popCListElement()` left the count alone.** `GCSkillToTileOK1`,
  `OK2`, `OK4`, `OK5` and `OK6` wrapped their `m_CListNum` at 256 exactly
  as `ModifyInfo` did, and a packet read, popped and forwarded declared
  more ids than it held. Each derives the count from the list now,
  `setCListNum` is gone, and `addCListElement` and `write()` refuse the id
  past `kMaxCount`. The bound is 255: the count travels in a byte, so
  nothing wider can be expressed, and no narrower bound is defensible —
  the widest mask a tile skill uses is `IceWave`'s 193 tiles, and a tile
  holds one creature per move mode, so a sweep can in principle reach 579.
  A crowded sweep therefore refuses the packet where it used to wrap the
  count; that is loud where the old behaviour desynchronised the stream.
  > **Status:** fixed (wire/combat-disagreements)

- **No tile packet's factory max budgeted its creature list.**
  `GCSkillToTileOK1`, `OK2` and `OK6` budgeted one `ObjectID` and `OK4`
  and `OK5` one plus a spare 255 bytes, for a list the count byte lets
  reach 255, so a sweep that caught more than one creature wrote a body
  larger than the read buffer the receiver sizes from that maximum. Each
  max multiplies `szObjectID` by its packet's `kMaxCount`, which moves five
  lines in `tests/wire-layout.txt`: `GCSkillToTileOK1` 2060 -> 3076,
  `GCSkillToTileOK2` 2061 -> 3077, `GCSkillToTileOK4` 270 -> 1286,
  `GCSkillToTileOK5` 274 -> 1290 and `GCSkillToTileOK6` 2059 -> 3075. This
  is the buffer the server budgets for a body it receives; the client sizes
  its own from its own copy of the max, and no byte any packet emits
  changes.
  > **Status:** fixed (wire/combat-disagreements)

- **`ModifyInfo::read()` and the tile `read()`s appended to the list the
  packet already held.** A reader is reused, so a packet read into twice
  declared one record's worth of entries and wrote two. All six clear
  before they parse, the shape `GMServerInfo::read` already has.
  > **Status:** fixed (wire/combat-disagreements)

- **The `ModifyType` tag travelled unchecked while `toString()` indexed
  `ModifyType2String` with it.** A tag past the last enumerator round
  tripped intact and the debug string then read past the end of a
  74-element table. `read()` checks the raw byte before it keeps the
  entry, the adders refuse the same value, and `modifyType2String()`
  prints an unknown tag as its number, the way `dir2String()` does.
  > **Status:** fixed (wire/combat-disagreements)

- **`GCSkillToTileOK3::getObjectID()` and
  `GCSkillToInventoryOK2::getObjectID()` were declared `CEffectID_t` over
  an `ObjectID_t` member.** Every caller saw the low half of the id the
  packet puts on the wire, so two creatures whose ids differ only above
  bit 16 were one creature to it. Both return `ObjectID_t`; no server
  source calls either accessor.
  > **Status:** fixed (wire/combat-disagreements)

- **Thirty-one of the thirty-three packets left a member uninitialised in
  the default constructor.** Only `GCStatusCurrentHP` and
  `GCModifyInformation` initialised everything they write, so a packet
  sent without every setter called put whatever the allocation held on the
  wire. All of them initialise every member now, pinned by constructing
  each over storage poisoned with two different bytes and requiring the
  same body.
  > **Status:** fixed (wire/combat-disagreements)

- **`GCAttackArmsOK1` and `GCAttackArmsOK5` read the hit flag into a
  `bool`.** Any byte other than 0 or 1 left an object no load may touch.
  Both read a `BYTE` and narrow it, so a non-zero byte is a hit. `write()`
  is untouched and the wire does not move.
  > **Status:** fixed (wire/combat-disagreements)

- **Three guild lists were capped nowhere.** `GCActiveGuildList` and
  `GCWaitGuildList` emitted every guild they held against a factory max
  that budgets 5000 (the count is a `WORD`, so nothing wrapped, but a
  larger table outgrew the read buffer), and `GCShowWaitGuildInfo`'s
  founding-member list did the same against its 5. All three refuse in
  `addGuildInfo`/`addMember`, in `write()` and in `read()`, the way
  `GCGuildMemberList` does, and destroy a refused owned record. Neither
  refusal is reachable: `GuildManager::makeActiveGuildList` /
  `makeWaitGuildList` walk the guild table, which would need 5001 guilds
  of one race in one state, and a guild leaves `GUILD_STATE_WAIT` for
  `GUILD_STATE_ACTIVE` the moment it has five members
  (`GSAddGuildMemberHandler`), so `CGSelectGuildHandler` can never hand
  `GCShowWaitGuildInfo` a sixth name.
  > **Status:** fixed (wire/combat-disagreements)

- **The last raw direction and sex indexers.** `Dir2String[m_Dir]` in
  `CGMove`, `CGUnburrow`, `GCMove`, `GCMoveOK`, `GCUnburrowOK`,
  `GCUntransformOK`, `PCSlayerInfo3`, `PCOustersInfo3`, `MonsterCorpse`,
  `ActionSetPosition`, `EffectBloodySnake` and `VisionInfo`, and
  `Sex2String[m_Sex]` in `PCSlayerInfo3`, `PCVampireInfo3` and
  `PCOustersInfo3`, indexed an eight- and a two-element table with a value
  that arrives off the wire. All go through `dir2String()` and a new
  `sex2String()` beside it, which print the number instead. Debug strings
  only.
  > **Status:** fixed (wire/combat-disagreements)

## Social protocol write/read disagreements (2026-09-09)

The thirteen findings task 1.2 stated as flip-tests in
`tests/packet_party_test.cpp`, `tests/packet_guild_test.cpp` and
`tests/packet_trade_test.cpp`, plus the two readers its review reported
could not be round-tripped. These packets are client-facing, so `write()`
and `getPacketSize()` are the contract: every fix is a refusal, a cap at
a width the factory max already budgets, a size-accounting correction or
a read-side correction to what `write()` emits. No golden changed; the
guild roster and union offer maxima below are the set's only
`tests/wire-layout.txt` movements.

- **`GuildInfo::getSize()` omitted the expiry date.** `write()` emits a
  length byte and then the date; `getSize()` counted neither, so
  `GCActiveGuildList` declared a body a byte plus the date shorter than
  the one it sends, for every guild in the table, and `writePacket()`
  put that short length on the wire ahead of the longer body.
  `getMaxSize()` already budgets `szBYTE + 11` for the date, so only the
  size was wrong and no maximum moves.
  > **Status:** fixed (wire/social-disagreements)

- **Four client-facing lists arrived reversed.**
  `GCActiveGuildList::read()`, `GCGuildMemberList::read()`,
  `GCWaitGuildList::read()` and `GCShowWaitGuildInfo::read()` pushed each
  record they parsed to the front of a list their `write()` emits front
  to back. All four push to the back now. The two guild tables are drawn
  as a list the player picks from, the roster is drawn the same way, and
  `GCShowWaitGuildInfo`'s founding members are a name list — order is
  what the player sees. Read side only; the wire does not move.
  > **Status:** fixed (wire/social-disagreements)

- **The guild-table count was narrowed to a `BYTE` by its accessor.**
  `GCActiveGuildList::getListNum()` and `GCWaitGuildList::getListNum()`
  returned a `BYTE` of a list whose count goes on the wire as a `WORD`
  and whose factory max budgets 5000 guilds, so a caller asking how many
  guilds the packet holds was told zero at 256. Both return the `WORD`.
  > **Status:** fixed (wire/social-disagreements)

- **Four lists were capped nowhere.** `GCGuildMemberList`,
  `GCUnionOfferList`, `GCPartyJoined` and `GCTradeAddItem`'s option and
  sub-item lists each emitted every element they held against a factory
  max that budgets a fixed number, so the body outgrew the read buffer
  the receiver sizes from that max. Each refuses the entry past the
  budget in `addListElement`, in `write()` and in `read()`, and destroys
  a refused owned record, the way `LCWorldList` does with
  `WorldInfo::kMaxCount`. Two of the maxima were wrong as well:
  `GuildMemberInfo::getMaxSize()` was a whole 220-member table that
  counted one `ServerID` for all of them and left out the roster's own
  count byte, and `GCUnionOfferList`'s max was exactly twenty offers with
  no room for the count byte in front of them. The record maximum is one
  record now and each packet multiplies by its own `kMaxCount`, which
  moves two lines in `tests/wire-layout.txt`: `GCGuildMemberList`
  5064 → 5502 and `GCUnionOfferList` 1180 → 1181. Both are read buffers
  the server budgets for a body it receives, not fields on the wire. Of
  the fill sites, two can reach a refusal: `Guild::makeMemberInfo()`
  walks the guild's member map and nothing limits a guild to 220 members,
  and `GuildUnionOfferManager::makeOfferList()` reads its offers from the
  database, so a union with more than twenty outstanding offers throws
  out of the list request. The party roster cannot —
  `PARTY_MAX_SIZE` is the same six — and the belt and armsband pocket
  counts in the item data stop at the eight sub-items budgeted.
  > **Status:** fixed (wire/social-disagreements)

- **`GCTradeAddItem`'s sub-item count was set independently of the
  records written.** `setListNum()` wrote a member that
  `addListElement()` left alone; `write()` put that member on the wire
  and then emitted every record the list held, and `getPacketSize()`
  sized the records from the member. A packet built without a matching
  `setListNum()` declared one length, announced a second count and
  emitted a third number of records, and the receiver read the rest of
  the body as the next packet's header. The count is the list now and
  `setListNum` is gone, the shape `InventoryInfo`, `GearInfo`,
  `ExtraInfo` and `RideMotorcycleInfo` already have. Both callers — the
  belt and the armsband branches of
  `CGTradeAddItemHandler::makeGCTradeAddItemPacket()` — set exactly the
  number of records they had just added, so no count on the wire changes.
  > **Status:** fixed (wire/social-disagreements)

- **The protocol's strings were bounded on neither side.** The guild and
  member introductions in `CGJoinGuild`, `CGModifyGuildIntro`,
  `CGModifyGuildMemberIntro`, `CGRegistGuild`, `GCShowGuildInfo`,
  `GCShowGuildMemberInfo` and `GCShowWaitGuildInfo` each derived a `BYTE`
  length from the string and emitted the string whole; the guards that
  looked like a cap compared a `BYTE` against 255, which no `BYTE` can
  exceed. They are cut to `GUILD_INTRO_MAX_LENGTH` in the setter and
  refused past it in `write()` — the same width and the same shape as
  the inter-server introductions, since every one of these budgets 255
  or 256 for it. The party protocol's names and chat messages did the
  same against budgets of 10, 20 and 128: the names are refused past
  their width, and when empty where the packet needs one, while the
  messages are cut to 128 in the setter and refused when empty, the way
  `GCGuildChat` already refuses its own. `GCGuildChat`'s sending guild
  name is refused empty or past `GUILD_NAME_MAX_LENGTH`, next to the
  sender and the message it already bounded. A valid value is untouched,
  so the wire does not move.
  > **Status:** fixed (wire/social-disagreements)

- **Two result-code accessors halved a `WORD`.**
  `GCGuildResponse::getCode()` and `GCNPCResponse::getCode()` returned a
  `BYTE` of a member and a wire field that are both `WORD`, so a code
  past 255 reached the wire whole and the accessor whole halved.
  `GCNPCResponse` already declares 139 dialogue codes. Both return the
  `WORD`; no server source calls either accessor, so nothing else moves.
  > **Status:** fixed (wire/social-disagreements)

- **Two readers tested an uninitialised length byte.**
  `GCModifyGuildMemberInfo::read()` and `GCOtherGuildName::read()`
  declared a `BYTE` for the guild-name length, tested it against 30 and
  against 0, and only then read the length the stream carries. So the
  read consumed a length that was never checked when the indeterminate
  value happened to be non-zero, and read no length byte at all when it
  happened to be zero, leaving that byte in the stream and taking the
  rank from it. Both read the length first and check it after, so both
  packets round trip.
  > **Status:** fixed (wire/social-disagreements)

- **Five more packets read a leading flag byte `write()` does not
  emit.** `GCCreatureDied`, `GCDropItemToZone`, `GCMove`, `GCNPCSay` and
  `GCSkillFailed2` each carry a commented-out `oStream.write((BYTE)48)`
  and a `read()` that consumes the byte it would have produced — the
  shape `GCAddEffect` and `GCAddMonsterFromBurrowing` had. The reader
  took the first byte of the object id as a flag and shifted every field
  after it. All five start at the first field `write()` emits, and the
  commented-out writes are gone. `GCDropItemToZone` round trips through
  `tests/packet_encrypter_test.cpp` at every encrypt code now instead of
  being pinned write-side only; its goldens are unchanged.
  > **Status:** fixed (wire/social-disagreements)

- **`toString()` indexed `Dir2String[8]` unchecked.** `GCAddMonster`,
  `GCAddMonsterCorpse`, `GCAddMonsterFromBurrowing`,
  `GCAddMonsterFromTransformation` and `PCVampireInfo3` print a
  direction that arrives off the wire, so a value outside the eight read
  past the end of the array. All five go through `dir2String()`, which
  prints the number instead. Debug strings only; nothing on the wire
  moves.
  > **Status:** fixed (wire/social-disagreements)

## Inter-server link write/read disagreements (2026-09-09)

The findings task 1.2 stated as flip-tests in
`tests/packet_interserver_test.cpp`, plus the one its review reported
that no test stated usefully, and the leftovers earlier rounds reported
in the packets they did not cover. Each is now pinned as the behaviour
it produces. No golden changed; the `GuildInfo2` maximum below is the
set's only `tests/wire-layout.txt` movement.

- **Two chat packets guarded the message length with the sender
  length.** `GGGuildChat::read()` and `GGServerChat::read()` both test
  `szSender > 128` where they mean the message. The sender is capped at
  10 two lines above, so the guard can never fire and a declared message
  length of up to 255 was accepted — while `write()` refuses anything
  past 128 and the receiver sizes its buffer from a factory max that
  budgets 128. Both guards name the message now.
  > **Status:** fixed (wire/interserver-disagreements)

- **The guild introduction was bounded nowhere.** `GSAddGuild`,
  `GSModifyGuildIntro`, `SGAddGuildOK` and `SGModifyGuildIntroOK`, and
  the `GuildInfo2` record nested in `SGGuildInfo`, each derived a `BYTE`
  length from the string and then emitted the string whole. Past 255
  characters the length byte wrapped and the reader stopped mid-text;
  below that the body simply outgrew the read buffer the receiver sizes
  from the factory max. The guards that looked like caps
  (`szGuildIntro > 255` in the two `ModifyGuildIntro` packets,
  `szIntro > 256` in `GuildInfo2`) compared a `BYTE` against a value no
  `BYTE` can hold. `GUILD_INTRO_MAX_LENGTH` (255, the width the length
  byte can express and the smallest of the five budgets) is now cut to
  in every setter and refused past in every `write()`; the read side
  needs no guard, because the one-byte length is the bound. The
  introduction is player-typed and arrives inside the same one-byte
  frame in `CGRegistGuild` and `CGModifyGuildIntro`, so the game server
  never has to refuse a value the client already limited, and the shared
  server's `Guild.Intro` column — a MySQL `text` — is cut on the way
  into the record.
  > **Status:** fixed (wire/interserver-disagreements)

- **`SGGuildInfo` and `GuildInfo2` read their lists back reversed.**
  Both `read()`s push each record they parse to the front of a list
  their `write()` emits front to back, so the guild table and every
  guild's member list arrived in the opposite order. Both push to the
  back now. Nothing depended on the reversal: `SGGuildInfoHandler`
  inserts each guild into `GuildManager`'s id-keyed map and each member
  into `Guild`'s name-keyed map, and the shared server fills the packet
  by walking its own hash map, whose order is arbitrary to begin with.
  > **Status:** fixed (wire/interserver-disagreements)

- **`SGGuildInfo`'s entry count was capped on neither side.**
  `SGGuildInfoFactory::kMaxSize` is 500 guilds' worth of `GuildInfo2`
  and that number is the receiver's read buffer, but neither `write()`
  nor `read()` held the `WORD` count to it, so a table of 501 full-sized
  guilds was emitted and parsed rather than refused. `addGuildInfo()`
  now refuses the entry past `GuildInfo2::kMaxCount` and destroys it,
  the way `LCWorldList::addListElement` does with `WorldInfo::kMaxCount`;
  `write()` refuses a list that reached that length another way, and
  `read()` refuses a declared count past it before allocating a record.
  The shared server fills the packet in
  `GuildManager::makeSGGuildInfo()`, under its own mutex, from
  `GSRequestGuildInfoHandler`: a guild table past 500 makes
  `addGuildInfo()` throw, the scoped critical section releases the
  mutex, and `GameServerManager::processCommands()` catches the
  `ProtocolException` and drops that one game server's connection. The
  shared server stays up; the game server reconnects and asks again.
  > **Status:** fixed (wire/interserver-disagreements)

- **`GuildInfo2::getMaxSize()` counted the member-count word twice.** It
  added `szWORD` once for the count `write()` emits and once more at the
  end, so every guild in `SGGuildInfo`'s budget was two bytes too large
  and the packet's factory max was 1000 bytes above what 500 full guilds
  can occupy. It over-budgeted, so nothing truncated, but the number was
  not the record's size. One line moves in `tests/wire-layout.txt`:
  `SGGuildInfo` 2916502 → 2915502. This is the read buffer the game
  server budgets for a body it receives, not a field on the wire.
  > **Status:** fixed (wire/interserver-disagreements)

- **`GMServerInfo::read()` appended to the zone table it already
  held.** It read the count into the member that `getPacketSize()` and
  `write()` use and then pushed the new rows onto the existing list, so
  a packet read into twice reported fewer zones than it carried. The
  body carries the whole table, so `read()` clears the list first. The
  receive path builds a fresh packet each time, so nothing reached that
  state.
  > **Status:** fixed (wire/interserver-disagreements)

- **Four transition packets dereferenced a missing effect record.**
  `GCAddMonsterFromBurrowing`, `GCAddMonsterFromTransformation`,
  `GCAddVampireFromBurrowing` and `GCAddVampireFromTransformation` read
  `m_pEffectInfo` unconditionally in `getPacketSize()`, `write()` and
  `toString()`, so a default-constructed instance — the one every
  factory creates for a reader — crashed before it could read anything.
  All four fall back to an empty list, as `GCAddSlayer`, `GCAddVampire`,
  `GCAddOusters` and `GCAddMonster` already do.
  > **Status:** fixed (wire/interserver-disagreements)

- **`GCAddMonsterFromBurrowing::read()` consumed a byte `write()` never
  emits.** The same shape `GCAddEffect` had: the
  `oStream.write((BYTE)48)` that would produce the leading flag is
  commented out and `getPacketSize()` never counted it, so the reader
  took the first byte of the object id as a flag and shifted every field
  after it. `read()` starts at the object id now and the commented-out
  write is gone. No byte moves: `write()` never emitted the flag. Five
  more packets carry the same commented-out write and the same leading
  read — `GCCreatureDied`, `GCDropItemToZone`, `GCMove`, `GCNPCSay` and
  `GCSkillFailed2` — and are left alone here; `GCDropItemToZone` is
  already an open finding under task 1.2.
  > **Status:** fixed (wire/interserver-disagreements)

- **Four more records swallowed the exceptions their bodies raised.**
  `RideMotorcycleSlotInfo`, `SubOustersSkillInfo`, `SubSlayerSkillInfo`
  and `SubVampireSkillInfo` each wrapped the whole of `read()` and
  `write()` in `try { … } catch (Throwable& t) { cout … }`, the shape
  `PCSlayerInfo`, `PCSlayerInfo2`, `PCSlayerInfo3` and `SubItemInfo`
  had. A stream that stops short leaves the record half-parsed and the
  caller reading its next field from the wrong offset, so the failure
  has to reach it. The exceptions now leave all eight functions, and no
  `read()` or `write()` in `src/Core` catches `Throwable` any more.
  > **Status:** fixed (wire/interserver-disagreements)

## Zone population scan write/read disagreements (2026-09-09)

The findings task 1.2 stated as flip-tests in
`tests/packet_zone_scan_test.cpp`, plus the ones its review reported
that no test could state while they were open. Each is now pinned as
the behaviour it produces. No valid packet's bytes moved and no golden
changed; the alignment field below is the set's only
`tests/wire-layout.txt` movement.

- **`PCSlayerInfo3::read`/`write` swallowed the exceptions they
  raised.** Both wrapped their whole body in
  `try { … } catch (Throwable& t) { cout … }`, the third copy of the
  shape `PCSlayerInfo` and `PCSlayerInfo2` had. `write()` refuses an
  empty name and a name past 20; the object id was already on the wire
  when the throw happened, so the PC record stopped after four bytes
  while `GCAddSlayer::getPacketSize()` — which `writePacket()` puts on
  the wire ahead of the body — still counted the whole record, and the
  client misframed every packet after it. `PCVampireInfo3` and
  `PCOustersInfo3` never had the wrapper. The exceptions now leave both
  functions.
  > **Status:** fixed (wire/zone-scan-disagreements)

- **`GCAddEffect::read()` consumed a byte `write()` never emits.** The
  `oStream.write((BYTE)48)` that produced it is commented out and
  `getPacketSize()` never counted it, so the reader took the first byte
  of the object id as a flag and shifted every field after it. `read()`
  now starts at the object id, and the commented-out write is gone.
  > **Status:** fixed (wire/zone-scan-disagreements)

- **Two record maxima omitted the alignment field their size counts.**
  `PCVampireInfo3::getMaxSize()` and `PCOustersInfo3::getMaxSize()` left
  out the `szAlignment` that `getSize()` puts on the wire. `Alignment_t`
  is an int, so every carrier of either record budgeted four bytes too
  few, and a Vampire or Ousters name of 17 characters or more already
  declared a body larger than the read buffer the receiver sizes from
  the factory max. Seven factory maxima grow by four bytes in
  `tests/wire-layout.txt` (`GCAddOusters` 1228 → 1232,
  `GCAddOustersCorpse` 54 → 58, `GCAddVampire` 1230 → 1234,
  `GCAddVampireCorpse` 55 → 59, `GCAddVampireFromBurrowing`
  1075 → 1079, `GCAddVampireFromTransformation` 1075 → 1079,
  `GCMorphVampire2` 54 → 58). This is the read buffer the server
  budgets; the client sizes its own from its own copy of the max, and no
  byte any packet emits changes.
  > **Status:** fixed (wire/zone-scan-disagreements)

- **`PCVampireInfo3` narrowed its coat type to a byte in silence.** The
  member is an `ItemType_t`, a WORD, while `write()` cast it to a BYTE
  and `getSize()` budgeted one byte for it, so a value above 255 lost
  its high half and dressed the character in a different coat. The wire
  byte is the client's contract and stays a byte; the domain fits it
  (`VampireCoatInfo.ItemType` is a `tinyint unsigned`, and the gameserver
  only ever assigns `pItem->getItemType()` of a worn `VampireCoat`), so
  `write()` refuses a wider value instead of truncating it.
  > **Status:** fixed (wire/zone-scan-disagreements)

- **Five strings were bounded on neither side.** `GCAddMonster` and
  `GCAddMonsterCorpse` derived a BYTE length from the monster name and
  then wrote the whole string, against factory maxima that budget 32 and
  128 characters; `StoreOutlook`'s shop sign, `PetInfo`'s pet nickname
  and `GCAddVampirePortal`'s owner name did the same against
  `MAX_SIGN_SIZE`, 22 and 20; `NPCInfo::setName` took any length against
  the 30 its own max budgets. Past 255 the length byte wrapped and the
  reader stopped mid-string; below that the body simply outgrew the read
  buffer the receiver sizes from the max. All five are refused past the
  cap in `write()` and `read()`. The two a player types — the shop sign
  and the pet nickname — are also cut to the cap in their setter, the
  way the character nickname is; the monster and NPC names come from
  game data and the portal owner is a character name, so those three are
  refused rather than quietly shortened. A valid value is untouched.
  > **Status:** fixed (wire/zone-scan-disagreements)

- **`GCNPCInfo` wrote a list its count byte could not describe.**
  `write()` puts the record count in a BYTE while `getPacketSize()`
  counted the whole list and the factory max budgets 255 records — the
  shape `GCUpdateInfo::addNPCInfo` already refuses. `addNPCInfo` now
  refuses the record past the count byte; the zone owns the records
  either way, so a refused one is simply not listed.
  > **Status:** fixed (wire/zone-scan-disagreements)

- **The corpse packets left the treasure count indeterminate.**
  `GCAddSlayerCorpse`, `GCAddVampireCorpse` and `GCAddOustersCorpse`
  zeroed it in the default constructor and not in the info-taking one,
  so a corpse built from a PC record put whatever the allocation held on
  the wire. All three initialise it.
  > **Status:** fixed (wire/zone-scan-disagreements)

- **`GCAddOusters` freed records the creature owns.** Its destructor
  deleted the effect, the pet and the nickname records, while
  `GCAddSlayer` and `GCAddVampire` deleted only the effect record.
  `makeGCAddOusters` installs `pOusters->getPetInfo()` and
  `pOusters->getNickname()`, which are the creature's own members, so
  every Ousters that entered someone's view left the creature holding
  two dangling pointers. Only the effect record is packet-owned — both
  `read()` and `PacketUtil` hand over a freshly allocated one — and all
  three destructors now free that and nothing else.
  > **Status:** fixed (wire/zone-scan-disagreements)

- **A missing effect record crashed the size and the write.**
  `GCAddSlayer`, `GCAddVampire`, `GCAddOusters` and `GCAddMonster`
  dereferenced `m_pEffectInfo` unconditionally in `getPacketSize()`,
  `write()` and `toString()`, so a default-constructed instance — the
  one every factory creates for a reader — crashed before it could read
  anything. All four fall back to an empty list, the way `GCUpdateInfo`
  falls back to an empty nickname and blood bible sign.
  > **Status:** fixed (wire/zone-scan-disagreements)

- **`GCAddSlayer::write()` and `GCAddVampire::write()` shared a mutable
  static.** The null-pet branch wrote a function-local
  `static PetInfo NullPetInfo` from a `const` member function, so every
  zone thread emitting a pet-less character touched the same object.
  Each `write()` now builds its own, as `GCAddOusters::write()` already
  did. The bytes are the empty pet record either way.
  > **Status:** fixed (wire/zone-scan-disagreements)

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
