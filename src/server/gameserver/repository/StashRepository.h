#ifndef __STASH_REPOSITORY_H__
#define __STASH_REPOSITORY_H__

#include <string>

#include "Types.h"

// Persistence seam for the stash columns (task 3.2). There is no stash
// table: StashNum/StashGold are columns ON the three race tables
// (Slayer/Vampire/Ousters), written outside the normal character save by
// the immediate-persist stash operations. isOusters selects which second
// table is written — see the quirk notes on the MySQL implementation.
//
// Which race table one character's row lives in. The WRITES fan out
// (Slayer always, plus the race's own table); the integrity-check READ
// targets only the character's own table.
enum StashRace { STASH_RACE_SLAYER = 0, STASH_RACE_VAMPIRE = 1, STASH_RACE_OUSTERS = 2 };

class StashRepository {
public:
    virtual ~StashRepository() {}

    // Persist the stash slot count for a character.
    virtual void saveStashNum(const std::string& ownerName, bool isOusters, BYTE num) = 0;

    // Persist the stash gold total (the already-computed new balance).
    virtual void saveStashGold(const std::string& ownerName, bool isOusters, Gold_t gold) = 0;

    // Read back the stored stash gold from the character's own race table
    // (the checkStashGoldIntegrity flow, run by the stash deposit/withdraw
    // handlers before they call the writes above). Returns false when the
    // table has no row for the name; on true, gold carries the column as
    // the driver's getInt returned it.
    virtual bool loadStashGold(const std::string& ownerName, StashRace race, int& gold) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLStashRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
StashRepository& defaultStashRepository();

#endif
