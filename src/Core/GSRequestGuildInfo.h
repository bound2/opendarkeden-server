//----------------------------------------------------------------------
//
// Filename    : GSRequestGuildInfo.h
// Written By  :
// Description :
//
//----------------------------------------------------------------------

#ifndef __GS_REQUEST_GUILD_INFO_H__
#define __GS_REQUEST_GUILD_INFO_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GSRequestGuildInfo;
//
// Ask the shared server to add a team.
//
// *CAUTION*
//
//----------------------------------------------------------------------

class GSRequestGuildInfo : public Packet {
public:
    GSRequestGuildInfo(){};
    ~GSRequestGuildInfo(){};
    // Read data from the Stream object and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the Stream object.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GS_REQUEST_GUILD_INFO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet name
    string getPacketName() const {
        return "GSRequestGuildInfo";
    }

    // get packet's debug string
    string toString() const {
        return "GSRequestGuildInfo";
    }
};


//////////////////////////////////////////////////////////////////////
//
// class GSRequestGuildInfoFactory;
//
// Factory for GSRequestGuildInfo
//
//////////////////////////////////////////////////////////////////////

class GSRequestGuildInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GS_REQUEST_GUILD_INFO;
    static constexpr std::string_view kName = "GSRequestGuildInfo";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new GSRequestGuildInfo();
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

class GSRequestGuildInfoHandler {
public:
    // execute packet's handler
    static void execute(GSRequestGuildInfo* pPacket, Player* pPlayer);
};

#endif
