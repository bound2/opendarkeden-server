#ifndef __FAKE_GOODS_REPOSITORY_H__
#define __FAKE_GOODS_REPOSITORY_H__

#include <string>
#include <vector>

#include "repository/GoodsRepository.h"

// In-memory GoodsRepository for domain tests (docs/RESTRUCTURING.md 3.2).
// addPurchase() stands in for the website writing a purchase row. Mirrors
// the MySQL implementation's contract:
//  - loadPending() filters on world + playerID + character name + Status
//    'NOT' and returns storage order (the real SELECT has no ORDER BY).
//  - takeOne() finds by id alone (no Status check in its WHERE): the
//    decrement clamps at 0 (Num is tinyint UNSIGNED with strict mode
//    off), and Status becomes 'GET' when the ALREADY-DECREMENTED count
//    is below 1 — MySQL's left-to-right SET evaluation quirk.
//  - takeOne() returns whether anything CHANGED (the connection lacks
//    CLIENT_FOUND_ROWS): an unknown id is false, and so is a re-take of
//    an exhausted row already at Num=0/'GET'.
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

            int newNum = itr->num > 0 ? itr->num - 1 : 0; // unsigned clamp
            bool newTaken = newNum < 1;                   // IF() sees the new Num
            bool changed = newNum != itr->num || newTaken != itr->taken;
            itr->num = newNum;
            itr->taken = newTaken;
            return changed;
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
