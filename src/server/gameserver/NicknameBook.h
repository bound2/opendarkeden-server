#ifndef __NICKNAME_BOOK_H__
#define __NICKNAME_BOOK_H__

#include <memory>

#include <unordered_map>

#include "GCNicknameList.h"
#include "NicknameInfo.h"
#include "Types.h"

class PlayerCreature;
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

    // The listing the client redraws the book from. Its records belong to
    // the book, so the packet must not outlive them and owns none of them;
    // the caller sends it and lets it go.
    std::unique_ptr<GCNicknameList> getNicknameBookListPacket() const;
    WORD popNicknameID() {
        return m_NextNicknameID++;
    }

    void load();
    void addNewNickname(const string& nick);

    // The repository this book was constructed with. Callers that mutate
    // the book's rows must write through THIS, not the process-wide
    // default, or a book built over a fake still writes to MySQL.
    NicknameRepository& repository() const {
        return *m_pRepository;
    }

private:
    PlayerCreature* m_pOwner;
    NicknameRepository* m_pRepository;
    unordered_map<WORD, NicknameInfo*> m_Nicknames;
    WORD m_NextNicknameID;
};

#endif
