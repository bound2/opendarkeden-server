//////////////////////////////////////////////////////////////////////
//
// Filename    : GCCrossCounterOK3.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_CROSS_COUNTER_OK_3_H__
#define __GC_CROSS_COUNTER_OK_3_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCCrossCounterOK3;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCCrossCounterOK3 : public Packet {
public:
    // constructor
    GCCrossCounterOK3();

    // destructor
    ~GCCrossCounterOK3();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_CROSS_COUNTER_OK_3;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szObjectID + szObjectID + szSkillType;
    }

    // get packet's name
    string getPacketName() const {
        return "GCCrossCounterOK3";
    }

    // get packet's debug string
    string toString() const;

    // get / set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get / set ObjectID
    ObjectID_t getTargetObjectID() const {
        return m_TargetObjectID;
    }
    void setTargetObjectID(ObjectID_t TargetObjectID) {
        m_TargetObjectID = TargetObjectID;
    }

    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

private:
    // ObjectID
    ObjectID_t m_ObjectID = 0;

    // TargetObjectID
    ObjectID_t m_TargetObjectID = 0;

    // CounterSkillType
    SkillType_t m_SkillType = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCCrossCounterOK3Factory;
//
// Factory for GCCrossCounterOK3
//
//////////////////////////////////////////////////////////////////////

class GCCrossCounterOK3Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_CROSS_COUNTER_OK_3;
    static constexpr std::string_view kName = "GCCrossCounterOK3";
    static constexpr PacketSize_t kMaxSize{szObjectID + szObjectID + szSkillType};

    // constructor
    GCCrossCounterOK3Factory() {}

    // destructor
    virtual ~GCCrossCounterOK3Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCCrossCounterOK3();
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
//
//////////////////////////////////////////////////////////////////////

#endif
