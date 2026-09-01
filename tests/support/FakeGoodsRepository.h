#ifndef __FAKE_GOODS_REPOSITORY_H__
#define __FAKE_GOODS_REPOSITORY_H__

#include <stdexcept>
#include <string>
#include <vector>

#include "repository/GoodsRepository.h"

// In-memory GoodsRepository for domain tests (docs/RESTRUCTURING.md 3.2).
// addPurchase() stands in for the website writing a purchase row. Mirrors
// the MySQL implementation's contract (empirically pinned by the MySQL
// integration tier in tests/integration/):
//  - loadPending() filters on world + playerID + character name + Status
//    'NOT' and returns storage order (the real SELECT has no ORDER BY).
//  - takeOne() finds by id alone (no Status check in its WHERE): Status
//    becomes 'GET' when the ALREADY-DECREMENTED count is below 1 —
//    MySQL's left-to-right SET evaluation quirk.
//  - takeOne() on a row at Num=0 THROWS: Num - 1 on the UNSIGNED column
//    raises ER_DATA_OUT_OF_RANGE (1690) regardless of strict mode, and
//    the row is left untouched. The real error surfaces as a raw
//    const char* out of END_DB; the fake throws std::runtime_error so
//    tests have something typed to catch.
//  - takeOne() of an unknown id returns false (no row matched).
class FakeGoodsRepository : public GoodsRepository {
public:
    void addPurchase(const std::string& id, int world, const std::string& playerID, const std::string& characterName,
                     DWORD goodsID, int num) {
        Row row;
        row.id = id;
        row.world = world;
        row.playerID = playerID;
        row.characterName = characterName;
        row.goodsID = goodsID;
        row.num = num;
        row.taken = false;
        m_Rows.push_back(row);
    }

    std::vector<GoodsRecord> loadPending(int world, const std::string& playerID, const std::string& characterName) {
        std::vector<GoodsRecord> records;
        for (Rows::const_iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->world == world && itr->playerID == playerID && itr->characterName == characterName &&
                !itr->taken) {
                GoodsRecord record;
                record.id = itr->id;
                record.goodsID = itr->goodsID;
                record.num = itr->num;
                records.push_back(record);
            }
        }
        return records;
    }

    bool takeOne(const std::string& id) {
        for (Rows::iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->id != id)
                continue;

            if (itr->num < 1)
                // ER_DATA_OUT_OF_RANGE: the row is left untouched
                throw std::runtime_error("FakeGoodsRepository: BIGINT UNSIGNED value is out of range in 'Num - 1'");

            itr->num = itr->num - 1;
            itr->taken = itr->num < 1; // IF() sees the new Num
            return true;
        }
        return false;
    }

private:
    struct Row {
        std::string id;
        int world;
        std::string playerID;
        std::string characterName;
        DWORD goodsID;
        int num;
        bool taken; // Status enum: false='NOT', true='GET'
    };
    typedef std::vector<Row> Rows;

    Rows m_Rows;
};

#endif
