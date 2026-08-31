#ifndef __FAKE_NICKNAME_REPOSITORY_H__
#define __FAKE_NICKNAME_REPOSITORY_H__

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "repository/NicknameRepository.h"

// In-memory NicknameRepository for domain tests (docs/RESTRUCTURING.md 3.2).
// Mirrors the MySQL implementation's contract, including the table's
// PRIMARY KEY (nID, OwnerID): one row per (owner, id), and the default
// custom slot insert is idempotent (INSERT IGNORE).
class FakeNicknameRepository : public NicknameRepository {
public:
    std::vector<NicknameRecord> load(const std::string& ownerName) {
        std::vector<NicknameRecord> records;
        for (RowMap::const_iterator itr = m_Rows.begin(); itr != m_Rows.end(); ++itr) {
            if (itr->first.first == ownerName)
                records.push_back(itr->second);
        }
        return records;
    }

    void insertDefaultCustomSlot(const std::string& ownerName) {
        if (m_Rows.find(key(ownerName, 0)) != m_Rows.end())
            return; // INSERT IGNORE: the slot already exists
        NicknameRecord record;
        record.id = 0;
        record.type = NicknameInfo::NICK_CUSTOM;
        record.nickname = " "; // single space, never empty — client quirk
        record.index = 0;
        m_Rows[key(ownerName, 0)] = record;
    }

    void insert(const std::string& ownerName, WORD id, BYTE type, const std::string& nickname) {
        NicknameRecord record;
        record.id = id;
        record.type = type;
        record.nickname = nickname;
        record.index = 0; // column default: the plain insert omits NickIndex
        m_Rows[key(ownerName, id)] = record;
    }

    void updateNickname(const std::string& ownerName, WORD id, const std::string& nickname) {
        RowMap::iterator itr = m_Rows.find(key(ownerName, id));
        if (itr != m_Rows.end())
            itr->second.nickname = nickname;
    }

private:
    typedef std::pair<std::string, WORD> RowKey;
    typedef std::map<RowKey, NicknameRecord> RowMap;

    static RowKey key(const std::string& ownerName, WORD id) {
        return RowKey(ownerName, id);
    }

    RowMap m_Rows;
};

#endif
