//////////////////////////////////////////////////////////////////////
//
// Filename    : ExtraInfo.h
// Written By  : elca@ewestsoft.com
// Description : Information about the inventory items
//
//////////////////////////////////////////////////////////////////////

#ifndef __EXTRA_INFO_H__
#define __EXTRA_INFO_H__

// include files
#include "Exception.h"
#include "ExtraSlotInfo.h"
#include "Packet.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class ExtraInfo;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class ExtraInfo {
public:
    // constructor
    ExtraInfo();

    // destructor
    ~ExtraInfo() noexcept;

public:
    // Read data from the input stream (buffer) and initialise
    // the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getSize();

    static constexpr uint getMaxSize() {
        return szBYTE + (ExtraSlotInfo::getMaxSize() * 1);
    }

    // get packet's debug string
    string toString() const;

    // get ListNumber
    BYTE getListNum() const {
        return m_ListNum;
    }

    // add / delete / clear S List
    void addListElement(ExtraSlotInfo* pExtraSlotInfo) {
        m_ExtraSlotInfoList.push_back(pExtraSlotInfo);
        m_ListNum++;
    }

    // ClearList
    void clearList() {
        m_ExtraSlotInfoList.clear();
        m_ListNum = 0;
    }

    // pop front Element in Status List
    ExtraSlotInfo* popFrontListElement() {
        ExtraSlotInfo* TempExtraSlotInfo = m_ExtraSlotInfoList.front();
        m_ExtraSlotInfoList.pop_front();
        return TempExtraSlotInfo;
    }

private:
    // ExtraSlotInfo List Total Number
    BYTE m_ListNum;

    // ExtraSlotInfo List
    list<ExtraSlotInfo*> m_ExtraSlotInfoList;
};

#endif
