//----------------------------------------------------------------------
//
// Filename    : GSQuitGuild.h
// Written By  :
// Description :
//
//----------------------------------------------------------------------

#ifndef __GS_QUIT_GUILD_H__
#define __GS_QUIT_GUILD_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GSQuitGuild;
//
// Ask the shared server to add a team.
//
// *CAUTION*
//
//----------------------------------------------------------------------

class GSQuitGuild : public Packet {
public:
    GSQuitGuild(){};
    ~GSQuitGuild(){};
    // Read data from the Datagram object and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the Datagram object.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GS_QUIT_GUILD;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID +    // Guild ID
               szBYTE +       // name length
               m_Name.size(); // name
    }

    // get packet name
    string getPacketName() const {
        return "GSQuitGuild";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set Guild ID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t guildID) {
        m_GuildID = guildID;
    }

    // get/set Name
    const string& getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

private:
    // Guild ID
    GuildID_t m_GuildID;

    // name
    string m_Name;
};


//////////////////////////////////////////////////////////////////////
//
// class GSQuitGuildFactory;
//
// Factory for GSQuitGuild
//
//////////////////////////////////////////////////////////////////////

class GSQuitGuildFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GS_QUIT_GUILD;
    static constexpr std::string_view kName = "GSQuitGuild";
    static constexpr PacketSize_t kMaxSize{szGuildID + // guild ID
                                           szBYTE +    // name length
                                           20};        // name max length

    // create packet
    Packet* createPacket() override {
        return new GSQuitGuild();
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

class GSQuitGuildHandler {
public:
    // execute packet's handler
    static void execute(GSQuitGuild* pPacket, Player* pPlayer);
};

#endif
