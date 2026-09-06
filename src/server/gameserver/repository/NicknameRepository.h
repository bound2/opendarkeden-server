#ifndef __NICKNAME_REPOSITORY_H__
#define __NICKNAME_REPOSITORY_H__

#include <string>
#include <vector>

#include "NicknameRecord.h"

// Persistence seam for the NicknameBook table — the task 3.2 pilot
// repository. Game logic talks to this interface; the MySQL implementation
// lives with the app wiring (MySQLNicknameRepository.cpp) and quarantines
// the legacy schema quirks there, never in domain types.
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

    // CGSayHandler's GM forced-nickname commands (CGSay round, 2026-09-06):
    // the id-100 slot. "REPLACE INTO NicknameBook (nID, OwnerID, NickType,
    // Nickname, NickIndex, Time) VALUES (100, '%s', %u, '%s', 0, now())" — the
    // type is the caller's NicknameInfo enumerator through "%u" — and
    // "DELETE FROM NicknameBook WHERE OwnerID='%s' AND nID=100". The nickname
    // is interpolated raw here (the other writes go through getDBString),
    // as written.
    virtual void replaceForcedNickname(const std::string& ownerName, BYTE type, const std::string& nickname) = 0;
    virtual void deleteForcedNickname(const std::string& ownerName) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLNicknameRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
NicknameRepository& defaultNicknameRepository();

#endif
