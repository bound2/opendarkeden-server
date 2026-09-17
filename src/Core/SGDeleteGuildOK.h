//----------------------------------------------------------------------
//
// Filename    : SGDeleteGuildOK.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __SG_DELETE_GUILD_OK_H__
#define __SG_DELETE_GUILD_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class SGDeleteGuildOK;
//
// Tell the game server that a team has been added.
//
// *CAUTION*
//
//----------------------------------------------------------------------

class SGDeleteGuildOK : public Packet {
public:
    SGDeleteGuildOK(){};
    ~SGDeleteGuildOK(){};
    // Read data from the Datagram object and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the Datagram object.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_SG_DELETE_GUILD_OK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID; // guild ID
    }

    // get packet name
    string getPacketName() const {
        return "SGDeleteGuildOK";
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

private:
    // GuildID
    GuildID_t m_GuildID;
};


//////////////////////////////////////////////////////////////////////
//
// class SGDeleteGuildOKFactory;
//
// Factory for SGDeleteGuildOK
//
//////////////////////////////////////////////////////////////////////

class SGDeleteGuildOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_SG_DELETE_GUILD_OK;
    static constexpr std::string_view kName = "SGDeleteGuildOK";
    static constexpr PacketSize_t kMaxSize{szGuildID}; // guild ID

    // create packet
    Packet* createPacket() override {
        return new SGDeleteGuildOK();
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

class SGDeleteGuildOKHandler {
public:
    // execute packet's handler
    static void execute(SGDeleteGuildOK* pPacket);
};

#endif
