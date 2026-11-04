---
type: Rule
title: "Exceptions and outcomes"
description: "Throw an Error, never a string literal; gameplay results are Outcome values and exceptions are for programming and configuration errors; a database error is stopped where its failed statement has a meaning, never by a catch of Throwable."
tags: [architecture, conventions]
enforced_by: [ratchets, game_server_runtime_tests, login_server_runtime_tests, database_error_sites_audit]
generated: { by: claude-code/claude-fable-5-1, at: 2026-10-21T13:00:00Z }
---

# Exceptions and outcomes

**`throw Error("text")`, never `throw "text"`.** A bare literal throws a `const char*`, a type
nothing in the tree catches, so it walks past every handler into a `catch (...)` backstop or
`std::terminate`. Ratchet R11 holds bare string-literal throws at zero and R10b the
`catch (const char*)` handlers, so a `const char*` is neither thrown nor caught anywhere; R10a
holds throws of a pointer into a local string (`throw s.c_str()`) at zero, and R12 throw messages
carrying non-ASCII text ([ratchets](../testing/ratchets.md)).

**Gameplay results are `Outcome<Events, Rejection>`** (`src/Core/Outcome.h`): every refusal, the
precedence between refusals and the content of the reply are values a test checks against a fake
repository with no database, the way `decideLogin()` is tested in `login_decision_tests`.
Exceptions are for programming and configuration errors.

**A catch that drops a `Throwable` is control flow.** A `catch (Throwable&)` whose body is empty,
comments and `;` aside, ends the exception there: the caller never learns its block failed. The
gameserver's swallowing try macros became such catches unchanged, and each was judged, test first
where a test could drive the failure (task 3.8 lists the sites no test drives yet); a catch whose
block cannot throw a `Throwable` (it only clears standard containers, deletes objects whose
destructors are `noexcept` and calls code that throws no `Throwable`) is dead and goes without a
test, since nothing can drive it. A `noexcept` destructor cannot let the exception out, so there the
decision is what it does instead: it takes no lock of its own object ([critical
sections](../threading/critical-sections.md)), and it checks an invariant with `AssertNoThrow`
(`src/Core/Assert.h`), which appends the entry a failed `Assert` writes to `assertion_failed.log`
and returns; when its work fails (an effect's `unaffect`, a player's save, the server's stop), it
logs the `Throwable` to `destructorError.log` with the bounded `diagnosticFilelog` and carries on
(`Slayer`'s and `Vampire`'s saves keep their own `slayerDestructor.txt` and
`vampireDestructor.txt`). Outside a destructor, an exception a caller handles propagates, and one no
caller handles is logged where it is caught. R19 holds empty `catch (Throwable&)` clauses at zero in
the gameserver outside `gm/`, `handler/` and `packetfill/`, so a new one fails the suite, and R5
holds the try macros at zero in the whole gameserver.

A malformed packet body is the one exception class with a contract of its own: a reader refuses it
only with a `ProtocolException`, and the fuzz targets abort on anything else
([frame gate](../protocol/frame-gate.md)).

**A database error is stopped where its failed statement has a meaning.** `DatabaseError`
(`src/server/database/DatabaseError.h`, what `END_DB` throws) is unrelated to `Throwable` on
purpose: the swallowing `__END_CATCH_NO_RETHROW` sites and the rethrowing `__END_CATCH` stand
between a statement and the code that decides what a failure means, so the type they match may
not be one of its bases, and a `catch (const DatabaseError&)` names each place that decides.
Nothing else stops it, so a failed statement that reaches a worker's top ends the process
(`ManagedThread::start` turns what escapes a worker into `ServerShutdown::fail()`, and the
loginserver's main loop catches nothing). Three places decide for a loop that would otherwise end:
the login's kick wait, where `runLoginKickWait` (`src/server/loginserver/LoginKickRetry.h`)
reports the failure by its kind and retires the login with a `DisconnectException`, as the
dispatch wrap does for a handler, and the player's `disconnect` logs the account off; the flag
war's steps, where `FlagManager::heartbeat` logs the failure and queues the step again a minute
later, the one schedule it has, since `FlagWar::executeStart` and `executeFinish` clear or record
the round's statistics before they change or queue anything; and the zone-group tick, where
`ZonePlayerManager::processCommands` moves a player whose statement failed out of the group (the
logout's cleanup first, where there is a creature; its own save may fail on the same database),
`ZoneGroup::processPlayers` and `ZoneGroup::heartbeat` log a failure in a tick's step and go on
with the next zone or the next tick, and `GamePlayer::disconnect` ends the local session past a
failed save or log-off, the account row then keeping `GAME` until the loginserver's kick path
reclaims it. Owners: `LoginKickWait` in `login_server_runtime_tests`, `GameFlagStepFailure` and
`GameZoneDatabaseFailure` in `game_server_runtime_tests`, and `database_error_sites_audit`
(`tests/tools/check_database_error_sites.sh`) for the tick's and the disconnect's catches, which
no test drives without a loaded zone and a database. Left as they were: the main thread's
`warSystem().heartbeat()` and event heartbeat in `ClientManager::run`, and a statement that fails
inside a zone's own step (`Zone::heartbeat`), which the group's catch skips for that tick while
the step's scheduler keeps it for the next.
