//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowGuildJoin.h
// Written By  :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SHOW_GUILD_JOIN_H__
#define __GC_SHOW_GUILD_JOIN_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"


//////////////////////////////////////////////////////////////////////
//
// class GCShowGuildJoin;
//
// Make the client open the guild registration window.
//
//////////////////////////////////////////////////////////////////////

class GCShowGuildJoin : public Packet {
public:
    GCShowGuildJoin(){};
    ~GCShowGuildJoin(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SHOW_GUILD_JOIN;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID + de::wire::stringWireSize(m_GuildName) + szGuildMemberRank + szGold;
    }

    // get packet name
    string getPacketName() const {
        return "GCShowGuildJoin";
    }

    // get packet's debug string
    string toString() const;

    // get/set Guild ID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t GuildID) {
        m_GuildID = GuildID;
    }

    // get/set Guild Name
    const string& getGuildName() const {
        return m_GuildName;
    }
    void setGuildName(const string& GuildName) {
        m_GuildName = GuildName;
    }

    // get/set Guild Member Rak
    GuildMemberRank_t getGuildMemberRank() const {
        return m_GuildMemberRank;
    }
    void setGuildMemberRank(GuildMemberRank_t GuildMemberRank) {
        m_GuildMemberRank = GuildMemberRank;
    }

    // get/set Join Fee
    Gold_t getJoinFee() const {
        return m_JoinFee;
    }
    void setJoinFee(Gold_t JoinFee) {
        m_JoinFee = JoinFee;
    }

private:
    // Guild ID
    GuildID_t m_GuildID;

    // Guild Name
    string m_GuildName;

    // Guild Member Rank
    GuildMemberRank_t m_GuildMemberRank;

    // Join Fee
    Gold_t m_JoinFee;
};


//////////////////////////////////////////////////////////////////////
//
// class GCShowGuildJoinFactory;
//
// Factory for GCShowGuildJoin
//
//////////////////////////////////////////////////////////////////////

class GCShowGuildJoinFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SHOW_GUILD_JOIN;
    static constexpr std::string_view kName = "GCShowGuildJoin";
    static constexpr PacketSize_t kMaxSize{szGuildID + szBYTE + 30 + szGuildMemberRank + szGold};

    // create packet
    Packet* createPacket() override {
        return new GCShowGuildJoin();
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
    // Define and return const static GCSystemMessagePacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class GCShowGuildJoin;
//
//////////////////////////////////////////////////////////////////////

#endif
