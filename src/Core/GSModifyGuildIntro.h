//----------------------------------------------------------------------
//
// Filename    : GSModifyGuildIntro.h
// Written By  :
// Description :
//
//----------------------------------------------------------------------

#ifndef __GS_MODIFY_GUILD_INTRO_H__
#define __GS_MODIFY_GUILD_INTRO_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GSModifyGuildIntro;
//
// Ask the shared server to add a team.
//
// *CAUTION*
//
//----------------------------------------------------------------------

class GSModifyGuildIntro : public Packet {
public:
    GSModifyGuildIntro(){};
    ~GSModifyGuildIntro(){};
    // Read data from the Datagram object and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the Datagram object.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GS_MODIFY_GUILD_INTRO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID +          // Guild ID
               szBYTE +             // Guild Intro length
               m_GuildIntro.size(); // Guild Intro
    }

    // get packet name
    string getPacketName() const {
        return "GSModifyGuildIntro";
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

    // get/set Guild Intro
    const string& getGuildIntro() const {
        return m_GuildIntro;
    }
    // Truncates to the width the length byte and the factory max allow.
    void setGuildIntro(const string& intro) {
        m_GuildIntro = (intro.size() > GUILD_INTRO_MAX_LENGTH) ? intro.substr(0, GUILD_INTRO_MAX_LENGTH) : intro;
    }

private:
    // Guild ID
    GuildID_t m_GuildID;

    // Guild Intro
    string m_GuildIntro;
};


//////////////////////////////////////////////////////////////////////
//
// class GSModifyGuildIntroFactory;
//
// Factory for GSModifyGuildIntro
//
//////////////////////////////////////////////////////////////////////

class GSModifyGuildIntroFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GS_MODIFY_GUILD_INTRO;
    static constexpr std::string_view kName = "GSModifyGuildIntro";
    static constexpr PacketSize_t kMaxSize{szGuildID + // guild ID
                                           szBYTE +    // Guild Intro length
                                           255};       // Guild Intro max length

    // create packet
    Packet* createPacket() override {
        return new GSModifyGuildIntro();
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

class GSModifyGuildIntroHandler {
public:
    // execute packet's handler
    static void execute(GSModifyGuildIntro* pPacket, Player* pPlayer);
};

#endif
