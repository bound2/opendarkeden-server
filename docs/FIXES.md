# Fix log

Real bugs uncovered by the restructuring work (task 5.3 in
`docs/RESTRUCTURING.md`), recorded instead of fixed silently. Sidecar's
convention: every entry has a `> **Status:**` line updated in the same
commit as the fix.

The Exchange-reconcile defect set (SQL injection, string size/body
desync, `StringStream` stack overflows, unbounded listing counts, …) was
found before this file existed; task 1.4 in `docs/RESTRUCTURING.md` keeps
the rules each one left behind and the items still open, and the fixes
themselves are in the `restructuring/exchange-reconcile` branches of this
repo and the client's. Entries below are newest first; the oldest is the
1.4 max-size reconcile that followed it.

## The player tables hold four times the descriptors an fd_set does (2026-09-24)

- **`PlayerManager`'s table has 2000 slots and the managers admit every
  descriptor below that into their `fd_set` members,** but `fd_set` is the
  platform's own and holds `FD_SETSIZE` descriptors, 1024 on glibc, which
  no build raises. A gameserver or loginserver that reaches a descriptor of
  1024 has `FD_SET` write past `m_ReadFDs`, `m_WriteFDs` and `m_ExceptFDs`
  into whatever follows them in the manager, and calls `select` with an
  `nfds` the interface does not define. The accepted-descriptor bound at
  `IncomingPlayerManager::acceptNewConnection` and the one in
  `PlayerManager::addPlayer` both say 2000, so nothing refuses the
  descriptors in between. The sharedserver is not affected: its table is
  100 slots and it refuses anything above them. Closing it means either
  capping the tables at `FD_SETSIZE` -- which caps the concurrent player
  count with them -- or leaving `select` for an interface with no such
  limit, so it is a decision rather than an edit.
  > **Status:** recorded, not fixed (fix/config-and-listener)

## The client heartbeat is verified twice for every heartbeat packet (2026-09-24)

- **`GamePlayer::processCommand` ran `verifySpeed(pPacket)` and threw the
  answer away, and `CGVerifyTimeHandler` then ran it again on the same
  packet,** so the heartbeat state machine advanced twice per `CGVerifyTime`.
  The first run set the next expected time to *this* arrival plus the
  60-second interval, which makes the second run's test (`now > next - 5`)
  false for every heartbeat, so the run that actually decides always took the
  early branch: an on-time heartbeat decremented and incremented the error
  count in the same breath, and an early one counted twice. The run of early
  heartbeats needed to disconnect a player was two rather than the five the
  rule is written for. The discarded call arrived with the recovered sources
  (`3fccc6c6`) as part of the abandoned move/attack check below and is gone;
  the handler is the only caller again, as it was at import, and
  `tests/speed_hack_decision_test.cpp` pins the counting.
  > **Status:** fixed (fix/shrine-and-speed)

## A deleted guild's pending war comes straight back (2026-09-23)

- **`GuildManager::deleteGuild` reloads the castle zone's war scheduler
  when the guild has a war scheduled, but reloads it from the same table
  rows, where the war still waits,** so the same war returns. Only
  `cancelGuildWarSchedules` cancels rows, and this path never reaches it.
  The reload now runs on the owning zone thread; it is still a no-op for
  its purpose.
  > **Status:** recorded, not fixed (fix/recorded-defects-6)

## The sharedserver's descriptor walk starts from an unchecked listener (2026-09-23)

- **`GameServerManager` seeds its descriptor range from the listening
  socket's descriptor with no bound,** so a listener at or above the
  hundred-slot table makes every input and output walk read past it. The
  bound added for accepted connections covers player adds only. The
  gameserver's `IncomingPlayerManager` and the loginserver's
  `LoginPlayerManager` seed the same way over their 2000-slot player
  tables. All three now refuse at startup a listening socket their table
  cannot hold: clamping the walks alone would hide the listener from every
  one of them, and a manager that never sees its own listening socket
  accepts nothing. Beyond that seed, each sweep and each min/max rescan
  runs the range through `de::descriptorRange`
  (`src/server/DescriptorTable.h`), which clamps it to the table it
  indexes, so no walk can index past the table however the range was
  reached; `GameServerManager::deleteGameServerPlayer` refuses a
  descriptor outside the table the way its add already did.
  `tests/descriptor_table_test.cpp` pins the clamp, the listener case
  included.
  > **Status:** fixed (fix/config-and-listener)

## The account database connection takes the game database's port (2026-09-23)

- **`DatabaseManager::init` builds the USERINFO connection from the
  `UI_DB_*` host, database, user and password keys but reads its port from
  `DB_PORT`,** the game database's key, where the two other places that
  open a connection to that server read `UI_DB_PORT`. An account database
  on a port of its own is reached on the wrong one. Both connections are
  read as whole blocks now, by `de::connectionSettings`
  (`src/server/database/ConnectionSettings.h`), which takes the prefix and
  the five keys behind it, so one block's address cannot be built from
  another's; a block that names no port still means the driver's default,
  which is what the other two readers already did. The login and shared
  server configurations repeated `DB_PORT` inside their `UI_DB` and
  `DIST_DB` blocks, which is where the confusion came from; each port key
  is named after the block it sits in now, at the same value, so those
  deployments reach the same ports as before.
  `tests/connection_settings_test.cpp` covers the block reader; the rest of
  `DatabaseManager::init` opens sockets to MySQL and runs its first query,
  so it has no seam a unit test can hold.
  > **Status:** fixed (fix/config-and-listener)

## The Netmarble flag is read with opposite senses (2026-09-23)

- **`GamePlayer::logLoginoutDateTime` selects the Netmarble dimension when
  `IsNetMarble` is zero, against its own comment,** while the other six
  readers of the flag treat nonzero as Netmarble. Every shipped
  configuration sets the flag to zero, so every login and logout row in the
  web log was written under dimension 2 in place of the configured one. The
  site takes the majority sense now, and all seven readers ask
  `de::isNetMarbleDeployment()` (`src/server/Deployment.h`), so the sense
  has one place to be wrong in. Nothing on the wire depends on it: the
  client decides its own Netmarble build from `Netmarble != 0` in its
  Netmarble.inf, the same sense, and a Netmarble client never parses a
  dimension out of its command line at all -- `g_Dimension` keeps the zero
  it was initialised with, and it only selects which login-server address
  list the client reads. The dimension the server writes is its own
  `Dimension` key, which every other reader of that key passes straight
  through.
  `ZoneLoad.cpp`'s pay-zone message was recorded here as a second reversed
  site and is not one: its zero branch sends STRID_CANNOT_ENTER_PAY_ZONE,
  which is what the file's other pay-zone portal sends unconditionally, so
  zero already meant "not Netmarble" there. Its comment says so now.
  > **Status:** fixed (fix/config-and-listener)

## An ousters saved an indeterminate silver damage on destruction (2026-09-23)

- **`Ousters::Ousters()` never initialized `m_SilverDamage`, and
  `~Ousters()` passes it to `saveExps`,** which writes it to the
  character's `SilverDamage` column. Only `load()` ever set the field, so
  a character constructed and destroyed without a successful load wrote
  whatever the allocation happened to hold. `Vampire::Vampire()` zeroed
  its own copy. The field now lives once on `PlayerCreature` with a zero
  initializer, which closes it for all three races.
  > **Status:** fixed (refactor/race-load)

## A guild's deletion walks and rebuilds the war schedules from another thread (2026-09-23)

- **`WarScheduler::hasSchedule` walks the recent schedules with no lock,
  unlike every sibling, and `GuildManager::deleteGuild` calls it from the
  shared-server link thread,** then on a hit calls `load()`, which clears
  and rebuilds the schedules, deleting war objects the zone thread may be
  about to execute under its own lock. Deleting a guild whose castle has a
  pending war frees a war under the zone thread's scheduler heartbeat. The
  fix is to post the cancel to the owning zone group rather than reach
  across. `hasSchedule` takes the mutex now, and the reload is posted to
  the zone's group with `ZoneGroup::post()`, so it runs on the thread that
  executes those wars, under the group mutex; the command captures the
  zone and looks the scheduler up again, because a zone reload replaces
  it. Still open: the GM `reloadinfo` of a war schedule
  (`EventReloadInfo`) calls `load()` from the main thread the same way.
  > **Status:** fixed (fix/recorded-defects-6)

## The sharedserver's player table is indexed by raw descriptor with no bound (2026-09-23)

- **`GameServerManager::addGameServerPlayer` writes `m_pGameServerPlayers[fd]`
  into a table of a hundred slots with no check against its size,** and the
  input and output loops walk the table from the lowest to the highest
  descriptor seen. A game-server link whose socket descriptor is a hundred
  or more overruns the array. The add bound-checks the descriptor now, the
  way the gameserver's `PlayerManager` does, and the accept path logs the
  refused host and deletes the player, which closes the socket. The
  loginserver's `GameServerManager` owns a datagram socket and no table,
  and both `ClientManager`s reach their players through `PlayerManager`,
  whose add, delete and get all bound-check.
  > **Status:** fixed (fix/recorded-defects-6)

## A duplicate game-server connection would be freed twice (2026-09-23)

- **`GameServerManager::acceptNewConnection` frees the accepted socket and
  then the player built on it when the add reports a duplicate,** but the
  player's destructor closes and deletes that socket itself. Latent: the
  add never reports a duplicate today, having no duplicate check. The
  branch for a missing element also builds a message with the offending
  host and port that nothing prints. The socket now has one owner: the
  duplicate branch deletes only the player, the outer handler deletes
  whichever of the two exists, and the missing-element message goes to the
  log the file already writes.
  > **Status:** fixed (fix/recorded-defects-6)

## Registering a siege beside a scheduled guild war schedules a second war (2026-09-23)

- **`ActionRegisterSiege` looks up the castle's recent schedule, casts its
  war to `SiegeWar` and, when the cast fails, creates a new siege schedule
  instead of joining,** so a castle whose recent schedule holds a guild
  war gets two wars scheduled. The two registration actions are
  alternatives in quest data; it bites a server whose scripts use both.
  > **Status:** recorded, not fixed (fix/recorded-defects-5)

## A guild war reloads after a restart as a siege with no challengers (2026-09-23)

- **`WarScheduler::load()` rebuilds every `WAR_GUILD` schedule row as a
  `SiegeWar`,** and a guild war's row carries one attacker guild and no
  challenger count, so it comes back as a siege nobody attacks. Telling the
  two apart on reload needs a column `WarScheduleInfo` does not have.
  > **Status:** recorded, not fixed (fix/recorded-defects-5)

## A vampire's silver damage cleared by a GM is never saved (2026-09-23)

- **The exps save skips the `SilverDamage` column for a vampire when the
  value is zero, so it cannot reset the column,** and the one clear that
  does not go through `saveSilverDamage`, whose `tinysave` writes the
  column unconditionally, is the GM heal command's `setSilverDamage(0)`:
  a vampire a GM healed keeps the stale value across logout and reloads
  with it, while every in-game cure persists its result at once. The
  ousters row writes the column unconditionally. The repository header
  documents the skip and the integration tier pins it as it stands. The GM
  heal calls `saveSilverDamage(0)` now, for both races, so the clear is
  written the moment it is made, like every cure's.
  > **Status:** fixed (fix/recorded-defects-6)

## A reinforcement condition null-checked the wrong pointer (2026-09-23)

- **`ConditionExistReinforce` cast the castle's next scheduled war to
  `SiegeWar`, tested the uncast work pointer for null, and dereferenced
  the cast result,** so a next war that is not a siege, a guild war or a
  race war, crashed the condition. The four sibling actions test the cast
  result; the condition does now too.
  > **Status:** fixed (refactor/game-context-14, stack top)

## A retired guild member is read with its last rank (2026-09-23)

- **A `GuildMember` a thread may still hold is retired, not freed:**
  `Guild::deleteMember` and `retireAllMembers` move the object to
  `m_RetiredMembers` and erase it from the map, and `getMember()` returns
  a raw pointer after releasing the guild mutex, so a zone thread that
  took one before the removal keeps reading it. The object stays valid --
  that is what retirement buys -- but its rank, name and join date are
  frozen at the moment it left the guild, and nothing marks it as gone.
  A member kicked or a guild deleted while a zone thread is mid-tick can
  therefore still pass a rank check on that tick. The same holds for a
  retired `Guild` in `GuildManager::m_RetiredGuilds`. Giving the member a
  retired flag the readers test, or handing out a copy instead of the
  pointer, is the shape a fix would take.
  > **Status:** recorded, not fixed (refactor/exps-record)

## A castle shrine's defender check asserts on a siege (2026-09-23)

- **`CastleShrineInfoManager::isDefenderOfGuardShrine` asks `getActiveWar`
  for the castle's war and then casts the result to `GuildWar` and asserts
  it,** and `getActiveWar` returns either castle war class, so the check is
  right for a guild war and throws for a siege, the defender side a siege
  knows being on `SiegeWar`. Latent: nothing calls the castle manager's
  method; the dissection handler reaches `ShrineInfoManager`'s, which
  consults no war. The castle manager's method and its declaration are
  gone, so the cast that could not answer for a siege is gone with them.
  > **Status:** fixed (fix/recorded-defects-6, deleted as uncalled)

## The string pool is rewritten under readers on reload (2026-09-23)

- **`StringPool::load()` clears and refills its map, the class has no
  mutex, and the GM reload event calls it on the main thread** while the
  zone threads and the two manager threads read `getString` and `c_str`
  from the same map. A `reloadinfo` of the string pool can race every
  reader.
  > **Status:** recorded, not fixed (refactor/game-context-13)

## The variable manager is written by a GM command with no lock (2026-09-23)

- **`VariableManager` has no mutex; the GM `opset` command sets a variable
  on a zone thread** while the incoming-connection log macro reads it on
  the main thread and the login-link handlers read it on their own
  thread.
  > **Status:** recorded, not fixed (refactor/game-context-13)

## A guild war is scheduled but never becomes active (2026-09-22)

- **`WarSystem::addWar` and `WarSystem::heartbeat` make the same cast the
  lookups made: every `WAR_GUILD` war is `dynamic_cast` to `SiegeWar` and
  the result asserted,** and in `addWar` the assert sits after
  `addSchedule`, so a `GuildWar` is put on the recent schedules and the
  throw aborts the rest: it never reaches the active wars, never triggers
  the holy-land refresh, `hasCastleActiveWar`
  stays false for its castle, and at its end the heartbeat asserts once
  more on the same cast. The lookups no longer throw on it; the registration is the rest of
  the defect. `GuildWar` carries a castle zone id and its own owner-change
  and end-war overrides, so matching it by zone the way a siege is matched
  is the shape a fix would take.
  > **Status:** fixed (fix/recorded-defects-5) — `War` now answers for its
  > own castle, attacking guild, registration fee and participants
  > (`getCastleZoneID`, `getAttackerGuildID`, `getRegistrationFee`,
  > `isWarParticipant`; zero and no-one for a war fought over no castle), and
  > every `WAR_GUILD` site that cast to `SiegeWar` to read them asks the war
  > itself instead: `WarSystem::addWar`, `WarSystem::heartbeat`, the two
  > active-war lookups, `WarScheduler::hasSchedule` and
  > `WarSchedule::create`. A siege reads the same values through the virtuals
  > that it read through the cast, so the siege path is unchanged. A guild
  > war now writes its schedule row, reaches the active wars with its castle
  > and attacker, fires the holy-land refresh and the `GCWarList` broadcast,
  > answers `hasCastleActiveWar` and `getActiveWar` for its castle — so
  > `isModifyCastleOwner` and `endWar` dispatch to `GuildWar`'s own
  > overrides — and is erased from the active wars when its hour runs out.
  > `WarSchedule::save()` stays siege-only: its one caller,
  > `ActionRegisterSiege`, holds a `SiegeWar` already.

## Two servers link different classes under one name (2026-09-22)

- **`GameServerInfoManager` exists as two different classes of the same
  name, one in ServerCore and one in the sharedserver, both reachable from
  the sharedserver link.** The archive member is never pulled in, so the
  program holds only the sharedserver's copy today; five member functions
  (the constructor, the destructor, `init`, `load` and `toString`) share a
  mangled name, so pulling it would be a duplicate-symbol link error rather
  than a silent mis-bind. The sharedserver's `g_pGameServerInfoManager`
  definition duplicated ServerCore's until its retirement onto
  `SharedServer`; the twin classes remain. The three
  `GameServerGroupInfoManager` classes live in three different executables
  and are no hazard.

  > **Status:** fixed (refactor/shared-twin-classes) — the sharedserver's
  > copies are renamed `SharedGameServerInfoManager` and
  > `SharedGameServerInfo`, file names included: the element class was a
  > twin too, ServerCore's `GameServerInfo` carrying two members and four
  > accessors the sharedserver's does not, so two definitions of one class
  > name no longer meet in that link. `GameServerGroupInfoManager` is not
  > the same case: ServerCore has no copy of it, the gameserver, the
  > loginserver and the sharedserver each compile their own into their own
  > binary, and three classes that never share a link are not an ODR
  > problem, so they stay as they are. No loginserver class shadows a
  > ServerCore one. The sharedserver's `GameServerManager::heartbeat()` is
  > gone as well: it locked the manager's mutex, did nothing and had no
  > caller, the worker loop calling the guild manager's heartbeat instead.

## A motorcycle that cannot be placed is paid for and never delivered (2026-09-22)

- **`CGShopRequestBuyHandler::executeMotorcycle` charges the player and
  advances the shop version before it asks the zone for a tile, and when
  no tile is free it prints a banner and returns:** the gold is gone, no
  motorcycle or key follows, the probe key it created to test the
  inventory is never freed, and no `GCShopBuyOK` or `GCShopBuyFail` is
  sent, so the client's shop dialog waits for a reply that never comes.
  Every other exit of the function frees the probe and answers. Whether
  to refund and roll the shop version back, or to place the item first,
  is a design decision.
  > **Status:** fixed (fix/recorded-defects-4) — the no-tile branch now
  > refunds the price with `increaseGoldEx`, the exact inverse of the
  > `decreaseGoldEx` that charged it, frees the probe key and answers
  > `GCShopBuyFail` with `GC_SHOP_BUY_FAIL_NOT_ENOUGH_SPACE`, the way the
  > function's earlier exits do. The shop version is deliberately left
  > raised: a raised version only makes the client re-fetch the rack,
  > which still holds the motorcycle.

## A scheduled guild war makes every active-war lookup assert (2026-09-22)

- **`WarSystem::getActiveWar` and `getActiveWarSchedule_LOCKED` walk the
  recent schedules and, for every war whose type is `WAR_GUILD`,
  `dynamic_cast` it to `SiegeWar` and assert the result,** but `GuildWar`
  is a sibling of `SiegeWar` that reports the same type and reaches the
  schedules through `ActionWarRegistration` and the zone heartbeat. While
  a guild war sits there, every lookup for any zone throws
  `AssertionError`, including the castle shrine's owner change and the
  siege manager's checks.
  > **Status:** fixed (fix/recorded-defects-4) — both loops now skip a war
  > whose cast to `SiegeWar` fails instead of asserting it, so a scheduled
  > guild war is passed over and the lookups keep answering for every
  > other zone. Every caller already handles the `NULL` a zone with no
  > siege returns. A `GuildWar` is matched by its own `getCastleZoneID`
  > since fix/recorded-defects-5, so both loops answer for it too.

## A shrine set with no owner names its race from an uninitialised pointer (2026-09-22)

- **`ShrineInfoManager::putBloodBible` chose the race name for its
  broadcast in three branches, Slayer, Vampire and Ousters, and left the
  pointer indeterminate for a shrine set nobody owns,** then handed it to
  `sprintf`. The pointer now starts as an empty string, so an unowned
  shrine set is announced with no race name rather than whatever the stack
  held.
  > **Status:** fixed (refactor/game-context-12, stack top)

## The monster name manager's event names are neither zeroed nor freed (2026-09-22)

- **`MonsterNameManager` initialised its first, middle and last name arrays
  in the constructor and freed them in the destructor, but not the event
  last-name array `init()` also loads:** an `init()` that threw before that
  block left the pointer indeterminate, and every shutdown leaked it. Both
  now match the other three.
  > **Status:** fixed (refactor/game-context-12, stack top)

## The war system's end condition is always met (2026-09-22)

- **`WarSystem::isEndCondition` asserts its item and corpse and returns
  true;** the comparison of the blood bible's monster type against the
  corpse's is a comment above the return. Nothing calls it today, so the
  function is a stub a future caller would trust; the comparison it meant
  names an accessor that does not exist in that spelling.
  > **Status:** fixed (fix/recorded-defects-5) — deleted, uncalled. The
  > function had no caller in any of the three servers and none in the
  > history: it arrives with the original import and is untouched since, so
  > nothing loses a check. A caller that needs the comparison writes it
  > against the blood bible and the corpse it holds.

## The event monster name overload returns before its retry loop can retry (2026-09-22)

- **The event `MonsterNameManager::getRandomName(Monster*, bool)` returns on
  the first pass of its retry loop and spells its fallback as a comparison
  (`Name == "..."`) rather than an assignment,** the loop shape the non-event
  overload had: an empty event-name row comes back as an empty name, the
  trial count is spent on nothing and the fallback after the loop is both
  unreachable and a no-op. Nothing calls this overload today; the one
  `getRandomName` call site takes the non-event one.
  The loop now draws again while the row that came up is empty and the
  fallback is an assignment the 300th empty draw reaches. The name itself
  is unchanged: a non-empty first draw returns that event part alone, as
  before, with nothing appended around it.
  > **Status:** fixed (fix/recorded-defects-3)

## Placing a blood bible on the holy shrine flips its owner without the war check (2026-09-22)

- **`ShrineInfoManager`'s holy-shrine branch tested `isMatchHolyShrine`
  and the war system's `isModifyCastleOwner` together; the second conjunct
  was commented out with the closing parenthesis inside the comment,** so
  the live condition is the match alone, or a guard-shrine defender, and
  its only surviving statement sets the shrine set's owner race. Any
  player who places a matching blood bible there changes the owner with no
  war-eligibility test; the `endWar` and `returnBloodBible` that followed
  are commented out as well. The castle counterpart in
  `CastleShrineInfoManager` still applies the check.
  The war the holy shrines are fought over is the race war, not a castle
  war: `canPickupBloodBible` says the bible is used only in race wars,
  `RaceWar::executeStart` drops every guard-shrine shield and broadcasts the
  bible positions, and `RaceWar::executeEnd` returns them all and tallies the
  shrine owners into `RaceWarHistory`. So the test restored is the race
  war's own -- whoever the holy land is open to while it runs, which is
  everyone when the participant limiter is off and otherwise the players
  carrying the join ticket, the same pair `ConditionEnterHolyLand` and the
  waypoint handler ask -- and it is asked through
  `War::mayModifyShrineOwner`, which `RaceWar` answers and every other war
  refuses, over a `WarSystem::mayModifyShrineOwner` that reads
  `hasActiveRaceWar()` first and answers no when no race war is running
  rather than asserting. The castle's other two statements are deliberately
  not restored: a race war does not end when one shrine changes hands, and
  the bible travels back to its guard shrine either way, which is what the
  unconditional `returnBloodBible` below the branch already did.
  > **Status:** fixed (fix/shrine-and-speed)

## A random mine item is read off a row that may not exist (2026-09-22)

- **`ItemMineInfoManager::getRandomItem` draws an id in the caller's range
  and dereferences `getItemMineInfo(id)` without checking it,** so a gap
  in the `ItemMineInfo` rows faults the caller: the five hunting-pouch
  branches of the use-item handler and the GQuest inventory handler, each
  on a zone thread. The lair trade guards the same lookup one frame up.
  The function now answers null for a missing row; the GQuest handler
  already refuses a null item through `fitToPC`, and the pouch handler
  refuses it too.
  > **Status:** fixed (refactor/game-context-11, stack top)

## A third event action crashes on an Ousters (2026-09-22)

- **`ActionGiveTestServerReward` chooses its Lua item selector in a Slayer
  branch and a Vampire branch and calls `prepare()` on it,** the defect
  `ActionGiveEventItem`, `ActionGiveAccountEventItem` and
  `ActionTradeGiftBox` had before they refused the race instead.
  It refuses the race the same way now: the action reads a `SlayerFilename`
  and a `VampireFilename` and nothing else, its Lua scripts are the slayer
  and vampire pair, so a null selector logs the character to
  `TestServerRewardError.txt`, closes the NPC dialogue and returns. The
  reward flag is left set, so a character that reaches the action again
  with a selector still gets its reward.
  > **Status:** fixed (fix/recorded-defects-3)

## A vision info manager's debug string fell off the end of the function (2026-09-22)

- **`VisionInfoManager::toString()` returned `string` and had no `return`:**
  its whole body was a commented-out block, so the live function was a
  try/catch frame around nothing. Falling off the end of a non-void
  function is undefined behaviour; nothing calls it today. It now returns
  the class name, the manager having no state to print.
  > **Status:** fixed (refactor/game-context-10, stack top)

## The lottery handler used the item it failed to roll (2026-09-22)

- **`CGLotterySelectHandler` walks the treasure list and, when no entry
  rolled an item, still called `setItemGender` on the item pointer and, in
  its else branch, `setUnique`, `setTimeLimitItem` and `setQuestItem` on
  it.** The pointer was uninitialised before the leak fix and null after
  it, so the failure became a deterministic null call. The gender and the
  fallback branch now run only for an item that exists; a player whose
  roll produced nothing gets nothing, as the inventory branch already
  allowed.
  > **Status:** fixed (refactor/game-context-10, stack top)

## A lair-item trade dereferences its mine info and item before checking them (2026-09-22)

- **`ActionTradeLairItem` dereferences `pItemMineInfo->getItem()` unchecked
  and calls `setItemGender` on its item before the null check that
  follows it,** so a treasure whose roll produced nothing reaches
  `setItemGender` with null, the shape the lottery handler had.
  The mine info is now checked before its item is taken and the gender is
  set only for an item that exists, so a band with no mine row and a race
  branch that produced nothing leave the trade item-less and reach the
  no-item handling the other trade types already take, rather than
  dereferencing null. The mine-backed types name no lair master, so an
  item-less one of those ends in the trade's monster lookup rather than
  its `tradeLairItemBUG.txt` refusal; both are data faults and neither
  faults the server now.
  > **Status:** fixed (fix/recorded-defects-2)

## Two more event item actions crash on an Ousters (2026-09-22)

- **`ActionGiveAccountEventItem` and `ActionTradeGiftBox` choose their Lua
  item selector in a Slayer branch and a Vampire branch, so an Ousters
  leaves the pointer null and dereferences it,** the defect
  `ActionGiveEventItem` had before it refused the race instead.
  Neither action has an Ousters selector to reach for: both read only a
  `SlayerFilename` and a `VampireFilename`, and the slayer and vampire
  classes remain the only selectors in the tree. So both now refuse the
  way `ActionGiveEventItem` does -- the character is logged to the
  action's own error file (`AccountEventItemError.txt`,
  `XMasEventError.txt`) and the NPC dialogue closes. The same two-branch
  selector choice is also in `ActionGiveTestServerReward`, untouched.
  > **Status:** fixed (fix/recorded-defects-2)

## The movement and attack speed-hack check is gone (2026-09-22)

- **`GamePlayer::verifySpeed` keeps only the `CG_VERIFY_TIME` heartbeat
  check, and its one caller discards the result.** Its per-race move and
  attack timing check, 298 lines, sat inside a comment block since before the tree was imported and went with
  the commented-out code; what remains for `CG_MOVE` is an empty `if` on
  `m_MoveSpeedVerify` and a round-trip time computed into a local nothing
  reads, and `m_AttackSpeedVerify` is written by nothing that decides on
  it.
  The block was abandoned before the tree was imported, not paused: at the
  import commit `verifySpeed` was reached from one place, the `CGVerifyTime`
  handler, so its `CG_MOVE` and `CG_ATTACK` branches could not run whatever
  the comment markers said; it opens by redeclaring the `SpeedCheck` the live
  body above it already declares, so it does not compile where it sits; its
  race chain ends in an empty `else`, leaving Ousters unchecked; and its
  vampire arm assigns the same constant in all three speed tiers its own
  comments give different numbers for. Reinstating it would mean inventing
  per-race timings and an enforcement action the original never had, so the
  residue was removed instead: the empty `if`, the unread round-trip local,
  `m_MoveSpeedVerify`, `m_AttackSpeedVerify`, the untouched
  `m_SkillSpeedVerify[SKILL_MAX]` array and the `tv_sub` helper that only the
  unread local used. What survives is the heartbeat rule, now a pure
  `de::verifyHeartbeat` (`src/server/gameserver/SpeedHackDecision.cpp`) with
  the session's state beside it and `tests/speed_hack_decision_test.cpp`
  holding its arithmetic; the duplicate call the same residue left behind is
  the 2026-09-24 entry at the top of this file.
  > **Status:** not a defect (the check never ran and could not compile; its
  > residue was removed in fix/shrine-and-speed)

## The log client never opens its socket (2026-09-22)

- **Every `LogClient` member and every global `log(...)` had its whole body
  commented out,** so `openLogClient()` allocates a client that never
  connects, `LogClient::m_LogLevel` is read by nothing, every `log(...)`
  call in the tree is a no-op, and each `main()` still reads
  `LogServerIP`, `LogServerPort` and `LogLevel` from its config for it.
  The module is gone: its 66 call sites, the message locals that only fed
  them, the class, its header, the `LogData` record it framed its messages
  in, the two ServerCore entries that compiled them, the three
  configuration keys no other reader wanted, and the gameserver's startup
  banner line for it. `LogData::read` took a `short` length straight into a
  2,001-byte stack buffer, so anything reinstated must not start from it;
  of the thirty-seven event kinds the header declared, nineteen were ever
  called. What the servers still write
  is the file logs (`filelog` and the `FILELOG_*` macros) and the play
  records the repositories keep; reinstating a log-server link would be a
  new feature, not a repair.
  > **Status:** fixed (refactor/dead-logclient)

## Duplicate Self is not gated on a master lair (2026-09-22)

- **`checkTimingDuplicateSelf` in `MonsterAI.cpp` lacks the master-lair
  guard its siblings `checkMasterSummonTiming` and `checkMasterNotReady`
  both apply;** the guard was there, switched off in a comment block that
  is now deleted. A master outside its lair can duplicate itself.
  The guard is back, spelled as its two siblings spell it: a master whose
  zone is not a master lair fails the condition, so the duplicate-self
  directive is offered only inside a lair.
  > **Status:** fixed (fix/recorded-defects-2)

## A random monster name is only ever its middle part (2026-09-22)

- **The non-event `MonsterNameManager::getRandomName` draws a first and a
  last name index it never uses, returns on the first pass of its retry
  loop, and spells its fallback as a comparison (`Name == "..."`) rather
  than an assignment,** so the fallback is unreachable and a named monster
  gets its middle name followed by a trailing space, which reaches the
  client. The design the deleted comment described was banded by level:
  a last name alone up to level 33, first and last to 66, all three above.
  The event overload has the same loop shape but draws a sensible event
  name.
  The name is now the middle part alone with no trailing space, the loop
  draws again while the row that came up is empty, and the fallback is an
  assignment the 300th empty draw reaches. The banded design is **not**
  restored and stays a design decision: the parts the database ships are up
  to 13, 11 and 18 bytes, so its top band composes up to 44 bytes while
  `GCAddMonster`, `GCAddMonsterFromBurrowing` and
  `GCAddMonsterFromTransformation` all throw `InvalidProtocolException`
  above a 32-byte name, and how to keep a three-part name inside that
  field is not something the deleted comment says. The first and last
  name tables stay loaded and unread, as they already were behind the
  indices that were always -1.
  > **Status:** fixed (fix/recorded-defects-2)

## The item factory's out-of-range guard is caught by its own handler (2026-09-22)

- **`ItemFactoryManager::createItem` checks the item class and throws
  `NoSuchElementException` inside a `try` whose `catch (Throwable&)` only
  prints it,** so control falls through to `m_Factories[IClass]->createItem`
  with the very index or null factory the guard rejected: an out-of-bounds
  read of the factory array for a class past `ITEM_CLASS_MAX`, a null call
  for an in-range class no factory registered. Not reachable from a client
  packet: the callers pass a literal class, a class read off an existing
  item, or one from server-side data, and the GM item command is gated by
  `isPossibleItem`, which asserts the range first. A corrupt database or
  XML row (`ItemMineInfo` reads its class from the database) crashes where
  the guard meant to log and skip. `getItemName` has the weaker shape of the
  same defect: it builds its diagnostic and discards it, then indexes.
  Both guards now leave their function: the swallowing `catch` is gone, so
  `createItem`'s `NoSuchElementException` propagates through `__END_CATCH`
  to the caller, and `getItemName`, which nothing in the tree calls, throws
  the diagnostic it had been building. Propagating rather than answering
  NULL is what the callers support: of the 183 `createItem` call sites
  only 38 check the pointer
  within five lines, so a NULL return would have moved the crash rather
  than removed it. The logging the guard already did is unchanged. Three
  in-range classes never get a factory (`ITEM_CLASS_CORPSE`,
  `ITEM_CLASS_GQUEST_ITEM`, `ITEM_CLASS_BLOOD_BIBLE_SIGN`), so server data
  naming one of them takes the null-factory branch. On a client packet the
  exception ends in a clean disconnect; on the worker threads that catch
  nothing per tick (`GDRLairManager::run`, the monster drop paths of
  `ZoneGroup::heartbeat`) it reaches `ManagedThread` and stops the server,
  where the old fall-through faulted on the same tick.
  > **Status:** fixed (fix/recorded-defects-1)

## A lair-item trade leaks every winning treasure but the last (2026-09-22)

- **`ActionTradeLairItem::execute` walks the monster type's treasure list
  and calls `createItem` for every entry whose `getRandomItem` succeeds,
  assigning each to the same `pItem1`,** so only the last survivor reaches
  `registerObject` and the earlier ones are heap objects nothing frees. One
  entry means no leak; several mean a leak on every trade, a repeatable NPC
  action. `CGLotterySelectHandler` has the identical loop shape.
  Both loops now `SAFE_DELETE` the pointer before overwriting it, so the
  roll the player is given is still the last winning one and the earlier
  ones are freed as they are replaced. The lottery handler's pointer also
  starts at NULL, which it needs to be deleted safely; it was left
  indeterminate. Still open there, and untouched: when no treasure in the
  list rolls an item, the body goes on to sex, store and log an item it
  does not have.
  > **Status:** fixed (fix/recorded-defects-1)

## Yellow Poison restores a fixed sight instead of the one it replaced (2026-09-22)

- **`EffectYellowPoisonToCreature::unaffect` sets the sight to the literal
  13 and only then removes the effect flag;** `m_OldSight`, which the affect
  and the loader both write, is read nowhere. `EffectLightness::unaffect`
  does it the right way round: remove the flag, then
  `setSight(getEffectedSight())`; `EffectFlare::unaffect` removes its flag
  first too, then restores the sight it saved, or a monster's per-type
  sight. The default and Lightness sights are
  13 too, so it diverges only under another sight effect: a creature blinded
  by Flare and then poisoned gets full vision back the moment the poison
  ticks out, and monsters, whose Flare restore reads the per-type sight,
  are restored to a value that need not be theirs.
  The flag now comes off first and the sight that follows is
  `getEffectedSight()`, so what is left is what the creature's remaining
  effects say: the Flare sight for one still flared, the default otherwise.
  Only a player creature can carry the effect, so the per-type restore
  Flare does for a monster is not needed here.
  `m_OldSight` stays as it is -- it is a persisted column
  (`EffectYellowPoisonToCreature.OldSight`, written by `create` and
  `save`), so the field and its writes are not dead, only its readers.
  > **Status:** fixed (fix/recorded-defects-1)

## The event item action dereferences a null selector for an Ousters (2026-09-22)

- **`ActionGiveEventItem::execute` picks the Lua item selector in an
  `isSlayer()` branch and an `isVampire()` branch, neither of which an
  Ousters enters,** so `pLuaSelectItem` stays null and is dereferenced a
  few lines down.
  There is no Ousters selector to reach for: the action reads a
  `SlayerFilename` and a `VampireFilename` and nothing else, and
  `LuaTradeEventSlayerItem` and `LuaTradeEventVampireItem` are the only
  two selector classes in the tree. So a null selector is now a refusal:
  the action logs the character to `GiveEventItemError.txt`, closes the
  NPC dialogue the way its other refusals do, and returns. The same
  two-branch selector choice is in `ActionGiveAccountEventItem` and
  `ActionTradeGiftBox`, which now refuse it too; `ActionGiveTestServerReward`
  still carries it.
  > **Status:** fixed (fix/recorded-defects-1)

## A skill off cooldown is sent a 4-billion-turn casting time (2026-09-22)

- **`getRemainTurn` returns `Turn_t`, which is `DWORD`.** The body subtracts
  the current time from the slot's run time, so a slot whose cooldown has
  already elapsed — the normal state of every skill a character is not
  mid-cast on — computes a negative remainder and converts it to a value
  near 2^32. All three races do the same thing with it:
  `Slayer::sendSlayerSkillInfo`, `Vampire::sendVampireSkillInfo` and
  `Ousters::sendOustersSkillInfo` put it in the sub-info's *casting time*
  field, and `SubSlayerSkillInfo`/`SubVampireSkillInfo`/`SubOustersSkillInfo`
  write it to the client as four bytes. The load path calls `setRunTime()`,
  which places the run time the stored delay ahead, and each race's
  `addSkill` stores a delay of 0 for a newly learned skill, so a skill
  learned and not yet cast is already past its run time by the time
  `ZoneSpawn.cpp` sends the skill list on the first zone entry after login;
  every later send repeats it — each zone entry, the four `CGSkillTo*`
  handlers after a vampire cast, `EventMorph.cpp`, `PCManager.cpp` and
  `skill/Restore.cpp`. The same body sits in `skill/SkillSlot.cpp` and
  `skill/RaceSkillSlot.cpp` alike, so this is one defect behind three
  callers, not three defects.
  Both bodies now clamp: a run time that is already past reports no turns
  left. The wire layout is unchanged and the client already handles the
  value — `GCSkillInfoHandler` passes the casting time to
  `SKILLINFO_NODE::SetAvailableTime`, whose zero means "usable now", the
  same thing the wrapped value reached through its negative `int`. The
  clamped rule is pinned by `race_skill_slot_tests` and by the new
  `skill_slot_tests`, which covers the slayer slot's copy of the body.
  > **Status:** fixed (fix/recorded-defects-1)

## ObjectManager created a volume info manager it never deleted (2026-09-17)

- **`g_pVolumeInfoManager` was `new`ed in `ObjectManager`'s constructor and
  named nowhere in its destructor.** Every other manager the constructor
  creates is freed there, so the manager leaked for the life of the process;
  it is now an `ObjectManager` member, deleted in the position that mirrors
  its creation, between the item factory and the item loader manager. (Its
  own destructor still frees none of the `VolumeInfo` entries `init()`
  allocates, which is a second leak, left as it is.) The same sweep dropped
  the duplicate `SAFE_DELETE` of `g_pOptionInfoManager` and of
  `g_pSkillDomainInfoManager`, each written twice in that destructor: the
  macro nulls what it frees, so the second call did nothing, but it read as
  a delete of a live pointer.
  > **Status:** fixed (refactor/game-context-4)

## A flag manager could wake up believing a flag war was already on (2026-09-17)

- **`FlagManager::m_bHasFlagWar` was never initialised.** The constructor
  set up the two wars and their schedules but left the flag holding whatever
  the allocation held. Read as true, `startFlagWar()` refused the first
  scheduled or manual start, and `endFlagWar()` then recorded a flag-war
  history row for a war that never happened, complete with the external
  history script run on an uninitialised end time and empty counts, before
  clearing the flag. The member now starts false, so a fresh manager starts
  its first war and records nothing until one has run.
  > **Status:** fixed (refactor/game-context-1)

## A GM command crashed the game server on a dynamic zone type it did not know (2026-09-16)

- **`opAddDynamicZone` dereferenced both halves of
  `getDynamicZoneGroup(type)->getAvailableDynamicZone()` unchecked.**
  `DynamicZoneManager::getDynamicZoneGroup` answers NULL for a type it holds
  no group for -- anything outside the set the dynamic zone info loads -- so
  `*command addDynamicZone` with a number that is not one of them called a
  member function through NULL and took the process down. The body now keeps
  the group pointer, answers the GM that there is no group of that type, and
  answers again should a group hand back no instance; the row already carries
  a `gcSystemMessage`, so both refusals reach the console. A type that has a
  group behaves as before.
  > **Status:** fixed (fix/dead-zone-bodies)

## A GM setting gold read the name of a creature it had just found NULL (2026-09-16)

- **`opSetGold` wrote the money-trace log outside the `pCreature != NULL`
  guard the rest of its body sits in.** The command reads the GM's own
  creature, does nothing with it when there is none -- a line typed before
  the character is in a zone, or by a player whose character failed to load
  -- and then, for an amount at or above the money-trace limit, logged
  `pCreature->getName()` regardless. The log now sits inside the guard, so a
  line with no creature behind it changes nothing and logs nothing instead of
  dereferencing NULL. With a creature, the same trace is written as before.
  > **Status:** fixed (fix/dead-zone-bodies)

## WarSystem::broadcastWarList sends through an unchecked player pointer (2026-09-16)

- **Read in the same sweep and left as it is.** The three
  `pGamePlayer->sendPacket(...)` calls in `WarSystem::broadcastWarList` never
  ask whether there is a player, but its one caller, `opShowWarList`, is
  registered `Relay::PlayerOnly`, and `SubcommandTable::dispatch` passes over
  such a row when the line has no player behind it -- which is the only way a
  `*command` body is reached without one. No other code calls the method. A
  guard was therefore not added; the pin that keeps this closed is the
  `showWarList` row in `tests/gm_console_command_test.cpp`, and a future
  caller that can pass NULL has to guard or the method has to grow one.
  > **Status:** not a defect (fix/dead-zone-bodies)

## Slayer's inventory packets lost the failing frame from the stack trace (2026-09-16)

- **`Slayer::getExtraInfo()` and `Slayer::getInventoryInfo()` opened a
  `__BEGIN_DEBUG` block but not the `__BEGIN_TRY`/`__END_CATCH` pair the
  Vampire and Ousters copies of the same code carried.** Both walk the
  character's items building one info object per item, so a `Throwable`
  raised inside them left the function straight for the caller's handler:
  the exception was the same and reached the same place, but `__END_CATCH`
  had not run, so the recorded stack named the caller and never the builder
  that actually failed. The three copies are now one body on
  `PlayerCreature`, the Vampire and Ousters one, so the Slayer path adds its
  frame and rethrows like its siblings. Control flow is unchanged.
  > **Status:** fixed (refactor/race-shared-2)

## A vampire or ousters character carried indeterminate gold until its first load (2026-09-16)

- **Only `Slayer::Slayer()` set `m_Gold = 0`; the Vampire and Ousters
  constructors left the member uninitialised.** Every path that reads gold
  before `load()` fills it in, such as `getGold()` in an info packet built
  for a character that failed to load, read whatever the allocation held.
  The member now lives on `PlayerCreature`, defined once for the three races,
  and carries a member initialiser of zero, so a fresh character of any race
  starts with the gold a Slayer always started with.
  > **Status:** fixed (refactor/race-shared-1)

## A relayed `*command` could crash the receiving game server (2026-09-16)

- **`GGCommandHandler` ran `*command` sub-command bodies with no player,
  and 40 of the 61 read one the moment they start.** `*world` relays
  whatever a GOD typed after it to every other game server, so
  `*world *command heal` reached each receiving server's `opHeal`, which
  opens with `pGamePlayer->getCreature()` on the NULL the relay hands it.
  Every sub-command row now declares whether its body is correct with no
  player behind the line, and `SubcommandTable::dispatch` passes over a
  `PlayerOnly` row when there is none, the way it passes over a row whose
  gate the caller is below. The behaviour change is that a `PlayerOnly`
  sub-command arriving through the relay now does nothing instead of
  dereferencing NULL; no body changed. `GGCommandHandler` hands the
  relayed message to a table of its own
  (`src/server/gameserver/gm/RelayCommandRegistration.cpp`) listing the
  twelve commands a relay may run, all of which are correct with no
  player; `tests/gm_console_command_test.cpp` and
  `tests/gm_command_router_test.cpp` pin both.
  > **Status:** fixed (refactor/gg-relay-table)

## Guild union changes never reached the other game servers (2026-09-16)

- **`GGCommandHandler` compared a 17-character slice of the relayed
  message against the 15-character name `modifyunioninfo`.**
  `GuildUnionManager::sendModifyUnionInfo` sends `*modifyunioninfo <guild
  id>` to every other game server so that each reloads the union the guild
  belongs to. The receiving slice always carried the space and the first
  digit of the id as well, so it never matched, the relay was dropped
  without a log line, and the other servers kept the old union membership
  until the next `*refreshguildunion` (whose name is 17 characters and did
  match). The slice is now the name's length and `opmodifyunioninfo` runs
  on the relay as intended.
  > **Status:** fixed (refactor/zone-split-1)

## CGWhisper wrote names its own read() refuses (2026-09-16)

- **`CGWhisper::write()` bounded the name at 128 while `read()` bounds
  it at 10.** The packet is client-to-game, so `read()` is the contract
  and `CGWhisperFactory::kMaxSize` budgets the name at 10 as well: a name
  of 11 to 128 bytes was written happily and then refused by every reader
  of the field, the server's own included. `write()` now refuses the same
  lengths `read()` does, which is a behaviour change on the write side --
  the call that used to emit an unreadable packet now throws
  `InvalidProtocolException`. No valid packet's bytes move:
  `tests/wire-layout.txt` and `tests/golden/CGWhisper.code0.hex` are
  unchanged, and the server never writes this packet. The client repo's
  copy (`Client/Packet/Cpackets/CGWhisper.cpp`) carries the same 128/10
  split, recorded there in a comment; it needs the identical change.
  `tests/packet_roundtrip_test.cpp` pins both ends of the bound.
  > **Status:** fixed (fix/exception-text)

## Three GM commands answered any player (2026-09-16)

- **`*find`, `*OpenPayMap` and `*ClosePayMap` ran for anyone who typed
  them.** Each branch of the say handler's command ladder tested two
  names with `nameA || nameB && isGOD()`, and `&&` binds tighter than
  `||`, so the GOD gate guarded only the second, mojibake, name: any
  player could look up which server another account was on, and could
  open or close the pay zone's portals. The command table registers all
  three as GOD-only, and `tests/gm_command_router_test.cpp` pins the
  gate.
  > **Status:** fixed (refactor/gm-command-router)

## Session and take-out write/read disagreements (2026-09-16)

The twelve findings task 1.2 stated as flip-tests in
`tests/packet_session_test.cpp`, the initialisation split the same file
pinned, and the four its header recorded rather than tested. These
packets are client-facing, so `write()` and `getPacketSize()` are the
contract: every fix is a refusal, a cap, a size-accounting correction, a
read-side correction or an initialisation. **No valid packet's bytes
move.** One `tests/wire-layout.txt` line moves — `GCReconnect` 42 → 41 —
a read-buffer budget, not a field on the wire. One golden is re-recorded,
`GCExecuteElement.code0.hex` (`…aa…` → `…03…`), because its fixture
carried a condition byte outside the four the packet now bounds; that is
a fixture change, not a protocol change.

- **`GCReconnect::getPacketSize()` counted a pc-type byte `write()`
  never emitted.** The declared body was one byte longer than the body
  sent, and `writePacket()` puts the declared length on the wire before
  calling `write()`, so the client's next packet header started one byte
  early. The client's own copy
  (`Client/Packet/Gpackets/GCReconnect.cpp`) reads a name, a server
  address and a key and no pc type, so `write()`/`read()` were right and
  the size was wrong: the `szPCType` term is gone from `getPacketSize()`
  and from the factory max, with `m_PCType` and its accessors, which
  nothing wrote, read or initialised — `getPCType()` returned whatever
  the storage held. No `src/server` source used them. The golden is
  unchanged.
  > **Status:** fixed (wire/session-disagreements)

- **`CGRequestIP` and `GCRequestedIP` carried a name past what their
  factory maxima budget.** Neither setter, `write()` nor `read()` bounded
  it, while both maxima budget ten bytes, so an eleven-byte name declared
  and emitted a body past the buffer the receiver sizes from that max —
  and `CGRequestIP::read()` took the full 255 its count byte carries.
  Both setters cut at ten, and both fields go through
  `de::wire::readString`/`writeString` at that width: `{0, 10}` for the
  request, which admits a nameless one, `{1, 10}` for the answer, which
  keeps refusing it on both sides. A ten-byte name's bytes are unchanged.
  > **Status:** fixed (wire/session-disagreements)

- **`CGRequestIP::read` left the name the packet already held
  untouched** when the count byte was zero, so a packet read into twice
  kept the first name while `write()` had emitted none. The helper clears
  the field on a zero length.
  > **Status:** fixed (wire/session-disagreements)

- **`GCGoodsList` narrowed both of its counts to a BYTE before testing
  them.** The record count was narrowed and then compared against
  `MAX_GOODS_LIST`, so 256 records wrapped the count to zero while
  `write()` still emitted every one of them — a listing the client reads
  as empty followed by bytes it takes for the next packet's header. The
  per-record option count was narrowed with no test at all, while
  `GoodsInfo::getPacketMaxSize` budgets 255 options. `addGoodsInfo`
  refuses a record past `MAX_GOODS_LIST` and one whose option list runs
  past 255, and `write()` tests both sizes before narrowing them.
  > **Status:** fixed (wire/session-disagreements)

- **`GCGoodsList::read` appended to the listing the packet already
  held** instead of replacing it, so a packet read into twice declared
  and wrote both listings, and the records of the first leaked. Ownership
  is settled from the sender: `quest/ActionTakeOutGoods.cpp` allocates
  each record fresh and keeps none, so the records belong to the packet —
  `read()` frees what it holds before filling the listing again, the
  destructor frees the rest, and `popGoodsInfo()` hands one to the caller
  to free.
  > **Status:** fixed (wire/session-disagreements)

- **`GCGoodsList` followed pointers it never checked.** `addGoodsInfo`
  accepted `NULL`, which `getPacketSize()` then dereferenced while
  `write()` bounded it through an `Assert()`; and `popGoodsInfo` took
  `front()` off a list it never tested for emptiness, which is undefined
  behaviour on an empty listing and the packet exposes no count to bound
  a drain with. The adder refuses a null record and the pop refuses an
  empty listing, both through `InvalidProtocolException`.
  > **Status:** fixed (wire/session-disagreements)

- **`CGRequestInfo::read`, `CGLotterySelect::read` and
  `CGTypeStringList::read` stored a code byte they compared against
  nothing**, each with a named range in its own header —
  `REQUEST_INFO_MAX`, `TYPE_MAX`, and the three `StringType` values. All
  three test the raw byte before storing it, the shape
  `GCChangeWeather::read` already uses.
  > **Status:** fixed (wire/session-disagreements)

- **`CGCrashReport::write` emitted a body `read()` refused.** `write()`
  admitted an empty OS, call stack and message and emitted each as a bare
  zero-length word, while `read()` refused a zero length on all three, so
  a crash report with any of the three empty disconnected the client that
  sent it. The client's own writer
  (`Client/Packet/Cpackets/CGCrashReport.cpp`) emits the same shape and
  its reader bounds only the upper end, so `read()` admits an empty field
  on all three. Its two fixed-width fields were bounded through
  `Assert()`, which appends to `assertion_failed.log` in the working
  directory before it throws; they are `InvalidProtocolException` now.
  > **Status:** fixed (wire/session-disagreements)

- **Twelve of the eighteen packets left a member the default constructor
  never set**, so a packet sent without every setter called put
  indeterminate bytes on the wire. Every member of all eighteen has an
  initialiser, and the poisoned-storage pin in
  `tests/packet_session_test.cpp` covers the whole set in one list.
  > **Status:** fixed (wire/session-disagreements)

- **`GCUpdateInfo::toString` indexed `Weather2String[m_Weather]`
  unguarded.** The table holds three entries and the enum names a
  `WEATHER_MAX` past them, so a stored value the table does not name read
  past its end. It prints as its number, the way `GCChangeWeather`'s own
  `toString()` does.
  > **Status:** fixed (wire/session-disagreements)

- **`GCExecuteElement` bounded its condition byte on read alone.** The
  four conditions a quest element fires under are `GQuestInfo`'s
  `HAPPEN`, `COMPLETE`, `FAIL` and `REWARD`; the setter and `write()` took
  any byte. Both refuse one past the four now. No reachable send moves:
  the packet's one sender passes the `ElementType` `makeVector` stamped on
  the element.
  > **Status:** fixed (wire/session-disagreements)

- **`CGPortCheck` is registered on the game server's client-facing
  dispatch table although `DatagramPacket::read(SocketInputStream&)`
  refuses a TCP stream by design.** A client that sends the id over TCP
  is already refused there. The registration is not the defect and cannot
  be dropped: `PacketFactoryManager::init()` builds one factory table per
  server process, and the game server's UDP path resolves that same table
  — `Datagram::read(DatagramPacket*&)` (in `src/Core/DatagramFactoryRead.cpp`
  since the 2.4 kernel split) asks `g_pPacketFactoryManager` for
  the packet and `Datagram::isDatagram` names `PACKET_CG_PORT_CHECK` — so
  the entry is what lets the packet arrive at all.
  > **Status:** recorded, left as it is (wire/session-disagreements)

## Creature-state write/read disagreements (2026-09-13)

The five findings task 1.2 stated as flip-tests in
`tests/packet_creature_test.cpp`, the initialisation split the same file
pinned, and the seven its header recorded rather than tested. These
packets are client-facing, so `write()` and `getPacketSize()` are the
contract: every fix is a refusal, a widening to what the field already
carries, a size-accounting correction, a read-side correction or an
initialisation. **No golden changes.** One `tests/wire-layout.txt` line
moves — `GCAddInjuriousCreature` 11 → 21 — a read-buffer budget, not a
field on the wire.

- **`PCSlayerInfo3`, `PCVampireInfo3` and `PCOustersInfo3` had
  hand-written copy constructors that left the union id out**, while
  their equally hand-written assignment operators copied it. The union
  id never reached a client: `Slayer::getSlayerInfo3()`,
  `Vampire::getVampireInfo3()` and `Ousters::getOustersInfo3()` each
  refresh the character's cached record — coordinates, HP, alignment,
  guild id, and the union id they look up through
  `GuildUnionManager::getGuildUnion` — and then `return m_SlayerInfo;`.
  Returning a *member* by value is a copy the compiler cannot elide, so
  the copy constructor ran on every call and dropped the id it had just
  written. Ten packets hold one of those records by value and are built
  from those getters: `GCAddSlayer`, `GCAddVampire` and `GCAddOusters`
  (every zone scan, every character that walks into view, via
  `PacketUtil.cpp`'s `makeGCAdd*`), the three corpse packets,
  `GCAddVampireFromBurrowing`, `GCAddVampireFromTransformation` and the
  two morph packets. What the client got in those four bytes was
  whatever the return slot held: zero for a vampire, whose record
  declared `uint m_UnionID = 0`, and indeterminate bytes for a slayer or
  an ousters, whose records declared no member initialisers at all. All
  three copy constructors and assignment operators are `= default` — the
  records are plain data — and every member of all three has an
  initialiser, so a copy of one carries no indeterminate byte either.
  > **Status:** fixed (wire/creature-disagreements)

- **`GCMorph1::getPacketSize()` did not count the pc-type byte
  `write()` puts in front of the record**, so the declared body was one
  byte shorter than the body sent. `writePacket()` puts the declared
  length on the wire before calling `write()`, so the extra byte was not
  a wrong length but a stream the client could not resynchronise: the
  next packet's header started one byte late. The factory max already
  budgeted the byte. It counts `szBYTE` now.
  > **Status:** fixed (wire/creature-disagreements)

- **`GCMorph1` followed null record pointers.** Its constructor sets all
  four to `NULL`, `getPacketSize()` dereferenced all four and `write()`
  guarded only the PC one, so a sender that set the PC record and forgot
  the inventory crashed the game server instead of being refused. Both
  refuse a missing record through `InvalidProtocolException`, and
  `write()`'s pc-type bound is that exception in place of the `Assert()`
  that appended to `assertion_failed.log` in the working directory before
  throwing. Ownership is settled from the senders: all three
  (`EventMorph.cpp` and `skill/Restore.cpp` twice) pass records the
  `Slayer`/`Vampire` accessors allocate fresh and nobody keeps, so the
  four belong to the packet — each setter frees what it replaces,
  `read()` frees the four it holds before filling four more, and the
  destructor frees what is left. A second `read()` leaked all four
  before.
  > **Status:** fixed (wire/creature-disagreements)

- **`GCAddInjuriousCreature` bounded at ten the character name it
  carries.** `write()` and `read()` both stopped at ten while the name
  comes from a creature and runs to `maxNameLength` (20), so a player
  with an eleven-byte name could not be announced as the one who struck
  first: `write()` raised instead of truncating, and the target never
  learned who to strike back at. The setter cuts at `maxNameLength`,
  `write()` and `read()` carry the field at that width, and the factory
  max budgets a whole name (`szBYTE + maxNameLength`, 11 → 21). A
  ten-byte name's bytes are unchanged.
  > **Status:** fixed (wire/creature-disagreements)

- **`GCChangeWeather::read` cast the wire byte straight to `Weather`.**
  The enum declares three weathers and a `WEATHER_MAX`, so a byte past
  its range was an out-of-range enum load, which the Debug toolchain's
  UBSan traps on; and `Weather2String`, which `toString()` indexes with
  the stored value, holds three entries, so a stored `WEATHER_MAX` read
  past its end. `read()` tests the raw byte against `WEATHER_MAX` before
  assigning, and `toString()` prints a value the table does not name as
  its number.
  > **Status:** fixed (wire/creature-disagreements)

- **`GCKnocksTargetBackOK1::read` and `GCKnocksTargetBackOK5::read`
  loaded the success flag straight into a `bool` member**, so a wire byte
  other than 0 or 1 became an invalid bool — a value on which every later
  test of it is undefined. Both read a BYTE, refuse anything but 0 or 1
  and assign, the shape `SlayerSkillInfo` and `OustersSkillInfo` already
  use.
  > **Status:** fixed (wire/creature-disagreements)

- **`GCExecuteElement::read` took a condition byte it compared against
  nothing**, where the four conditions a quest element fires under are
  `GQuestInfo::ElementType`'s `HAPPEN`, `COMPLETE`, `FAIL` and `REWARD`.
  `read()` refuses a byte past them. The sending side is left as it is:
  the packet's one sender, `GQuestExecuteElement::checkCondition`, passes
  the `ElementType` `GQuestInfo::makeVector` stamped on the element, so
  no reachable send is out of range and no emitted byte moves.
  > **Status:** fixed (wire/creature-disagreements)

- **`GCMorphVampire2`'s accessor for the vampire record it holds was
  named `getSlayerInfo`.** It is `getVampireInfo`, matching
  `setVampireInfo` beside it and `GCAddVampire`. No `src/server` source
  called it.
  > **Status:** fixed (wire/creature-disagreements)

- **`CGBloodDrain::read` and `::write` copied the object id as raw
  bytes** (`iStream.read((char*)&m_ObjectID, szObjectID)`) rather than
  through the typed stream calls every other packet in the set uses,
  which is what `de::WireScalar` exists to constrain. `szObjectID` is
  `sizeof(ObjectID_t)`, so the bytes are identical.
  > **Status:** fixed (wire/creature-disagreements)

- **Thirty-one of the forty-two left at least one member the default
  constructor never set**, so a packet sent without every setter called
  put indeterminate bytes on the wire — a client reading a coordinate, a
  skill type or a direction that was never assigned. All forty-two
  initialise every member their `write()` emits.
  > **Status:** fixed (wire/creature-disagreements)

## Error paths that could not report anything (2026-09-13)

- **`XMLUtil::filelog`'s own overflow branch passed a null format to
  itself.** `filelog(NULL, "filelog buffer overflow!")` reaches
  `vsnprintf(buf, n, NULL, valist)` on the one path that exists to report
  that formatting already failed, which is undefined behaviour; the message
  is the format now. Both `filelog` overloads also threw their overflow
  `Error` before `va_end(valist)`, leaving the variadic state unclosed on
  the throwing path; `va_end` runs right after `vsnprintf` in both.
  > **Status:** fixed (seam/item-literal-throws)

- **`EventManager::addEvent` had its debug macros in the wrong order**
  (`__BEGIN_TRY __END_DEBUG` ... `__BEGIN_DEBUG __END_CATCH`), which
  expands to an empty `try` with two handlers, then the whole body outside
  any `try`, then a second empty one: the duplicate-event-class `Error` it
  raises reached the caller with nothing printed and no frame added to its
  stack. Correcting the order to `__BEGIN_TRY __BEGIN_DEBUG` ...
  `__END_DEBUG __END_CATCH` puts the body inside both, so the refusal is
  written to standard output and carries `addEvent` in its trace. It still
  propagates: both macros rethrow.
  > **Status:** fixed (seam/item-literal-throws)

## Chat, nickname, union and SMS write/read disagreements (2026-09-13)

The eleven findings task 1.2 stated as flip-tests in
`tests/packet_chat_test.cpp`, the initialisation split the same file
pinned, and the seven its header recorded rather than tested. These
packets are client-facing, so `write()` and `getPacketSize()` are the
contract: every fix is a refusal, a cap at the width the factory max
budgets, a size-accounting correction, a read-side correction or an
initialisation. **No golden changes.** Two `tests/wire-layout.txt` lines
move — `GCNicknameList` 13001 → 6631 and `CGSMSSend` 108 → 114 — both
read-buffer budgets, not fields on the wire.

- **`GCSystemMessage` carried a `Race_t` with a getter and a setter
  neither `read()` nor `write()` touches**, so a sender's `setRace` never
  left the process. The client's own reader
  (`Client/Packet/Gpackets/GCSystemMessage.cpp`) takes the message behind
  its length byte, the colour and the type byte and stops; its
  `GCSystemMessage.h` declares no race member at all. The two mentions in
  `src/server` — `CGRangerSayHandler.cpp`'s `setRace` and
  `ZonePlayerManager.cpp`'s `getRace` — both sit inside commented-out
  blocks, so nothing reachable used it. The member and both accessors are
  gone; the commented blocks are left as they are.
  > **Status:** fixed (wire/chat-disagreements)

- **`GCModifyNickname`'s record pointer started indeterminate and
  `getPacketSize()`, `write()` and `read()` all dereferenced it.** A
  sender that skipped `setNicknameInfo` followed an indeterminate value
  and a receiver wrote a whole record through one. All four senders hand
  over a record they keep — the character's `NicknameBook`'s in
  `CGModifyNicknameHandler`, `CGSelectNicknameHandler` and
  `PlayerCreature::whenQuestLevelUpgrade`, a function-local `static` or a
  stack `NicknameInfo` in the two admin commands, which live in
  `gm/ConsoleCommands.cpp` since the 4.1 router — so the packet
  tracks which record is its own the way `GCGQuestStatusModify` does: the
  pointer starts empty, `getPacketSize()` and `write()` refuse on it,
  `read()` allocates and owns the record it fills and frees it before
  allocating the next, and the destructor frees that and never a
  sender's.
  > **Status:** fixed (wire/chat-disagreements)

- **`GCRequestFailed::setCode` took a `WORD` into a `BYTE`, and the
  name's length byte was derived with no bound** while the factory max
  budgets ten bytes for it, so an eleven-byte name already outgrew the
  read buffer the receiver sizes from that max. The setter takes the
  `BYTE` the wire carries (its one caller passes `REQUEST_FAILED_IP`),
  the name is cut to `kMaxNameLength` in the setter and carried through
  `de::wire` in `write()` and `read()`, and the zero-length refusal both
  halves raised as `ProtocolException("")` — a log line naming no field —
  is now the helper's `InvalidProtocolException` naming `Name`. The max
  is unchanged, restated as that constant.
  > **Status:** fixed (wire/chat-disagreements)

- **`CGModifyNickname::setItemObjectID` took a `WORD`** while the member
  it writes and the wire both carry an `ObjectID_t`, so no caller could
  set an item id past 65535. It takes the full `ObjectID_t`; no
  `src/server` source calls it.
  > **Status:** fixed (wire/chat-disagreements)

- **`NicknameInfo::write` bounded its custom nickname at the 255 its
  length byte carries where `read()` stopped at `MAX_NICKNAME_SIZE` and
  refused an empty one**, so the record `write()` emits for a
  `NICK_CUSTOM` slot with no text could not be read back. All three sides
  agree at `{0, MAX_NICKNAME_SIZE}` now. The empty value is admitted
  rather than refused because senders produce it —
  `CGModifyNicknameHandler`'s add-a-nickname branch builds a
  `NICK_CUSTOM` record straight from the packet's nickname without
  checking it, and `CGModifyNickname::read` admits an empty one — and
  because the client's own `NicknameInfo::read` writes the field only
  when its length byte is non-zero. The three `Assert(false)` defaults
  that closed the switch in `getSize()`, `read()` and `write()`, which
  append to `assertion_failed.log` in the working directory before they
  throw, are `InvalidProtocolException`s; in `read()` that is also the
  range check on the type byte.
  > **Status:** fixed (wire/chat-disagreements)

- **Three counted lists derived a count byte they capped nowhere**, so
  the 256th entry wrapped it to zero while `write()` still emitted every
  one: `GCNicknameList`'s records, `GCSMSAddressList`'s entries and
  `CGSMSSend`'s receivers. Each is held to what its own maximum budgets —
  255, `MAX_ADDRESS_NUM` 30 and `MAX_RECEVIER_NUM` 5 — in `write()` and
  in `read()`. `GCNicknameListFactory`'s max budgeted `MAX_NICKNAME_NUM`
  500 records, twice what the count byte in front of them can ever
  describe, so `MAX_NICKNAME_NUM` is the 255 that byte carries and the
  max follows it: `GCNicknameList` 13001 → 6631.
  `SMSAddressBook::addAddressElement` compared its size with `>` before
  inserting, so the book could reach 31 entries and the listing packet
  refuse to write at all; it stops at the thirty the listing carries.
  > **Status:** fixed (wire/chat-disagreements)

- **`CGSMSSendFactory`'s maximum budgeted `MAX_RECEVIER_NUM` (5) bytes
  for the caller number where `read()` accepts `MAX_NUMBER_LENGTH`
  (11)** — the wrong constant in the arithmetic — so a packet built at
  the lengths `read()` admits outgrew the read buffer by six bytes. The
  max budgets the length it accepts, and a packet at every read cap is
  now exactly that max: `CGSMSSend` 108 → 114. The same packet derived
  all four of its length bytes from the strings with no bound, so a
  message past `MAX_MESSAGE_LENGTH` was emitted whole and one of 256
  wrapped its byte to zero; every string travels through `de::wire` now,
  at the widths the max budgets. Its four `Assert()` bounds on the read
  side, which wrote `assertion_failed.log` before throwing, are
  `InvalidProtocolException`s.
  > **Status:** fixed (wire/chat-disagreements)

- **`GCSystemMessage::read`, `GCKickMessage::read` and
  `GCKickMessage::setType` cast a byte straight to an enum narrower than
  a byte.** `SystemMessageType` declares eight values and
  `KickMessageType` two, so a byte above their range is an out-of-range
  enum load, which the Debug toolchain's UBSan traps on. Each tests the
  raw `BYTE` against the enum's `_MAX` before assigning.
  > **Status:** fixed (wire/chat-disagreements)

- **`~GCNicknameList` and `~GCSMSAddressList` freed no record, and both
  `read()`s cleared the vector without freeing what it held**, so every
  record a `read()` allocated leaked. The two differ in who owns what.
  `GCNicknameList`'s one sender, `NicknameBook::getNicknameBookListPacket`,
  pushes the book's own records, so the listing starts unowned and the
  packet frees only what `read()` allocated. Every `GCSMSAddressList`
  entry is the packet's: its one sender,
  `SMSAddressBook::getGCSMSAddressList`, builds a fresh `AddressUnit` per
  address through `SMSAddressElement::getAddressUnit` and keeps only the
  element, so the destructor frees the whole listing and `read()` frees
  before replacing it. `CGSMSAddressListHandler` also never deleted the
  packet it sent — the nickname list's handler does — so the records
  leaked with it; it deletes it now.
  > **Status:** fixed (wire/chat-disagreements)

- **Fifteen of the thirty-one left at least one member the default
  constructor never sets**, so a packet sent without every setter called
  put indeterminate bytes on the wire: `GCSay`'s object id and colour,
  `GCWhisper`'s and `GCGlobalChat`'s colour and race, `CGGlobalChat`'s
  colour, `GCKickMessage`'s seconds, `CGSelectNickname`'s nickname id,
  `CGModifyNickname`'s item id, the six union packets' guild id,
  `CGQuitUnion`'s quit method, `CGAppointSubmaster`'s guild id and
  `CGDeleteSMSAddress`'s element id. All 31 initialise every member their
  `write()` emits now, pinned over poisoned storage.
  > **Status:** fixed (wire/chat-disagreements)

- **`GCFriendChatting` capped its message at 128 on read and 512 on
  write, and admitted on write the empty name and message `read()`
  refused; `CGAddSMSAddress` admitted all three of its empty fields on
  write and refused them on read.** Both are read-side widenings, because
  the write side is what the senders and the client already agree on: the
  client's `GCFriendChatting::read` refuses only past 32 and 512 and
  writes each field only when its length is non-zero, most of
  `GCFriendChattingHandler`'s sends leave both strings empty, and the
  client's `CGAddSMSAddress::write` admits an empty value in each of the
  three. The pins in `tests/packet_roundtrip_test.cpp` follow.
  > **Status:** fixed (wire/chat-disagreements)

## A bare string-literal throw walked past every handler written for it (2026-09-10)

- **149 sites answered a refusal with `throw "text"`, a `const char*`
  nothing in the tree catches.** `__END_CATCH` rethrows a `Throwable` and
  the 344 `__END_CATCH_NO_RETHROW` sites swallow one; `__END_DEBUG_EX`,
  which every CG handler ends with, rethrows an `Error` and logs a plain
  `Exception`; a skill's own `catch (Throwable& t)` runs its
  `executeSkillFailException` branch. A `const char*` matches none of
  them, so it walked past the handler the surrounding code had written for
  exactly this failure and landed in whatever `catch (...)` stood highest —
  `main()`'s "unknown exception...", `GamePlayer::processCommand`'s
  disconnect, `ManagedThread`'s `ServerShutdown::fail()` — with the message
  dropped on the way. Out of a destructor there is no backstop at all:
  `~GamePlayer` is implicitly `noexcept`, so its literal was
  `std::terminate` for the whole game server whenever a departing player's
  party lookup missed. The 62 sites outside `src/server/gameserver/item`
  now throw `Error` with the same text, the 15 Korean ones translated.
  Each lands in the handler that was already there: a load failure reaches
  `main()`'s `catch (Throwable&)`, which writes it to `../log/instant.log`
  and refuses to start where the literal only printed "unknown
  exception..."; a skill's missing-ammunition refusal reaches the skill's
  own fail branch instead of disconnecting the player; a zone tick failure
  reaches `zoneGroupThreadError.log` before the same shutdown; and
  `~GamePlayer`'s reaches the destructor's own `__END_CATCH_NO_RETHROW`,
  which swallows it after the `cerr` line the site already writes — a
  swallow accepted deliberately, because terminating every logged-in
  session over one departing player's party bookkeeping is not a refusal
  worth keeping. The item classes' identical `Invalid item type or
  optionType` constructor refusal, 87 sites, one per class, went the same
  way. It is raised only by the `(itemType, optionType)` constructor the
  `ItemFactory` subclasses call, never by the argument-less one every
  `<Class>Loader::load` uses, so no database row is skipped at startup by
  the change. Eleven of the paths that reach a factory do land somewhere
  else now. Four improve: the item-making skills (`CreateBomb`,
  `CreateMine`, `CreateHolyWater`, `AbsorbSoul`) already catch `Throwable`
  and answer with their own skill-fail packet, so a refusal ends the skill
  instead of disconnecting the player. Seven turn a kill into a swallow:
  monster loot (`MonsterManager::processCreatures`) and NPC-triggered quest
  actions (`NPCManager::processCreatures`) logged nothing and took the whole
  server down through `ManagedThread`'s `catch (...)`, and now log one line
  and abandon the rest of that tick; the GM relic command, the NPC-dialogue
  rewards, the event-tree inventory move, the quick-slot motorcycle key and
  `ActionRedeemMotorcycle` disconnected the player and now swallow, four of
  them with their diagnostic commented out. All eleven are accepted: the
  `filelog("itembug.log", ...)` line beside every one of these throws
  reports the failure whatever catches it, and a malformed `ItemInfo` row is
  not worth a server. Ratchet R11 is 0, so a new literal anywhere in `src`
  fails it.
  > **Status:** fixed; the 62 sites outside `src/server/gameserver/item` in
  > seam/statement-leak, the 87 item constructors in
  > seam/item-literal-throws

## A statement that met anything but a SQL failure was never closed (2026-09-10)

- **`BEGIN_DB`/`END_DB` closed the `Statement` only in the clause that
  answers a failed statement.** The shape all 682 sites share is
  `Statement* pStmt = NULL; BEGIN_DB { pStmt = conn->createStatement();
  ...; SAFE_DELETE(pStmt); } END_DB(pStmt)`, and that `SAFE_DELETE` sits
  inside the try. Any other exception raised between the query and it — a
  `bad_alloc`, an `OutOfBoundException` or `NoSuchElementException` from a
  container walked while building the row, an `Error` from a helper called
  with the statement still open — carried the site's only reference away
  and leaked the `Statement` together with the `Result` it owns, which for
  a SELECT is the whole result set. `END_DB`, `END_DB_EX` and
  `MySQLSMSMessageRepository.cpp`'s file-local `END_DB_RETHROW` now carry a
  `catch (...)` that deletes the statement and rethrows unchanged, so every
  site is covered without touching one. Nothing is logged there: the
  failure is not the statement's, and the handler that catches it decides
  what to say. Double deletion cannot follow, because `SAFE_DELETE` clears
  the pointer: a site that finished early leaves NULL behind, and the four
  blocks in `MySQLExchangeRepository.cpp` that open a second statement each
  do it after a `SAFE_DELETE` of the first. `tests/database_error_test.cpp`
  pins the clause with no database, counting destructions through a
  `Statement` subclass declared at the site — the macro deletes the pointer
  with the type the site gives it — for a `Throwable`, for a
  `std::bad_alloc`, for the SQL clause, and for a site that already closed
  its statement.
  > **Status:** fixed (seam/statement-leak)

## Quest, war and zone-selection write/read disagreements (2026-09-10)

The fifteen findings task 1.2 stated as flip-tests in
`tests/packet_quest_war_test.cpp`, the initialisation split the same file
pinned, and the six its header recorded rather than tested. These packets
are client-facing, so `write()` and `getPacketSize()` are the contract:
every fix is a refusal, a cap at the width the factory max budgets, a
size-accounting correction, a read-side correction or an initialisation.
**No golden changes.** Two `tests/wire-layout.txt` lines move —
`GCWarList` 13344 → 14257 and `GCWarScheduleList` 2281 → 2401 — both
read-buffer budgets for packets no server reads, not fields on the wire.

- **`GCRegenZoneStatus::read` appended the eight bytes it took off the
  wire to the eight its constructor had already pushed into a
  `vector<BYTE>`**, so `getStatus()` kept returning the constructor's
  zeros and `write()` emitted them again: the packet could not round trip
  at all. `write()` always emits exactly eight, so the statuses are a
  fixed eight-slot array `read()` writes into, and a slot outside the
  eight is refused in the getter and the setter instead of indexing past
  the end. `RegenZoneManager` indexes those slots with the
  `RegenZonePosition.id` column, whose seed rows are 0..7.
  > **Status:** fixed (wire/quest-war-disagreements)

- **`GCMiniGameScores::getPacketSize` never advanced its iterator**, so
  it counted the first name's length once per entry and a table of names
  of different lengths declared a size `write()` did not send —
  `writePacket()` puts that size on the wire before `write()` runs, so
  the stream never resynchronises. It walks the table now. The same
  packet derived each name's length byte from the name with no bound
  while its max budgets twenty bytes for one, so a name of 256 wrapped
  the byte to zero while `write()` emitted the whole string: `addScore`
  cuts the name to twenty, `write()` and `read()` carry the field through
  `de::wire`, and `read()` refuses a table past the ten `write()` emits.
  The max is unchanged, restated as its two constants.
  > **Status:** fixed (wire/quest-war-disagreements)

- **`GCNoticeEvent::getCode` returned a `BYTE` of the `WORD` it holds and
  puts on the wire**, so a caller could not see a code past 255, and
  **`setParameter(WORD, WORD)` assigned `makeDWORD` of its two halves to
  the code** rather than the parameter, overwriting the notice with the
  low half. Both are fixed; no `src/server` source called either. The
  code was also never compared against `NOTICE_EVENT_MAX`, so a peer
  could announce a notice no branch of the client draws: `read()` refuses
  on the raw `WORD` before it reaches the member the three switches read.
  > **Status:** fixed (wire/quest-war-disagreements)

- **`MissionInfo::write` printed every mission it emitted to standard
  output**, on the wire path, once per player the packet was sent to. The
  print is gone.
  > **Status:** fixed (wire/quest-war-disagreements)

- **Five counted lists derived a count byte they capped nowhere**, so the
  256th entry wrapped it to zero while `write()` still emitted every one:
  `GCGQuestStatusInfo`'s records, `QuestStatusInfo`'s missions,
  `GCWarList`'s wars, `GCWarScheduleList`'s entries and the `ValueList`
  carrying a guild war's join guilds and a race war's castles. Each is
  held to what its own maximum budgets — `MAX_QUEST_NUM` 100,
  `MAX_MISSION_NUM` 100, 24 wars, `MAX_WAR_NUM` 20 and 255 values — in
  the adder where there is one, in `write()` and in `read()`.
  `GCSelectQuestID` and `GCMonsterKillQuestInfo` did refuse a list past
  255, but through `Assert()`, which appends to `assertion_failed.log` in
  the working directory before it throws; both throw
  `InvalidProtocolException` now, and `GCMonsterKillQuestInfo` refuses in
  its adder as well.
  > **Status:** fixed (wire/quest-war-disagreements)

- **`GCWarListFactory`'s maximum budgeted twelve race wars and twelve
  guild wars and nothing else** — not the count byte, not the war type
  byte in front of each record, and not the level war its own `read()`
  builds — so twelve full guild wars and twelve full race wars already
  outgrew the buffer the receiver sizes from it. `write()` can put any of
  the three shapes behind each war type byte, so the max budgets the
  count byte and twenty-four times the widest of them, a `GuildWarInfo`:
  `GCWarList` 13344 → 14257. Twenty-four is the record count the old
  number already stood for, and `addWarInfo`, `write()` and `read()`
  refuse past it.
  > **Status:** fixed (wire/quest-war-disagreements)

- **`GCWarScheduleList` derived each of the six guild names' length bytes
  from the name with no bound.** The names are held to sixteen through
  `de::wire`, the width its max budgets — and which the golden's own
  sixteen-byte reinforcing guild name shows is the real field width — so
  a name of 256 can no longer wrap the byte while `write()` emits the
  whole string. The max never budgeted the length byte in front of each
  of those six names, so a full schedule of full names outgrew it by six
  bytes an entry; it counts them now: `GCWarScheduleList` 2281 → 2401.
  `WarScheduleInfo` also initialises every member, so an entry
  `read()` builds for a war type that carries no challengers no longer
  holds indeterminate guild ids.
  > **Status:** fixed (wire/quest-war-disagreements)

- **Seven readers appended to what the packet already held instead of
  replacing it**, so a reused packet grew by one listing per read:
  `GCSelectQuestID`, `GCGQuestStatusInfo`, `GCWarList`,
  `GCWarScheduleList`, `GCMonsterKillQuestInfo`, `QuestStatusInfo`'s
  mission list and `ValueList`. All seven replace what they hold at the
  top of `read()`, freeing the records they drop where they own them.
  > **Status:** fixed (wire/quest-war-disagreements)

- **`GCGQuestStatusInfo`'s destructor freed no record — the loop that did
  was commented out — and `GCGQuestStatusModify::read` allocated one on
  every call and freed none**, while its destructor dropped that one too.
  Every sender of both hands over a `GQuestStatus` the character's
  `GQuestManager` owns and keeps (`GQuestManager::getStatusInfoPacket`
  pushes `m_QuestStatuses` entries into the listing; `accept`, `cancel`,
  the two mission-progress sites and `GQuestStatus::update` hand over the
  status itself), so both packets track which record is their own: the
  listing and the pointer start unowned, `read()` frees what it allocated
  before allocating the next, and the destructor frees that and never a
  sender's. `QuestStatusInfo` owns the missions it holds and frees them,
  which is where the records `read()` allocates were leaking their
  missions; `GQuestStatus` no longer frees them itself.
  `GCGQuestStatusModify`'s record pointer starts empty and
  `getPacketSize()` and `write()` refuse on it rather than following the
  indeterminate value a sender that skipped `setInfo` left.
  > **Status:** fixed (wire/quest-war-disagreements)

- **`GCWarList::read` cast the war type byte straight to the `WarType`
  enum before switching on it.** The enum declares three values, so its
  range is 0..3 and any byte above that is an out-of-range enum load,
  which the Debug toolchain's UBSan traps on. The raw `BYTE` is tested
  against the three shapes `read()` builds before it reaches the enum.
  **`GCFlagWarStatus::getFlagCount` and `setFlagCount` indexed a
  three-element array with a `Race_t` they never bounded**; both refuse a
  race outside the three slots `write()` emits.
  > **Status:** fixed (wire/quest-war-disagreements)

- **Eighteen of the twenty-seven left at least one member the default
  constructor never set**, so a packet sent without every setter called
  put indeterminate bytes on the wire. All twenty-seven initialise every
  member now, pinned by constructing each over storage poisoned with two
  different bytes. `MissionInfo`, `GuildWarInfo` and `LevelWarInfo`
  initialise theirs with them.
  > **Status:** fixed (wire/quest-war-disagreements)

- **`GCAddHelicopter`'s code getter was named `setCode`**, overloading
  the setter. It is `getCode`; no `src/server` source called the getter.
  > **Status:** fixed (wire/quest-war-disagreements)

## Every SQL failure threw a dangling pointer (2026-09-10)

- **`END_DB` and `END_DB_EX` built the `DBError.log` line in a local
  `std::string` and then threw `msg.c_str()`**, a pointer into storage
  that dies as the catch clause unwinds. All 735 statement sites in
  `src/` answer a failed statement that way, and the 34
  `catch (const char*)` handlers in `src/server` receive it. Nothing
  crashed only because almost every one of them logged a fixed string
  and never read the pointer; the MySQL integration tier said so in a
  comment beside each refusal it pinned, having no way to assert on the
  error MySQL gave. The macros now throw a `DatabaseError`
  (`src/server/database/DatabaseError.h`), a value holding the same
  message, and the handlers catch that type. The ones that report the
  failure carry the real text rather than pointing at the log: the
  startup managers' `Error`, the login handlers' `DisconnectException`,
  `PaySystem`'s `paySystem.txt` lines and `CLDeletePCHandler`'s console
  line. `DatabaseError` derives from nothing on purpose — `__END_CATCH`
  rethrows a `Throwable` and `__END_CATCH_NO_RETHROW` swallows one, so a
  base would have changed how far a SQL failure travels before it
  reaches the handler that decides what the caller is told.
  `MySQLSMSMessageRepository`'s file-local `END_DB_RETHROW` is
  untouched: its thread catches the `SQLQueryException` itself to
  reconnect. `tests/database_error_test.cpp` pins the macros with no
  database, and the MySQL tier pins a statement against a missing table;
  both copy the error out of the handler and read its message after the
  catch block is gone. Ratchet R10 keeps both halves at zero.
  > **Status:** fixed (seam/db-error-type)

## Character progression write/read disagreements (2026-09-10)

The nine findings task 1.2 stated as flip-tests in
`tests/packet_skill_test.cpp`, the initialisation split the same file
pinned, and three of the four its header recorded rather than tested.
These packets are client-facing, so `write()` and `getPacketSize()` are
the contract: every fix is a refusal, a cap at the width the factory max
already budgets, a size-accounting correction, a read-side correction or
an initialisation. **No golden changes.** One `tests/wire-layout.txt`
line moves — `GCSkillInfo` 4338 → 34706 — a read-buffer budget for a
packet no server reads, not a field on the wire.

- **`GCSkillInfo` derived its record count into a `BYTE` it capped
  nowhere, and `GCSkillInfoFactory`'s maximum was one bare
  `SlayerSkillInfo`.** The 256th record wrapped the count to zero while
  `write()` still emitted every record, and the max budgeted neither the
  pc type byte nor the record count byte nor a second record, so a single
  full record already outgrew the buffer the receiver sizes from it. A
  book carries one record per skill domain — `SKILL_DOMAIN_MAX`, 8, the
  bound `Slayer::sendSlayerSkillInfo` fills against and the one a vampire
  and an ousters use one slot of — and `addListElement`, `write()` and
  `read()` refuse past it. The max budgets the two header bytes and eight
  slayer records, the widest of the three races: `GCSkillInfo`
  4338 → 34706.
  > **Status:** fixed (wire/skill-disagreements)

- **`SlayerSkillInfo`, `VampireSkillInfo` and `OustersSkillInfo` kept
  their skill count in a `BYTE` the caller set by hand while
  `addListElement` left it alone.** `write()` emitted that count and then
  the whole list, so a record whose list was longer sent skills the
  reader never consumed and `getPacketSize()` counted them. The count is
  the list now, held to the 255, 120 and 120 each record's own
  `getMaxSize()` budgets in the adder, in `write()` and in `read()`.
  `setListNum` is gone from all three, with the `SkillCount` bookkeeping
  in `Slayer.cpp`, `Vampire.cpp` and `Ousters.cpp` that maintained it.
  > **Status:** fixed (wire/skill-disagreements)

- **`GCSkillInfo::read` appended to the book the packet already held**,
  as `GCRankBonusInfo`, `GCSweeperBonusInfo`, `GCHolyLandBonusInfo` and
  `GCBloodBibleList` did to their lists, so a reused packet declared one
  listing and held two. All five replace what they hold at the top of
  `read()`, with `GCSkillInfo` destroying the records it drops instead of
  leaking them.
  > **Status:** fixed (wire/skill-disagreements)

- **`GCSkillInfo::write` emitted any pc type while `read()` refused every
  one but `PC_SLAYER`, `PC_VAMPIRE` and `PC_OUSTERS`**, so a book
  announced with any other type went out and could not be read back — and
  the type was one of the members the constructor left alone. The setter,
  `write()` and `read()` refuse the same three-value set, testing the raw
  `BYTE` before it reaches an enum, which the Debug toolchain's UBSan
  traps on.
  > **Status:** fixed (wire/skill-disagreements)

- **`GCRankBonusInfo`, `GCSweeperBonusInfo` and `GCHolyLandBonusInfo`
  derived their count byte from a list they bounded against nothing.**
  The 256th entry wrapped the count to zero while `write()` still emitted
  every entry, and the body outgrew maxima budgeting 100, 12 and 12
  entries. Each is held to its own max's number in the adder, in
  `write()` and in `read()`. `GCBloodBibleList`, whose list is reached
  through `getList()`, only `Assert`ed the same bound and now refuses past
  the 12 its max budgets in `write()` and in `read()`. All four maxima
  were already right, so no wire-layout line moves for them.
  > **Status:** fixed (wire/skill-disagreements)

- **`SweeperBonusInfo` and `BloodBibleBonusInfo` carried a type the
  managers set on every record and the wire never took.** Both `read()`
  and `write()` handle the race byte alone; the type and the option list
  are commented out on both sides. The client decides which side is right
  and it reads no type: its own
  `Client/Packet/Gpackets/SweeperBonusInfo.cpp` and
  `BloodBibleBonusInfo.cpp` read the race byte only, and
  `GCSweeperBonusInfoHandler` and `GCHolyLandBonusInfoHandler` use
  `getRace()` and never `getType()`. So the wire is right and the type was
  dead API: `m_Type` / `getType` / `setType` are gone from both records,
  with the two `SweeperBonusManager::makeSweeperBonusInfo` /
  `makeVoidSweeperBonusInfo` calls that set it. Both records' `getSize()`
  and `getMaxSize()` already counted the race byte alone, so neither
  factory max moves. The option list the same two calls still fill is
  dead the same way and is left alone.
  > **Status:** fixed (wire/skill-disagreements)

- **`GCBloodBibleSignInfo` left its record pointer uninitialised and both
  `getPacketSize()` and `write()` dereferenced it**, while `read()`
  allocated a record on every call and freed none and the destructor
  dropped the pointer. Every sender hands the packet the character's own
  `BloodBibleSignInfo` (`PlayerCreature::getBloodBibleSign`) and keeps it,
  so the packet tracks which record is its own: the pointer starts empty
  and `getPacketSize()` and `write()` refuse on it, `read()` frees the
  record it allocated before allocating the next, and the destructor frees
  that one and never a sender's.
  > **Status:** fixed (wire/skill-disagreements)

- **`BloodBibleSignInfo::read` appended to the sign list it already held
  and bounded the count against nothing**, where `write()` emits at most
  the six slots the record has and `getMaxSize()` budgets six. It replaces
  the list and refuses a count past `BLOOD_BIBLE_SIGN_SLOT_NUM`.
  > **Status:** fixed (wire/skill-disagreements)

- **Eighteen of the twenty-nine left at least one member the default
  constructor never set**, so a packet sent without every setter called
  put whatever the allocation held on the wire. All twenty-nine
  initialise every member now, pinned by constructing each over storage
  poisoned with two different bytes.
  > **Status:** fixed (wire/skill-disagreements)

- **Each race record's learn flag and a slayer skill's enable flag were
  read straight into a `bool`**, so a peer sending any byte but 0 or 1
  left a `bool` holding a value no `bool` may hold. Each travels as a
  `BYTE` that `read()` holds to 0 or 1 before it reaches the member. A
  sender writes a real `bool`, so no byte on the wire changes.
  > **Status:** fixed (wire/skill-disagreements)

- **Four `throw "..."` raw string literals in the packet layer could be
  caught by nothing.** `__END_CATCH` catches `Throwable`, so an
  out-of-range index in `GCShopList::getShopItem`,
  `GCShopListMysterious::getShopItem`, `GCShopVersion::getVersion` or
  `GCShopVersion::setVersion` unwound past every handler on the path.
  All four throw `InvalidProtocolException`, pinned in
  `tests/packet_store_test.cpp`. The two remaining raw-literal throws in
  `src/Core` (`SXml.cpp` and `Utility.cpp`, both in `filelog()`) are not
  in the packet layer and are left alone.
  > **Status:** fixed (wire/skill-disagreements)

- `GCUsePowerPointResult::read` and `GCRequestPowerPointResult::read`
  take their code bytes without comparing them against the last
  enumerator of the `RESULT_CODE` and `ITEM_CODE` lists their own headers
  declare, so a peer can announce a result no branch of the client
  handles.
  > **Status:** open — recorded in `tests/packet_skill_test.cpp`

## Hard-coded BBS credentials in the `*notice` operator command (2026-09-10)

- **`CGSayHandler::opnotice` opened a MySQL connection to a
  third-party host whose address, database, user and password were
  literals in the source**, and inserted the GM's chat text into that
  server's `quick1001` table **unquoted and unescaped**: the text went
  straight between the VALUES parentheses, so whatever a GM typed was
  SQL. The credentials are not moved into a repository. Nothing else
  happened in the command — it answered the player nothing and touched
  no game state — so the command is deleted: the dispatch branch that
  reached it and the whole body. Its declaration stays in
  `src/Core/CGSay.h`, the packet header the client repo mirrors.
  > **Status:** fixed (seam/inline-sql-residue)

- **`MySQLSMSMessageRepository` turned every SQL failure into a
  `const char*`, so the SMS relay could never reconnect.**
  `SMSServiceThread::run()` catches `SQLQueryException` to call
  `reopen()` on the repository, but the repository's statements sat
  inside `END_DB`, which logs to `DBError.log` and then throws
  `msg.c_str()` of a local string. Neither that branch nor the
  thread's `catch (Throwable&)` matches a `const char*`, so a dropped
  relay connection left the loop instead of being reopened. The
  repository now catches the `SQLQueryException` itself, writes the
  same `DBError.log` line and rethrows it; `END_DB` and its other call
  sites are untouched. Dormant: `GameServer::start()` never starts the
  thread.
  > **Status:** fixed (seam/inline-sql-residue)

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
> **Status:** fixed (refactor/shared-twin-classes) — the shadowing
> declaration is gone, so the branch deletes the player the outer pointer
> names, the shape the ConnectException branch below it already had, and
> `m_pGameServerPlayers` is default-initialised, so a slot no connection
> filled reads as NULL instead of as an indeterminate pointer. The table
> is still indexed by raw descriptor and sized 100, and a descriptor above
> that overruns it, which this entry does not close.

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
  Separately, the trailing `m_Agree` byte was written only under
  `__NETMARBLE_SERVER__`, which nothing defines, but was counted
  unconditionally in `getPacketMaxSize()`; the byte and the macro are
  both gone now (R14), so the size no longer carries it.
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
