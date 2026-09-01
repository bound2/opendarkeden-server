#ifndef __FAKE_RANK_BONUS_REPOSITORY_H__
#define __FAKE_RANK_BONUS_REPOSITORY_H__

#include <string>
#include <utility>
#include <vector>

#include "repository/RankBonusRepository.h"

// In-memory RankBonusRepository for domain tests (docs/RESTRUCTURING.md
// 3.2). Mirrors the MySQL implementation's contract:
//  - The table has NO primary or unique key: insert() of the same
//    (owner, type) twice stores two rows, and loadTypes() surfaces both.
//  - loadTypes() returns values in INSERTION order — the real SELECT
//    carries no ORDER BY.
//  - deleteOne() removes EVERY row of that type, duplicates included.
class FakeRankBonusRepository : public RankBonusRepository {
public:
    std::vector<DWORD> loadTypes(const std::string& ownerName) {
        std::vector<DWORD> types;
        for (Rows::const_iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->first == ownerName)
                types.push_back(itr->second);
        }
        return types;
    }

    void insert(const std::string& ownerName, DWORD type) {
        m_Rows.push_back(std::make_pair(ownerName, type));
    }

    void deleteOne(const std::string& ownerName, DWORD type) {
        for (Rows::iterator itr = m_Rows.begin(); itr != m_Rows.end();) {
            if (itr->first == ownerName && itr->second == type)
                itr = m_Rows.erase(itr);
            else
                ++itr;
        }
    }

    void deleteAll(const std::string& ownerName) {
        for (Rows::iterator itr = m_Rows.begin(); itr != m_Rows.end();) {
            if (itr->first == ownerName)
                itr = m_Rows.erase(itr);
            else
                ++itr;
        }
    }

private:
    typedef std::vector<std::pair<std::string, DWORD>> Rows;

    Rows m_Rows;
};

#endif
