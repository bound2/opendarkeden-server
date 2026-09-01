#ifndef __FAKE_BLOOD_BIBLE_SIGN_REPOSITORY_H__
#define __FAKE_BLOOD_BIBLE_SIGN_REPOSITORY_H__

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "repository/BloodBibleSignRepository.h"

// In-memory BloodBibleSignRepository for domain tests
// (docs/RESTRUCTURING.md 3.2). The production interface is READ-ONLY —
// nothing in the gameserver writes the table — so the fake's addRow()
// stands in for the out-of-band process that grants signs. Mirrors the
// MySQL implementation's contract:
//  - loadItemTypes() returns ascending ItemType order (the query's
//    ORDER BY), regardless of insertion order.
//  - Duplicate ItemTypes are possible (the table's key is an unrelated
//    auto-increment) and come back as-is.
class FakeBloodBibleSignRepository : public BloodBibleSignRepository {
public:
    // Test seeding only: in production these rows appear from outside the
    // server process.
    void addRow(const std::string& ownerName, ItemType_t itemType) {
        m_Rows.push_back(std::make_pair(ownerName, itemType));
    }

    std::vector<ItemType_t> loadItemTypes(const std::string& ownerName) {
        std::vector<ItemType_t> itemTypes;
        for (Rows::const_iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->first == ownerName)
                itemTypes.push_back(itr->second);
        }
        std::sort(itemTypes.begin(), itemTypes.end());
        return itemTypes;
    }

private:
    typedef std::vector<std::pair<std::string, ItemType_t>> Rows;

    Rows m_Rows;
};

#endif
