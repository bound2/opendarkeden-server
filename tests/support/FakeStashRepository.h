#ifndef __FAKE_STASH_REPOSITORY_H__
#define __FAKE_STASH_REPOSITORY_H__

#include <string>
#include <vector>

#include "repository/StashRepository.h"

// In-memory StashRepository for domain tests (docs/RESTRUCTURING.md 3.2).
// The interface is write-only (loading stash state rides the race tables'
// own character load), so the fake is an audit log of column writes rather
// than a table model. It mirrors the MySQL implementation's contract:
//  - Every save writes the Slayer table UNCONDITIONALLY, then Ousters if
//    isOusters, else Vampire — two writes per save, always.
//  - Values are recorded through the same (int) cast the SQL interpolation
//    applies, so a Gold_t above 2^31-1 shows up negative.
class FakeStashRepository : public StashRepository {
public:
    struct Write {
        std::string table; // "Slayer" / "Vampire" / "Ousters"
        std::string ownerName;
        std::string column; // "StashNum" / "StashGold"
        int value;
    };

    void saveStashNum(const std::string& ownerName, bool isOusters, BYTE num) {
        record(ownerName, isOusters, "StashNum", (int)num);
    }

    void saveStashGold(const std::string& ownerName, bool isOusters, Gold_t gold) {
        record(ownerName, isOusters, "StashGold", (int)gold);
    }

    const std::vector<Write>& writes() const {
        return m_Writes;
    }

private:
    void record(const std::string& ownerName, bool isOusters, const std::string& column, int value) {
        Write slayer;
        slayer.table = "Slayer";
        slayer.ownerName = ownerName;
        slayer.column = column;
        slayer.value = value;
        m_Writes.push_back(slayer);

        Write second = slayer;
        second.table = isOusters ? "Ousters" : "Vampire";
        m_Writes.push_back(second);
    }

    std::vector<Write> m_Writes;
};

#endif
