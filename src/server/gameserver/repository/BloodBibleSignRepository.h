#ifndef __BLOOD_BIBLE_SIGN_REPOSITORY_H__
#define __BLOOD_BIBLE_SIGN_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// The BloodBibleSignObject table. The gameserver only ever READS it: no
// code path in this tree inserts, updates, or deletes rows — sign grants
// arrive from outside the server process. The interface is read-only on
// purpose.
class BloodBibleSignRepository {
public:
    virtual ~BloodBibleSignRepository() {}

    // Every stored sign for a character, ordered by ItemType ascending
    // (the client list relies on the order).
    virtual std::vector<ItemType_t> loadItemTypes(const std::string& ownerName) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLBloodBibleSignRepository.cpp.
BloodBibleSignRepository& defaultBloodBibleSignRepository();

#endif
