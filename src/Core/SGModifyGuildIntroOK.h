//----------------------------------------------------------------------
//
// Filename    : SGModifyGuildIntroOK.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __SG_MODIFY_GUILD_INTRO_OK_H__
#define __SG_MODIFY_GUILD_INTRO_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class SGModifyGuildIntroOK;
//
// Tell the game server that a team has been added.
//
// *CAUTION*
//
//----------------------------------------------------------------------

class SGModifyGuildIntroOK : public Packet {
public:
    SGModifyGuildIntroOK(){};
    ~SGModifyGuildIntroOK(){};
    // Read data from the Datagram object and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the Datagram object.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_SG_MODIFY_GUILD_INTRO_OK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID +          // guild ID
               szBYTE +             // Guild Intro length
               m_GuildIntro.size(); // Guild Intro
    }

    // get packet name
    string getPacketName() const {
        return "SGModifyGuildIntroOK";
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

    // get/set guild intro
    const string& getGuildIntro() const {
        return m_GuildIntro;
    }
    // Truncates to the width the length byte and the factory max allow.
    void setGuildIntro(const string& intro) {
        m_GuildIntro = (intro.size() > GUILD_INTRO_MAX_LENGTH) ? intro.substr(0, GUILD_INTRO_MAX_LENGTH) : intro;
    }

private:
    // GuildID
    GuildID_t m_GuildID;

    // Guild Intro
    string m_GuildIntro;
};


//////////////////////////////////////////////////////////////////////
//
// class SGModifyGuildIntroOKFactory;
//
// Factory for SGModifyGuildIntroOK
//
//////////////////////////////////////////////////////////////////////

class SGModifyGuildIntroOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_SG_MODIFY_GUILD_INTRO_OK;
    static constexpr std::string_view kName = "SGModifyGuildIntroOK";
    static constexpr PacketSize_t kMaxSize{szGuildID + // guild ID
                                           szBYTE +    // Guild Intro length
                                           255};       // Guild Intro max length

    // create packet
    Packet* createPacket() override {
        return new SGModifyGuildIntroOK();
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

class SGModifyGuildIntroOKHandler {
public:
    // execute packet's handler
    static void execute(SGModifyGuildIntroOK* pPacket);
};

#endif
