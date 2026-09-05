//////////////////////////////////////////////////////////////////////////////
// Filename    : CGAddMouseToZone.h
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_ADD_MOUSE_TO_ZONE_H__
#define __CG_ADD_MOUSE_TO_ZONE_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGAddMouseToZone;
//////////////////////////////////////////////////////////////////////////////

class CGAddMouseToZone : public Packet {
public:
    CGAddMouseToZone();
    ~CGAddMouseToZone();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_ADD_MOUSE_TO_ZONE;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }
    string getPacketName() const {
        return "CGAddMouseToZone";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

private:
    ObjectID_t m_ObjectID;
};

//////////////////////////////////////////////////////////////////////////////
// class CGAddMouseToZoneFactory;
//////////////////////////////////////////////////////////////////////////////

class CGAddMouseToZoneFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_ADD_MOUSE_TO_ZONE;
    static constexpr std::string_view kName = "CGAddMouseToZone";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    Packet* createPacket() override {
        return new CGAddMouseToZone();
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
// class CGAddMouseToZoneHandler;
//////////////////////////////////////////////////////////////////////////////

class CGAddMouseToZoneHandler {
public:
    static void execute(CGAddMouseToZone* pPacket, Player* player);
};

#endif
