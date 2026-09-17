//////////////////////////////////////////////////////////////////////
//
// Filename    : GCOtherGuildName.h
// Written By  : Reiot
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_OTHER_GUILD_NAME_H__
#define __GC_OTHER_GUILD_NAME_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCOtherGuildName;
//
// Packet the game server sends when broadcasting a particular player's
// OtherGuildName to the other players. It holds the character name and that string
// as data fields.
//
//////////////////////////////////////////////////////////////////////

class GCOtherGuildName : public Packet {
public:
    GCOtherGuildName(){};
    ~GCOtherGuildName(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_OTHER_GUILD_NAME;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID + szGuildID + szBYTE + m_GuildName.size();
    }

    // get packet name
    string getPacketName() const {
        return "GCOtherGuildName";
    }

    // get packet's debug string
    string toString() const;

    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t objectID) {
        m_ObjectID = objectID;
    }

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

private:
    ObjectID_t m_ObjectID;

    // Guild ID
    GuildID_t m_GuildID;

    // Guild Name
    string m_GuildName;
};


//////////////////////////////////////////////////////////////////////
//
// class GCOtherGuildNameFactory;
//
// Factory for GCOtherGuildName
//
//////////////////////////////////////////////////////////////////////

class GCOtherGuildNameFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_OTHER_GUILD_NAME;
    static constexpr std::string_view kName = "GCOtherGuildName";
    static constexpr PacketSize_t kMaxSize{szObjectID + szGuildID + szBYTE + 30};

    // create packet
    Packet* createPacket() override {
        return new GCOtherGuildName();
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
    // Define and return const static GCOtherGuildNamePacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
