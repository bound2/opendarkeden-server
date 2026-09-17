//----------------------------------------------------------------------
//
// Filename    : SGQuitGuildOK.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __SG_QUIT_GUILD_OK_H__
#define __SG_QUIT_GUILD_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class SGQuitGuildOK;
//
// Tell the game server that a team has been added.
//
// *CAUTION*
//
//----------------------------------------------------------------------

class SGQuitGuildOK : public Packet {
public:
    SGQuitGuildOK(){};
    ~SGQuitGuildOK(){};
    // Read data from the Datagram object and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the Datagram object.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_SG_QUIT_GUILD_OK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID +    // guild ID
               szBYTE +       // name length
               m_Name.size(); // name size
    }

    // get packet name
    string getPacketName() const {
        return "SGQuitGuildOK";
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

private:
    // GuildID
    GuildID_t m_GuildID;

    // name
    string m_Name;
};


//////////////////////////////////////////////////////////////////////
//
// class SGQuitGuildOKFactory;
//
// Factory for SGQuitGuildOK
//
//////////////////////////////////////////////////////////////////////

class SGQuitGuildOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_SG_QUIT_GUILD_OK;
    static constexpr std::string_view kName = "SGQuitGuildOK";
    static constexpr PacketSize_t kMaxSize{szGuildID + // guild ID
                                           szBYTE +    // name length
                                           20};        // name max size

    // create packet
    Packet* createPacket() override {
        return new SGQuitGuildOK();
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

class SGQuitGuildOKHandler {
public:
    // execute packet's handler
    static void execute(SGQuitGuildOK* pPacket);
};

#endif
