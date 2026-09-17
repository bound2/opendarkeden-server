//////////////////////////////////////////////////////////////////////
//
// Filename    : GuildInfo.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GUILD_INFO_H__
#define __GUILD_INFO_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GuildInfo;
//
// Send the guild list to the client.
//
//////////////////////////////////////////////////////////////////////

class GuildInfo {
public:
    // constructor
    GuildInfo();

    // destructor
    ~GuildInfo() noexcept;

public:
    // Read data from the input stream (buffer) and initialise the
    // packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getSize();

    // The guild-table packets' factory maxima budget this many guilds;
    // GCActiveGuildList and GCWaitGuildList refuse one more.
    static constexpr uint kMaxCount = 5000;

    static constexpr uint getMaxSize() {
        //		return ( szGuildID + szBYTE + 30 + szBYTE + 20 + szBYTE + szBYTE + 11 ) * 256 + szBYTE;
        return szGuildID + szBYTE + 30 + szBYTE + 20 + szBYTE + szBYTE + 11;
    }

    // get packet's debug string
    string toString() const;

    // get/set GuildID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t GuildID) {
        m_GuildID = GuildID;
    }

    // get/set Guild Name
    string getGuildName() const {
        return m_GuildName;
    }
    void setGuildName(const string& GuildName) {
        m_GuildName = GuildName;
    }

    // get/set Guild Master
    string getGuildMaster() const {
        return m_GuildMaster;
    }
    void setGuildMaster(const string& GuildMaster) {
        m_GuildMaster = GuildMaster;
    }

    // get/set Guild Member Count
    BYTE getGuildMemberCount() const {
        return m_GuildMemberCount;
    }
    void setGuildMemberCount(BYTE GuildMemberCount) {
        m_GuildMemberCount = GuildMemberCount;
    }

    // get/set Guild Expire Date
    string getGuildExpireDate() const {
        return m_GuildExpireDate;
    }
    void setGuildExpireDate(const string& GuildExpireDate) {
        m_GuildExpireDate = GuildExpireDate;
    }


private:
    // Guild ID
    GuildID_t m_GuildID;

    // Guild name
    string m_GuildName;

    // Guild master
    string m_GuildMaster;

    // Guild member count
    BYTE m_GuildMemberCount;

    // Guild expire date
    string m_GuildExpireDate;
};

#endif
