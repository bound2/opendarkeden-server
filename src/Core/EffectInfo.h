//////////////////////////////////////////////////////////////////////
//
// Filename    : EffectInfo.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_INFO_H__
#define __EFFECT_INFO_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class EffectInfo;
//
// Carries effect information from the game server to the client; it rides
// inside GCUpdateInfo, AddSlayer, AddVampire and AddMonster.
//
//////////////////////////////////////////////////////////////////////

class EffectInfo {
public:
    // constructor
    EffectInfo();

    // destructor
    ~EffectInfo() noexcept;

public:
    // Read data from the input stream (buffer) and initialise
    // the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getSize() const {
        return szBYTE + szWORD * m_ListNum * 2;
    }
    static constexpr PacketSize_t getMaxSize() {
        return szBYTE + szWORD * 255 * 2;
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
    void addListElement(EffectID_t EffectID, WORD Value);

    // ClearList
    void clearList() {
        m_EList.clear();
        m_ListNum = 0;
    }

    // pop front Element in Status List
    WORD popFrontListElement() {
        WORD EffectList = m_EList.front();
        m_EList.pop_front();
        return EffectList;
    }

protected:
    // StatusList Element Number
    BYTE m_ListNum;

    // Status List
    list<WORD> m_EList;
};

#endif
