#ifndef __STASH_REPOSITORY_H__
#define __STASH_REPOSITORY_H__

#include <string>

#include "Types.h"

// Persistence seam for the stash columns (task 3.2). There is no stash
// table: StashNum/StashGold are columns ON the three race tables
// (Slayer/Vampire/Ousters), written outside the normal character save by
// the immediate-persist stash operations. isOusters selects which second
// table is written — see the quirk notes on the MySQL implementation.
class StashRepository {
public:
    virtual ~StashRepository() {}

    // Persist the stash slot count for a character.
    virtual void saveStashNum(const std::string& ownerName, bool isOusters, BYTE num) = 0;

    // Persist the stash gold total (the already-computed new balance).
    virtual void saveStashGold(const std::string& ownerName, bool isOusters, Gold_t gold) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLStashRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
StashRepository& defaultStashRepository();

#endif
