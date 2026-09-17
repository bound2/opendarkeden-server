//----------------------------------------------------------------------
//
// Filename    : SGAddGuildOK.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __SG_ADD_GUILD_OK_H__
#define __SG_ADD_GUILD_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class SGAddGuildOK;
//
// Tell the game server that a team has been added.
//
// *CAUTION*
//
//----------------------------------------------------------------------

class SGAddGuildOK : public Packet {
public:
    SGAddGuildOK(){};
    ~SGAddGuildOK(){};
    // Read data from the Datagram object and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the Datagram object.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_SG_ADD_GUILD_OK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID +            // guild ID
               szBYTE +               // guild name length
               m_GuildName.size() +   // guild name size
               szGuildRace +          // guild race size
               szGuildState +         // gulld state
               szServerGroupID +      // server group ID
               szZoneID +             // guild zone ID
               szBYTE +               // guild master length
               m_GuildMaster.size() + // guild master
               szBYTE +               // guild intro length
               m_GuildIntro.size();   // guild intro
    }

    // get packet name
    string getPacketName() const {
        return "SGAddGuildOK";
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
    const string& getGuildName() const {
        return m_GuildName;
    }
    void setGuildName(const string& name) {
        m_GuildName = name;
    }

    // get/set guild race
    GuildRace_t getGuildRace() const {
        return m_GuildRace;
    }
    void setGuildRace(GuildRace_t guildRace) {
        m_GuildRace = guildRace;
    }

    // get/set guild state
    GuildState_t getGuildState() const {
        return m_GuildState;
    }
    void setGuildState(GuildState_t guildState) {
        m_GuildState = guildState;
    }

    // get/set server group ID
    ServerGroupID_t getServerGroupID() const {
        return m_ServerGroupID;
    }
    void setServerGroupID(ServerGroupID_t serverGroupID) {
        m_ServerGroupID = serverGroupID;
    }

    // get/set guild zone ID
    ZoneID_t getGuildZoneID() const {
        return m_GuildZoneID;
    }
    void setGuildZoneID(ZoneID_t guildZoneID) {
        m_GuildZoneID = guildZoneID;
    }

    // get/set guild master
    const string& getGuildMaster() const {
        return m_GuildMaster;
    }
    void setGuildMaster(const string& master) {
        m_GuildMaster = master;
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

    // guild name
    string m_GuildName;

    // guild race
    GuildRace_t m_GuildRace;

    // guild state
    GuildState_t m_GuildState;

    // server group ID
    ServerGroupID_t m_ServerGroupID;

    // guild zone ID
    ZoneID_t m_GuildZoneID;

    // guild master
    string m_GuildMaster;

    // guild intro
    string m_GuildIntro;
};


//////////////////////////////////////////////////////////////////////
//
// class SGAddGuildOKFactory;
//
// Factory for SGAddGuildOK
//
//////////////////////////////////////////////////////////////////////

class SGAddGuildOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_SG_ADD_GUILD_OK;
    static constexpr std::string_view kName = "SGAddGuildOK";
    static constexpr PacketSize_t kMaxSize{szGuildID +       // guild ID
                                           szBYTE +          // guild name length
                                           30 +              // guild name max size
                                           szGuildRace +     // guild race size
                                           szGuildState +    // gulld state
                                           szServerGroupID + // server group ID
                                           szZoneID +        // guild zone ID
                                           szBYTE +          // guild master length
                                           20 +              // guild master max size
                                           szBYTE +          // guild intro length
                                           256};             // guild intro max size

    // create packet
    Packet* createPacket() override {
        return new SGAddGuildOK();
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

class SGAddGuildOKHandler {
public:
    // execute packet's handler
    static void execute(SGAddGuildOK* pPacket);
};

#endif
