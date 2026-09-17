//////////////////////////////////////////////////////////////////////
//
// Filename    : GCCrossCounterOK1.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_CROSS_COUNTER_OK_1_H__
#define __GC_CROSS_COUNTER_OK_1_H__

// include files
#include "Exception.h"
#include "ModifyInfo.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCCrossCounterOK1;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCCrossCounterOK1 : public ModifyInfo {
public:
    // constructor
    GCCrossCounterOK1();

    // destructor
    ~GCCrossCounterOK1();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_CROSS_COUNTER_OK_1;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szObjectID + ModifyInfo::getPacketSize() + szSkillType;
    }

    // get packet's name
    string getPacketName() const {
        return "GCCrossCounterOK1";
    }

    // get packet's debug string
    string toString() const;

    // get / set CEffectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
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

    // Counter SkillType
    SkillType_t m_SkillType = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCCrossCounterOK1Factory;
//
// Factory for GCCrossCounterOK1
//
//////////////////////////////////////////////////////////////////////

class GCCrossCounterOK1Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_CROSS_COUNTER_OK_1;
    static constexpr std::string_view kName = "GCCrossCounterOK1";
    static constexpr PacketSize_t kMaxSize{szObjectID + ModifyInfo::getPacketMaxSize() + szSkillType};

    // constructor
    GCCrossCounterOK1Factory() {}

    // destructor
    virtual ~GCCrossCounterOK1Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCCrossCounterOK1();
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
