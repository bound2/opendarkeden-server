#ifndef __FAKE_GOLD_REPOSITORY_H__
#define __FAKE_GOLD_REPOSITORY_H__

#include <stdexcept>
#include <string>
#include <vector>

#include "repository/GoldRepository.h"

// In-memory GoldRepository for domain tests (docs/RESTRUCTURING.md 3.2).
// Models the Gold column of the three race tables. Mirrors the MySQL
// implementation's contract (empirically pinned by the MySQL integration
// tier in tests/integration/):
//  - The writes are RELATIVE (Gold = Gold ± delta) against the ROW's
//    balance, on the character's OWN table only — no Slayer fan-out here,
//    unlike stash.
//  - addRow() seeds a character's row; an UPDATE against a missing row is
//    a SILENT no-op.
//  - decreaseGoldClamped() is the guild-fee write, whose clamp is IN
//    the statement (SET Gold = IF (fee > Gold, 0, Gold - fee)) because
//    its payer is offline. It therefore does NOT throw where
//    decreaseGold does: a row short of the fee is zeroed instead.
//  - decreaseGold() below the row's balance throws: Gold is int(10)
//    UNSIGNED, and the unsigned subtraction raises ER_DATA_OUT_OF_RANGE
//    (1690), leaving the row untouched. The real error surfaces as a raw
//    const char* out of END_DB; the fake throws std::runtime_error so
//    tests have something typed to catch.
//  - NOT modeled: deltas above INT_MAX and sums above the column's
//    4294967295 maximum (real MySQL would clamp with a warning under the
//    non-strict sql_mode; the fake's int arithmetic would just go
//    negative). Unreachable while the callers clamp at MAX_MONEY.
class FakeGoldRepository : public GoldRepository {
public:
    // Test seeding: the character's row in one race table.
    void addRow(CharacterRace race, const std::string& ownerName, int gold) {
        Row row;
        row.race = race;
        row.ownerName = ownerName;
        row.gold = gold;
        m_Rows.push_back(row);
    }

    void increaseGold(const std::string& ownerName, CharacterRace race, Gold_t delta) {
        for (Rows::iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->race == race && itr->ownerName == ownerName)
                itr->gold += (int)delta;
        }
        // no matching row: the real UPDATE matches zero rows, silently
    }

    void decreaseGold(const std::string& ownerName, CharacterRace race, Gold_t delta) {
        for (Rows::iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->race == race && itr->ownerName == ownerName) {
                if (itr->gold < (int)delta)
                    // ER_DATA_OUT_OF_RANGE: the row is left untouched
                    throw std::runtime_error(
                        "FakeGoldRepository: BIGINT UNSIGNED value is out of range in 'Gold - delta'");
                itr->gold -= (int)delta;
            }
        }
    }

    void decreaseGoldClamped(const std::string& ownerName, CharacterRace race, Gold_t fee) {
        for (Rows::iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->race == race && itr->ownerName == ownerName)
                // The clamp is the statement's, so a row that cannot pay
                // is emptied rather than raising — silently, as far as
                // the caller is concerned.
                itr->gold = itr->gold < (int)fee ? 0 : itr->gold - (int)fee;
        }
    }

    bool loadGold(const std::string& ownerName, CharacterRace race, int& gold) {
        for (Rows::const_iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->race == race && itr->ownerName == ownerName) {
                gold = itr->gold;
                return true;
            }
        }
        return false;
    }

private:
    struct Row {
        CharacterRace race;
        std::string ownerName;
        int gold;
    };
    typedef std::vector<Row> Rows;

    Rows m_Rows;
};

#endif
