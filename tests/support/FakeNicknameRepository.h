#ifndef __FAKE_NICKNAME_REPOSITORY_H__
#define __FAKE_NICKNAME_REPOSITORY_H__

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "repository/NicknameRepository.h"

// In-memory NicknameRepository for domain tests (docs/RESTRUCTURING.md 3.2).
// Mirrors the MySQL implementation's contract:
//  - PRIMARY KEY (nID, OwnerID): insert() on an existing (owner, id) THROWS,
//    like the real plain INSERT's duplicate-key error; only
//    insertDefaultCustomSlot() is idempotent (INSERT IGNORE).
//  - load() returns rows in nID-ASCENDING order — the real SELECT carries
//    no ORDER BY, but the secondary index IDX_OwnerID carries the primary
//    key (nID, OwnerID) as its suffix, so the ref scan returns nID order
//    (pinned by the MySQL integration tier; this comment originally said
//    "insertion order" — falsified there).
//  - Nickname is varchar(22) latin1 with STRICT_TRANS_TABLES off: stored
//    values silently truncate to 22 bytes.
class FakeNicknameRepository : public NicknameRepository {
public:
    std::vector<NicknameRecord> load(const std::string& ownerName) {
        std::vector<NicknameRecord> records;
        for (Rows::const_iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->first.first == ownerName)
                records.push_back(itr->second);
        }
        std::sort(records.begin(), records.end(), byID);
        return records;
    }

    void insertDefaultCustomSlot(const std::string& ownerName) {
        if (find(ownerName, 0) != m_Rows.end())
            return; // INSERT IGNORE: the slot already exists
        NicknameRecord record;
        record.id = 0;
        record.type = NicknameInfo::NICK_CUSTOM;
        record.nickname = " "; // single space, never empty — client quirk
        record.index = 0;
        m_Rows.push_back(std::make_pair(key(ownerName, 0), record));
    }

    void insert(const std::string& ownerName, WORD id, BYTE type, const std::string& nickname) {
        if (find(ownerName, id) != m_Rows.end())
            // the real impl raises SQLQueryException through the DB layer
            throw std::runtime_error("FakeNicknameRepository: duplicate PRIMARY KEY (nID, OwnerID)");
        NicknameRecord record;
        record.id = id;
        record.type = type;
        record.nickname = truncateToColumn(nickname);
        record.index = 0; // column default: the plain insert omits NickIndex
        m_Rows.push_back(std::make_pair(key(ownerName, id), record));
    }

    void updateNickname(const std::string& ownerName, WORD id, const std::string& nickname) {
        Rows::iterator itr = find(ownerName, id);
        if (itr != m_Rows.end())
            itr->second.nickname = truncateToColumn(nickname);
    }

private:
    typedef std::pair<std::string, WORD> RowKey;
    typedef std::vector<std::pair<RowKey, NicknameRecord>> Rows;

    static RowKey key(const std::string& ownerName, WORD id) {
        return RowKey(ownerName, id);
    }

    static bool byID(const NicknameRecord& a, const NicknameRecord& b) {
        return a.id < b.id;
    }

    static std::string truncateToColumn(const std::string& nickname) {
        return nickname.size() > 22 ? nickname.substr(0, 22) : nickname;
    }

    Rows::iterator find(const std::string& ownerName, WORD id) {
        for (Rows::iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->first == key(ownerName, id))
                return itr;
        }
        return m_Rows.end();
    }

    Rows m_Rows;
};

#endif
