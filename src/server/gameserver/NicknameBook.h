#ifndef __NICKNAME_BOOK_H__
#define __NICKNAME_BOOK_H__

#include <unordered_map>

#include "NicknameInfo.h"
#include "Types.h"

class PlayerCreature;
class Packet;
class NicknameRepository;

class NicknameBook {
public:
    // The repository defaults to the process-wide MySQL one; tests inject
    // a fake instead.
    NicknameBook(PlayerCreature* pOwner, NicknameRepository* pRepository = 0);

    NicknameInfo* getNicknameInfo(WORD id) {
        return m_Nicknames[id];
    }
    void setNicknameInfo(WORD id, NicknameInfo* pInfo) {
        m_Nicknames[id] = pInfo;
    }

    Packet* getNicknameBookListPacket() const;
    WORD popNicknameID() {
        return m_NextNicknameID++;
    }

    void load();
    void addNewNickname(const string& nick);

private:
    PlayerCreature* m_pOwner;
    NicknameRepository* m_pRepository;
    unordered_map<WORD, NicknameInfo*> m_Nicknames;
    WORD m_NextNicknameID;
};

#endif
