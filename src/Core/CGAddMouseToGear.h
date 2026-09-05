//////////////////////////////////////////////////////////////////////////////
// Filename    : CGAddMouseToGear.h
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_ADD_MOUSE_TO_GEAR_H__
#define __CG_ADD_MOUSE_TO_GEAR_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGAddMouseToGear;
//////////////////////////////////////////////////////////////////////////////

class CGAddMouseToGear : public Packet {
public:
    CGAddMouseToGear();
    ~CGAddMouseToGear();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_ADD_MOUSE_TO_GEAR;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szSlotID;
    }
    string getPacketName() const {
        return "CGAddMouseToGear";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    SlotID_t getSlotID() const {
        return m_SlotID;
    }
    void setSlotID(SlotID_t SlotID) {
        m_SlotID = SlotID;
    }

private:
    ObjectID_t m_ObjectID;
    SlotID_t m_SlotID;
};

//////////////////////////////////////////////////////////////////////////////
// class CGAddMouseToGearFactory;
//////////////////////////////////////////////////////////////////////////////

class CGAddMouseToGearFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_ADD_MOUSE_TO_GEAR;
    static constexpr std::string_view kName = "CGAddMouseToGear";
    static constexpr PacketSize_t kMaxSize{szObjectID + szSlotID};

    Packet* createPacket() override {
        return new CGAddMouseToGear();
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
// class CGAddMouseToGearHandler;
//////////////////////////////////////////////////////////////////////////////

class CGAddMouseToGearHandler {
public:
    static void execute(CGAddMouseToGear* pPacket, Player* player);
};

#endif
