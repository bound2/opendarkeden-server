#ifndef __NICKNAME_REPOSITORY_H__
#define __NICKNAME_REPOSITORY_H__

#include <string>
#include <vector>

#include "NicknameRecord.h"

// The NicknameBook table. The MySQL implementation
// (MySQLNicknameRepository.cpp) keeps the legacy schema quirks out of the
// domain types.
class NicknameRepository {
public:
    virtual ~NicknameRepository() {}

    // Every stored row for a character, NICK_NONE rows included.
    virtual std::vector<NicknameRecord> load(const std::string& ownerName) = 0;

    // Ensure the id-0 custom slot exists; idempotent.
    virtual void insertDefaultCustomSlot(const std::string& ownerName) = 0;

    // Store a newly granted nickname (no display index).
    virtual void insert(const std::string& ownerName, WORD id, BYTE type, const std::string& nickname) = 0;

    // Rename an existing row in place.
    virtual void updateNickname(const std::string& ownerName, WORD id, const std::string& nickname) = 0;

    // The GM forced-nickname slot, id 100: REPLACE the row (NickIndex 0,
    // Time now()) or delete it. The nickname is interpolated unescaped
    // here, where the other writes go through getDBString.
    virtual void replaceForcedNickname(const std::string& ownerName, BYTE type, const std::string& nickname) = 0;
    virtual void deleteForcedNickname(const std::string& ownerName) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLNicknameRepository.cpp.
NicknameRepository& defaultNicknameRepository();

#endif
