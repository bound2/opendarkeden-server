//////////////////////////////////////////////////////////////////////
//
// Filename    : CGAcceptUnion.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_ACCEPT_UNION_H__
#define __CG_ACCEPT_UNION_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGAcceptUnion;
//
//////////////////////////////////////////////////////////////////////

class CGAcceptUnion : public Packet {
public:
    CGAcceptUnion(){};
    ~CGAcceptUnion(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_ACCEPT_UNION;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID;
    }

    // get packet name
    string getPacketName() const {
        return "CGAcceptUnion";
    }

    // get packet's debug string
    string toString() const;

    // get/set GuildID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t GuildID) {
        m_GuildID = GuildID;
    }


private:
    // Guild ID
    GuildID_t m_GuildID = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class CGAcceptUnionFactory;
//
// Factory for CGAcceptUnion
//
//////////////////////////////////////////////////////////////////////

class CGAcceptUnionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_ACCEPT_UNION;
    static constexpr std::string_view kName = "CGAcceptUnion";
    static constexpr PacketSize_t kMaxSize{szGuildID};

    // constructor
    CGAcceptUnionFactory() {}

    // destructor
    virtual ~CGAcceptUnionFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGAcceptUnion();
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
// class CGAcceptUnionHandler;
//
//////////////////////////////////////////////////////////////////////

class CGAcceptUnionHandler {
public:
    // execute packet's handler
    static void execute(CGAcceptUnion* pCGAcceptUnion, Player* pPlayer);
};

#endif
