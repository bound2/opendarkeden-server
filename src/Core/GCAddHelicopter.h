//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddHelicopter.h
// Written By  : excel96
// Description :
// Packet sent when a slayer uses a radio to call a helicopter
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_HELICOPTER_H__
#define __GC_ADD_HELICOPTER_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCAddHelicopter;
//////////////////////////////////////////////////////////////////////////////

class GCAddHelicopter : public Packet {
public:
    GCAddHelicopter(){};
    ~GCAddHelicopter(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_HELICOPTER;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE;
    }
    string getPacketName() const {
        return "GCAddHelicopter";
    }
    string toString() const;

public:
    ObjectID_t getObjectID(void) const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t d) {
        m_ObjectID = d;
    }

    BYTE getCode(void) const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

private:
    ObjectID_t m_ObjectID = 0; // Object ID of the slayer who called the helicopter
    BYTE m_Code = 0;           // Code for the helicopter summon (0: call, 1: remove)
};

//////////////////////////////////////////////////////////////////////////////
// class GCAddHelicopterFactory;
//////////////////////////////////////////////////////////////////////////////

class GCAddHelicopterFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_HELICOPTER;
    static constexpr std::string_view kName = "GCAddHelicopter";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE};

    Packet* createPacket() override {
        return new GCAddHelicopter();
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

#endif
