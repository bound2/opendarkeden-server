//////////////////////////////////////////////////////////////////////
//
// Filename    : ModifyItemInfo.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __MODIFY_ITEM_INFO_H__
#define __MODIFY_ITEM_INFO_H__

// include files
#include "Exception.h"
#include "ModifyInfo.h"
#include "Packet.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class ModifyItemInfo;
//
// Object the game server uses to tell the client about its own changed
// data. It is carried in ModifyItemInformation, SkillToObjectOK and
// the like.
//
//////////////////////////////////////////////////////////////////////

class ModifyItemInfo {
public:
    // constructor
    ModifyItemInfo();

    // destructor
    ~ModifyItemInfo() noexcept;

public:
    // Read data from the input stream (buffer) and initialise the
    // packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getSize() const {
        return szDWORD + szDWORD * m_ListNum * 3;
    }

    // get packet's debug string
    string toString() const;

    // get / set ListNumber
    BYTE getListNum() const {
        return m_ListNum;
    }
    void setListNum(BYTE ListNum) {
        m_ListNum = ListNum;
    }

    // add / delete / clear S List
    void addListElement(ObjectID_t ObjectID, ModifyType List, DWORD Value);

    // ClearList
    void clearList() {
        m_SList.clear();
        m_ListNum = 0;
    }

    // pop front Element in Status List
    DWORD popFrontListElement() {
        DWORD StatusList = m_SList.front();
        m_SList.pop_front();
        return StatusList;
    }

private:
    // StatusList Element Number
    BYTE m_ListNum;

    // Status List
    list<DWORD> m_SList;
};

#endif
