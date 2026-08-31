#ifndef __NICKNAME_RECORD_H__
#define __NICKNAME_RECORD_H__

#include <string>
#include <vector>

#include "NicknameInfo.h"
#include "Types.h"

// One stored row of a character's NicknameBook, as the domain sees it:
// plain data, no SQL types. Rows typed NICK_NONE exist in storage and are
// surfaced here; skipping them is the caller's decision.
struct NicknameRecord {
    WORD id;
    BYTE type; // NicknameInfo::NICK_* values
    std::string nickname;
    WORD index;
};

// First id handed to a newly added custom nickname: ids below 10000 are
// reserved for the built-in slots, and rows typed NICK_NONE never claim
// an id.
inline WORD nextNicknameIDAfter(const std::vector<NicknameRecord>& records) {
    WORD next = 10000;
    for (size_t i = 0; i < records.size(); ++i) {
        if (records[i].type == NicknameInfo::NICK_NONE)
            continue;
        if (records[i].id >= next)
            next = records[i].id + 1;
    }
    return next;
}

// Whether the id-0 custom slot (the one CGModifyNickname edits in place)
// exists yet. A NICK_NONE row at id 0 does not count.
inline bool hasCustomSlot(const std::vector<NicknameRecord>& records) {
    for (size_t i = 0; i < records.size(); ++i) {
        if (records[i].id == 0 && records[i].type != NicknameInfo::NICK_NONE)
            return true;
    }
    return false;
}

#endif
