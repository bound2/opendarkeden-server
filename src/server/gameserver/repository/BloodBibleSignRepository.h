#ifndef __BLOOD_BIBLE_SIGN_REPOSITORY_H__
#define __BLOOD_BIBLE_SIGN_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the BloodBibleSignObject table (task 3.2). The
// gameserver only ever READS this table: no code path in this repository
// inserts, updates, or deletes rows — sign grants arrive from outside the
// server process. The interface is read-only on purpose; a write method
// would pretend at a flow that does not exist.
class BloodBibleSignRepository {
public:
    virtual ~BloodBibleSignRepository() {}

    // Every stored sign for a character, ordered by ItemType ascending
    // (the original query's ORDER BY, which the client list relies on).
    virtual std::vector<ItemType_t> loadItemTypes(const std::string& ownerName) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLBloodBibleSignRepository.cpp. An accessor function rather than a
// g_p* extern: ratchet R1 counts those.
BloodBibleSignRepository& defaultBloodBibleSignRepository();

#endif
