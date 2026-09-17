
//////////////////////////////////////////////////////////////////////
//
// Filename    : CGRequestGuildList.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_REQUER_GUILD_LIST_H__
#define __CG_REQUER_GUILD_LIST_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

typedef BYTE GuildType_t;


//////////////////////////////////////////////////////////////////////
//
// class CGRequestGuildList;
//
//////////////////////////////////////////////////////////////////////

class CGRequestGuildList : public Packet {
public:
    enum {
        GUILDTYPE_WAIT,   // Guild waiting to be registered.
        GUILDTYPE_NORMAL, // Registered guild (an ordinary guild).
        GUILDTYPE_MAX
    };

    CGRequestGuildList(){};
    virtual ~CGRequestGuildList(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_REQUEST_GUILD_LIST;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return sizeof(GuildType_t);
    }

    // get packet name
    string getPacketName() const {
        return "CGRequestGuildList";
    }

    // get packet's debug string
    string toString() const;

    // get/set GuildType
    GuildType_t getGuildType() const {
        return m_GuildType;
    }
    void setGuildType(GuildType_t GuildType) {
        m_GuildType = GuildType;
    }

    GuildType_t m_GuildType;
};


//////////////////////////////////////////////////////////////////////
//
// class CGRequestGuildListFactory;
//
// Factory for CGRequestGuildList
//
//////////////////////////////////////////////////////////////////////

class CGRequestGuildListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_REQUEST_GUILD_LIST;
    static constexpr std::string_view kName = "CGRequestGuildList";
    static constexpr PacketSize_t kMaxSize{sizeof(GuildType_t)};

    // constructor
    CGRequestGuildListFactory() {}

    // destructor
    virtual ~CGRequestGuildListFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGRequestGuildList();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGRequestGuildListHandler;
//
//////////////////////////////////////////////////////////////////////

class CGRequestGuildListHandler {
public:
    // execute packet's handler
    static void execute(CGRequestGuildList* pCGRequestGuildList, Player* pPlayer);
};

#endif
