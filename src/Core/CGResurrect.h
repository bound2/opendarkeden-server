//////////////////////////////////////////////////////////////////////////////
// Filename    : CGResurrect.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_RESURRECT_H__
#define __CG_RESURRECT_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGResurrect;
//////////////////////////////////////////////////////////////////////////////

class CGResurrect : public Packet {
public:
    CGResurrect(){};
    virtual ~CGResurrect(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_RESURRECT;
    }
    PacketSize_t getPacketSize() const {
        return 0;
    }
    string getPacketName() const {
        return "CGResurrect";
    }
    string toString() const;
};

//////////////////////////////////////////////////////////////////////////////
// class CGResurrectFactory;
//////////////////////////////////////////////////////////////////////////////

class CGResurrectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_RESURRECT;
    static constexpr std::string_view kName = "CGResurrect";
    static constexpr PacketSize_t kMaxSize{0};

    Packet* createPacket() override {
        return new CGResurrect();
    }
    string getPacketName() const override {
        return string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};

//////////////////////////////////////////////////////////////////////////////
// class CGResurrectHandler;
//////////////////////////////////////////////////////////////////////////////

class CGResurrectHandler {
public:
    static void execute(CGResurrect* pPacket, Player* player);
};

#endif
