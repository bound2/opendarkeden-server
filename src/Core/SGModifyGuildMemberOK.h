//----------------------------------------------------------------------
//
// Filename    : SGModifyGuildMemberOK.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __SG_MODIFY_GUILD_MEMBER_OK_H__
#define __SG_MODIFY_GUILD_MEMBER_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class SGModifyGuildMemberOK;
//
// Tell the game server that a team has been added.
//
// *CAUTION*
//
//----------------------------------------------------------------------

class SGModifyGuildMemberOK : public Packet {
public:
    SGModifyGuildMemberOK(){};
    ~SGModifyGuildMemberOK(){};
    // Read data from the Datagram object and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the Datagram object.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_SG_MODIFY_GUILD_MEMBER_OK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID +         // guild ID
               szBYTE +            // name length
               m_Name.size() +     // name size
               szGuildMemberRank + // guild member rank
               szBYTE +            // sender length
               m_Sender.size();    // sender size
    }

    // get packet name
    string getPacketName() const {
        return "SGModifyGuildMemberOK";
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

    // get/set guild name
    const string& getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    // get/set guild member rank
    GuildMemberRank_t getGuildMemberRank() const {
        return m_GuildMemberRank;
    }
    void setGuildMemberRank(GuildMemberRank_t GuildMemberRank) {
        m_GuildMemberRank = GuildMemberRank;
    }

    // get/set sender
    const string& getSender() const {
        return m_Sender;
    }
    void setSender(const string& sender) {
        m_Sender = sender;
    }

private:
    // GuildID
    GuildID_t m_GuildID;

    // name
    string m_Name;

    // guild member rank
    GuildMemberRank_t m_GuildMemberRank;

    // sender
    string m_Sender;
};


//////////////////////////////////////////////////////////////////////
//
// class SGModifyGuildMemberOKFactory;
//
// Factory for SGModifyGuildMemberOK
//
//////////////////////////////////////////////////////////////////////

class SGModifyGuildMemberOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_SG_MODIFY_GUILD_MEMBER_OK;
    static constexpr std::string_view kName = "SGModifyGuildMemberOK";
    static constexpr PacketSize_t kMaxSize{szGuildID +         // guild ID
                                           szBYTE +            // name length
                                           20 +                // name max size
                                           szGuildMemberRank + // guild member rank
                                           szBYTE +            // sender length
                                           20};                // sender max size

    // create packet
    Packet* createPacket() override {
        return new SGModifyGuildMemberOK();
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

class SGModifyGuildMemberOKHandler {
public:
    // execute packet's handler
    static void execute(SGModifyGuildMemberOK* pPacket);
};

#endif
