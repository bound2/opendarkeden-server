//////////////////////////////////////////////////////////////////////
//
// Filename    : ServerGroupInfo.h
// Written By  : elca@ewestsoft.com
// Description : Server group information
//
//////////////////////////////////////////////////////////////////////

#ifndef __SERVER_GROUP_INFO_H__
#define __SERVER_GROUP_INFO_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class ServerGroupInfo;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class ServerGroupInfo {
public:
    // constructor
    ServerGroupInfo();

    // destructor
    ~ServerGroupInfo() noexcept;

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
    // full-width name; LCServerList refuses one more.
    static constexpr uint kMaxCount = 37;

    static constexpr uint getMaxSize() {
        return (szServerGroupID + szBYTE + maxNameLength + szBYTE) * kMaxCount;
    }

    // get packet's debug string
    string toString() const;

    // get / set GroupID
    BYTE getGroupID() const {
        return m_GroupID;
    }
    void setGroupID(ServerGroupID_t GroupID) {
        m_GroupID = GroupID;
    }

    // get / set GroupName
    string getGroupName() const {
        return m_GroupName;
    }
    // Truncates to the width the length prefix and the factory max allow.
    void setGroupName(string GroupName) {
        m_GroupName = (GroupName.size() > maxNameLength) ? GroupName.substr(0, maxNameLength) : GroupName;
    }

    // get / set Group Stat
    BYTE getStat() const {
        return m_Stat;
    }
    void setStat(BYTE Stat) {
        m_Stat = Stat;
    }

private:
    // Group id
    ServerGroupID_t m_GroupID;

    // Group name
    string m_GroupName;

    // Group state
    BYTE m_Stat;
};

#endif
