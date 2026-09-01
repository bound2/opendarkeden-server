#ifndef __FAKE_STASH_REPOSITORY_H__
#define __FAKE_STASH_REPOSITORY_H__

#include <string>
#include <vector>

#include "repository/StashRepository.h"

// In-memory StashRepository for domain tests (docs/RESTRUCTURING.md 3.2).
// Models the stash columns of the three race tables plus an audit log of
// the write attempts. Mirrors the MySQL implementation's contract
// (empirically pinned by the MySQL integration tier in tests/integration/):
//  - Every save writes the Slayer table UNCONDITIONALLY, then Ousters if
//    isOusters, else Vampire — two writes per save, always. addRow()
//    seeds a character's row (character creation inserts Slayer plus the
//    race's own row); an UPDATE against a table with no row for the name
//    is a SILENT no-op, which the audit log still records as attempted.
//  - Values marshal through the same (int) cast the SQL interpolation
//    applies, so a Gold_t above 2^31-1 shows up negative in the audit —
//    and the UNSIGNED column then clamps the stored value to 0 (warning
//    1264 under the non-strict sql_mode), destroying the balance.
//  - loadStashGold() reads ONE table, the character's own.
class FakeStashRepository : public StashRepository {
public:
    struct Write {
        std::string table; // "Slayer" / "Vampire" / "Ousters"
        std::string ownerName;
        std::string column; // "StashNum" / "StashGold"
        int value;
    };

    // Test seeding: the character's row in one race table.
    void addRow(CharacterRace race, const std::string& ownerName) {
        Row row;
        row.race = race;
        row.ownerName = ownerName;
        row.stashNum = 0;
        row.stashGold = 0;
        m_Rows.push_back(row);
    }

    void saveStashNum(const std::string& ownerName, bool isOusters, BYTE num) {
        record(ownerName, isOusters, "StashNum", (int)num);
    }

    void saveStashGold(const std::string& ownerName, bool isOusters, Gold_t gold) {
        record(ownerName, isOusters, "StashGold", (int)gold);
    }

    bool loadStashGold(const std::string& ownerName, CharacterRace race, int& gold) {
        for (Rows::const_iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->race == race && itr->ownerName == ownerName) {
                gold = itr->stashGold;
                return true;
            }
        }
        return false;
    }

    const std::vector<Write>& writes() const {
        return m_Writes;
    }

private:
    struct Row {
        CharacterRace race;
        std::string ownerName;
        int stashNum;
        int stashGold;
    };
    typedef std::vector<Row> Rows;

    void apply(CharacterRace race, const std::string& ownerName, const std::string& column, int value) {
        Write write;
        write.table = characterRaceTable(race);
        write.ownerName = ownerName;
        write.column = column;
        write.value = value;
        m_Writes.push_back(write);

        for (Rows::iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->race == race && itr->ownerName == ownerName) {
                // int(10) unsigned: a negative literal clamps to 0
                int stored = value < 0 ? 0 : value;
                if (column == "StashNum")
                    itr->stashNum = stored;
                else
                    itr->stashGold = stored;
            }
        }
        // no matching row: the real UPDATE matches zero rows, silently
    }

    void record(const std::string& ownerName, bool isOusters, const std::string& column, int value) {
        apply(CHARACTER_RACE_SLAYER, ownerName, column, value);
        apply(isOusters ? CHARACTER_RACE_OUSTERS : CHARACTER_RACE_VAMPIRE, ownerName, column, value);
    }

    Rows m_Rows;
    std::vector<Write> m_Writes;
};

#endif
