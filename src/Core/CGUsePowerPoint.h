//////////////////////////////////////////////////////////////////////////////
// Filename    : CGUsePowerPoint.h
// Written By  : bezz
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_USE_POWER_POINT_H__
#define __CG_USE_POWER_POINT_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGUsePowerPoint;
//////////////////////////////////////////////////////////////////////////////

class CGUsePowerPoint : public Packet {
public:
    CGUsePowerPoint();
    ~CGUsePowerPoint();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_USE_POWER_POINT;
    }
    PacketSize_t getPacketSize() const {
        return 0;
    }
    string getPacketName() const {
        return "CGUsePowerPoint";
    }
    string toString() const;
};

//////////////////////////////////////////////////////////////////////////////
// class CGUsePowerPointFactory;
//////////////////////////////////////////////////////////////////////////////

class CGUsePowerPointFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_USE_POWER_POINT;
    static constexpr std::string_view kName = "CGUsePowerPoint";
    static constexpr PacketSize_t kMaxSize{0};

    Packet* createPacket() override {
        return new CGUsePowerPoint();
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
// class CGUsePowerPointHandler;
//////////////////////////////////////////////////////////////////////////////

class CGUsePowerPointHandler {
public:
    static void execute(CGUsePowerPoint* pCGUsePowerPoint, Player* pPlayer);
};


#endif
