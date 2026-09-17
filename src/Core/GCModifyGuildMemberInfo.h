//////////////////////////////////////////////////////////////////////
//
// Filename    : GCModifyGuildMemberInfo.h
// Written By  : Reiot
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_MODIFY_GUILD_MEMBER_INFO_H__
#define __GC_MODIFY_GUILD_MEMBER_INFO_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCModifyGuildMemberInfo;
//
// Packet the game server sends when broadcasting a particular player's
// ModifyGuildMemberInfo to the other players. It holds the character name and that string
// as data fields.
//
//////////////////////////////////////////////////////////////////////

class GCModifyGuildMemberInfo : public Packet {
public:
    GCModifyGuildMemberInfo(){};
    ~GCModifyGuildMemberInfo(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MODIFY_GUILD_MEMBER_INFO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID + szBYTE + m_GuildName.size() + szGuildMemberRank;
    }

    // get packet name
    string getPacketName() const {
        return "GCModifyGuildMemberInfo";
    }

    // get packet's debug string
    string toString() const;

    // get/set Guild ID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t guildID) {
        m_GuildID = guildID;
    }

    // get/set Guild Name
    string getGuildName() const {
        return m_GuildName;
    }
    void setGuildName(const string& guildName) {
        m_GuildName = guildName;
    }

    // get/set Guild ID
    GuildMemberRank_t getGuildMemberRank() const {
        return m_GuildMemberRank;
    }
    void setGuildMemberRank(GuildMemberRank_t guildMemberRank) {
        m_GuildMemberRank = guildMemberRank;
    }

private:
    // Guild ID
    GuildID_t m_GuildID;

    // Guild Name
    string m_GuildName;

    // Guild Member Rank
    GuildMemberRank_t m_GuildMemberRank;
};


//////////////////////////////////////////////////////////////////////
//
// class GCModifyGuildMemberInfoFactory;
//
// Factory for GCModifyGuildMemberInfo
//
//////////////////////////////////////////////////////////////////////

class GCModifyGuildMemberInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MODIFY_GUILD_MEMBER_INFO;
    static constexpr std::string_view kName = "GCModifyGuildMemberInfo";
    static constexpr PacketSize_t kMaxSize{szGuildID + szBYTE + 30 + szGuildMemberRank};

    // create packet
    Packet* createPacket() override {
        return new GCModifyGuildMemberInfo();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's max body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCModifyGuildMemberInfoPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
