//////////////////////////////////////////////////////////////////////////////
// Filename    : BloodBibleSignInfo.h
// Written By  : elca@ewestsoft.com
// Description :
// The list of information about the gear items being worn.
// For the information about each gear item see BloodBibleSignSlotInfo.
//////////////////////////////////////////////////////////////////////////////

#ifndef __BLOOD_BIBLE_SIGN_INFO_H__
#define __BLOOD_BIBLE_SIGN_INFO_H__

#include <vector>

#include "Exception.h"
#include "Packet.h"
#include "Types.h"

#define BLOOD_BIBLE_SIGN_SLOT_NUM 6

//////////////////////////////////////////////////////////////////////////////
// class BloodBibleSignInfo;
//////////////////////////////////////////////////////////////////////////////

class BloodBibleSignInfo {
public:
    BloodBibleSignInfo();
    ~BloodBibleSignInfo();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketSize_t getSize() const {
        return szuint + szBYTE + szItemType * signCount();
    }

    // write() emits at most this many signs, so the declared size counts
    // at most this many.
    BYTE signCount() const {
        return (m_SignList.size() > BLOOD_BIBLE_SIGN_SLOT_NUM) ? (BYTE)BLOOD_BIBLE_SIGN_SLOT_NUM
                                                               : (BYTE)m_SignList.size();
    }
    static constexpr uint getMaxSize() {
        return szuint + szBYTE + szItemType * BLOOD_BIBLE_SIGN_SLOT_NUM;
    }

public:
    uint getOpenNum() const {
        return m_OpenNum;
    }
    void setOpenNum(uint num) {
        m_OpenNum = num;
    }

    vector<ItemType_t>& getList() {
        return m_SignList;
    }
    const vector<ItemType_t>& getList() const {
        return m_SignList;
    }

private:
    uint m_OpenNum;
    vector<ItemType_t> m_SignList;
};

#endif
