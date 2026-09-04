#ifndef __SYSTEM_AVAILABILITY_REPOSITORY_H__
#define __SYSTEM_AVAILABILITY_REPOSITORY_H__

#include <string>
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
// those, so both arms are compiled out. The statements are enclosed
// anyway: they are real on a Thailand or China build, and the ratchets
// count them textually either way.
//
// Not enclosed: nothing. SystemAvailabilities is named nowhere else in
// the tree outside this seam and its two callers.

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
    // The quoting is kept — SystemKind is int(11) and the call sites
    // write '0', '1' and so on as strings.
    virtual void deleteSystemKind(const char* systemKind) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSystemAvailabilityRepository.cpp. An accessor function rather than
// a g_p* extern: ratchet R1 counts those.
SystemAvailabilityRepository& defaultSystemAvailabilityRepository();

#endif
