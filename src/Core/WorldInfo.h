//////////////////////////////////////////////////////////////////////
//
// Filename    : WorldInfo.h
// Written By  : elca@ewestsoft.com
// Description : World information
//
//////////////////////////////////////////////////////////////////////

#ifndef __WORLD_INFO_H__
#define __WORLD_INFO_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class WorldInfo;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class WorldInfo {
public:
    // constructor
    WorldInfo();

    // destructor
    ~WorldInfo() noexcept;

public:
    // Read data from the input stream (buffer) and initialise the
    // packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getSize();

    // The list packet's factory max budgets this many entries of a
    // full-width name; LCWorldList refuses one more.
    static constexpr uint kMaxCount = 37;

    static constexpr uint getMaxSize() {
        return (szWorldID + szBYTE + maxNameLength + szBYTE) * kMaxCount;
    }

    // get packet's debug string
    string toString() const;

    // get / set ID
    BYTE getID() const {
        return m_ID;
    }
    void setID(WorldID_t ID) {
        m_ID = ID;
    }

    // get / set Name
    string getName() const {
        return m_Name;
    }
    // Truncates to the width the length prefix and the factory max allow.
    void setName(string Name) {
        m_Name = (Name.size() > maxNameLength) ? Name.substr(0, maxNameLength) : Name;
    }

    // get / set  Stat
    BYTE getStat() const {
        return m_Stat;
    }
    void setStat(BYTE Stat) {
        m_Stat = Stat;
    }

private:
    // Group id
    WorldID_t m_ID;

    // Group name
    string m_Name;

    // Group state
    BYTE m_Stat;
};

#endif
