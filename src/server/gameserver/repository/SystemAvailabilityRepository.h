#ifndef __SYSTEM_AVAILABILITY_REPOSITORY_H__
#define __SYSTEM_AVAILABILITY_REPOSITORY_H__

#include <vector>

#include "Types.h"

// Persistence seam for SystemAvailabilities (task 3.2): the per-system
// on/off flags the server reads at boot, and the six rows the
// egg-dummy-DB shutdown path deletes on its way out.
//
// WORTH KNOWING BEFORE YOU JUDGE THIS SEAM'S VALUE. Neither caller's SQL
// compiles into the shipped gameserver. SystemAvailabilitiesManager::load
// puts its read behind
// "#if defined(__CHINA_SERVER__) || defined(__THAILAND_SERVER__)" and
// falls back to marking every system available; EventShutdown puts its
// deletes behind the "#else" of
// "#if !defined(__THAILAND_SERVER__) && !defined(__CHINA_SERVER__)". The
// gameserver build defines __GAME_SERVER__ and __COMBAT__ and neither of
// those, so both arms are compiled out. One caveat the audit owes: that
// is true of the BUILD, not of every translation unit. ItemUtil.cpp
// #defines __THAILAND_SERVER__ itself, unconditionally, and then
// includes SystemAvailabilitiesManager.h — so one shipped TU does see
// the macro. Neither converted caller lives there, so both stay
// compiled out, but "the build defines neither" is not the whole
// story. (That self-define also gives ItemUtil.cpp a
// SystemAvailabilitiesManager with an extra member and a different
// layout from every other TU's. Pre-existing, out of scope here, and
// worth someone's attention.) The statements are enclosed
// anyway: they are real on a Thailand or China build, and the ratchets
// count them textually either way.
//
// Not enclosed: nothing. No other SQL in the tree touches the
// SystemAvailabilities table. The NAME appears in plenty of other
// places — the schema and seed data in initdb/, the
// GCSystemAvailabilities packet, and the manager's own
// SystemAvailabilities.log — so "named nowhere else", which an earlier
// version of this line said, was wrong. It is the statements that are
// all here.

// One row of the boot-time read, which is a "SELECT *" — so the caller's
// positional getInt(1)/getInt(2) depend on SystemKind and Available being
// the first two columns. They are; Description is third and unread. A
// column added before them would silently reassign every flag, which is
// the hazard "SELECT *" carries and the reason it is worth naming here.
struct SystemAvailabilityRow {
    int systemKind;
    int available;
};

class SystemAvailabilityRepository {
public:
    virtual ~SystemAvailabilityRepository() {}

    // SystemAvailabilitiesManager::load.
    virtual std::vector<SystemAvailabilityRow> loadAll() = 0;

    // EventShutdown's teardown. The six statements differ only in the
    // quoted value, so this is one format rather than six literals: the
    // bytes MySQL receives are identical either way, which is NOT true of
    // the spelling variants other seams keep apart with an enum. Those
    // differ in how a statement is written; these differ in what it says.
    //
    // The quoting stays in the SQL text, because the call sites wrote it
    // there — but the PARAMETER is an int, not a string. SystemKind is
    // int(11), "'%d'" renders all six values byte-identically to the
    // originals, and an int cannot carry SQL into the statement the way
    // an unescaped %s could. An earlier version took a const char*,
    // which preserved nothing the format did not already preserve.
    virtual void deleteSystemKind(int systemKind) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSystemAvailabilityRepository.cpp. An accessor function rather than
// a g_p* extern: ratchet R1 counts those.
SystemAvailabilityRepository& defaultSystemAvailabilityRepository();

#endif
