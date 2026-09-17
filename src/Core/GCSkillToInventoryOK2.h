//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillToInventoryOK2.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SKILL_TO_INVENTORY_OK_2_H__
#define __GC_SKILL_TO_INVENTORY_OK_2_H__


// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCSkillToInventoryOK2;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCSkillToInventoryOK2 : public Packet {
public:
    // constructor
    GCSkillToInventoryOK2();

    // destructor
    ~GCSkillToInventoryOK2();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SKILL_TO_INVENTORY_OK_2;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szSkillType + szObjectID + szDuration;
    }

    // get packet's name
    string getPacketName() const {
        return "GCSkillToInventoryOK2";
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

    // get / set SkillType
    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

    // get / set Duration
    Duration_t getDuration() const {
        return m_Duration;
    }
    void setDuration(Duration_t Duration) {
        m_Duration = Duration;
    }

private:
    // ObjectID
    ObjectID_t m_ObjectID;

    // SkillType
    SkillType_t m_SkillType;

    // Duration
    Duration_t m_Duration;
};


//////////////////////////////////////////////////////////////////////
//
// class GCSkillToInventoryOK2Factory;
//
// Factory for GCSkillToInventoryOK2
//
//////////////////////////////////////////////////////////////////////

class GCSkillToInventoryOK2Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SKILL_TO_INVENTORY_OK_2;
    static constexpr std::string_view kName = "GCSkillToInventoryOK2";
    static constexpr PacketSize_t kMaxSize{szSkillType + szObjectID + szDuration};

    // constructor
    GCSkillToInventoryOK2Factory() {}

    // destructor
    virtual ~GCSkillToInventoryOK2Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCSkillToInventoryOK2();
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
