//////////////////////////////////////////////////////////////////////
//
// Filename    : CGDenyUnion.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_DENY_UNION_H__
#define __CG_DENY_UNION_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGDenyUnion;
//
//////////////////////////////////////////////////////////////////////

class CGDenyUnion : public Packet {
public:
    CGDenyUnion(){};
    ~CGDenyUnion(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_DENY_UNION;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID;
    }

    // get packet name
    string getPacketName() const {
        return "CGDenyUnion";
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
// class CGDenyUnionFactory;
//
// Factory for CGDenyUnion
//
//////////////////////////////////////////////////////////////////////

class CGDenyUnionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_DENY_UNION;
    static constexpr std::string_view kName = "CGDenyUnion";
    static constexpr PacketSize_t kMaxSize{szGuildID};

    // constructor
    CGDenyUnionFactory() {}

    // destructor
    virtual ~CGDenyUnionFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGDenyUnion();
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
// class CGDenyUnionHandler;
//
//////////////////////////////////////////////////////////////////////

class CGDenyUnionHandler {
public:
    // execute packet's handler
    static void execute(CGDenyUnion* pCGDenyUnion, Player* pPlayer);
};

#endif
