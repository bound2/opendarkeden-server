//----------------------------------------------------------------------
//
// Filename    : SGAddGuildMemberOK.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __SG_ADD_GUILD_MEMBER_OK_H__
#define __SG_ADD_GUILD_MEMBER_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class SGAddGuildMemberOK;
//
// Tell the game server that a member has been added.
//
// *CAUTION*
//
//----------------------------------------------------------------------

class SGAddGuildMemberOK : public Packet {
public:
    SGAddGuildMemberOK(){};
    ~SGAddGuildMemberOK(){};
    // Read data from the Datagram object and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the Datagram object.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_SG_ADD_GUILD_MEMBER_OK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID +         // guild ID
               szBYTE +            // name length
               m_Name.size() +     // name size
               szGuildMemberRank + // Guild Member Rank size
               szServerGroupID;    // server group ID
    }

    // get packet name
    string getPacketName() const {
        return "SGAddGuildMemberOK";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set guildID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t guildID) {
        m_GuildID = guildID;
    }

    // get/set name
    const string& getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    // get/set Guild Member Rank
    GuildMemberRank_t getGuildMemberRank() const {
        return m_GuildMemberRank;
    }
    void setGuildMemberRank(GuildMemberRank_t GuildMemberRank) {
        m_GuildMemberRank = GuildMemberRank;
    }

    // get/set server group ID
    ServerGroupID_t getServerGroupID() const {
        return m_ServerGroupID;
    }
    void setServerGroupID(ServerGroupID_t serverGroupID) {
        m_ServerGroupID = serverGroupID;
    }

private:
    // Guild ID
    GuildID_t m_GuildID;

    // Name
    string m_Name;

    // Guild Member Rank
    GuildMemberRank_t m_GuildMemberRank;

    // Server Group ID
    ServerGroupID_t m_ServerGroupID;
};


//////////////////////////////////////////////////////////////////////
//
// class SGAddGuildMemberOKFactory;
//
// Factory for SGAddGuildMemberOK
//
//////////////////////////////////////////////////////////////////////

class SGAddGuildMemberOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_SG_ADD_GUILD_MEMBER_OK;
    static constexpr std::string_view kName = "SGAddGuildMemberOK";
    static constexpr PacketSize_t kMaxSize{szGuildID +         // guild ID
                                           szBYTE +            // name length
                                           20 +                // name max size
                                           szGuildMemberRank + // Guild Member Rank size
                                           szServerGroupID};   // server group ID

    // create packet
    Packet* createPacket() override {
        return new SGAddGuildMemberOK();
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
    // Define and return const static LGIncomingConnectionPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class LGIncomingConnectionHandler;
//
//////////////////////////////////////////////////////////////////////

class SGAddGuildMemberOKHandler {
public:
    // execute packet's handler
    static void execute(SGAddGuildMemberOK* pPacket);
};

#endif
