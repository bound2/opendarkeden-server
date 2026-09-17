//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAttackMeleeOK2.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_ATTACK_MELEE_OK_2_H__
#define __GC_ATTACK_MELEE_OK_2_H__

// include files
#include "Exception.h"
#include "ModifyInfo.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCAttackMeleeOK2;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCAttackMeleeOK2 : public ModifyInfo {
public:
    // constructor
    GCAttackMeleeOK2();

    // destructor
    ~GCAttackMeleeOK2();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ATTACK_MELEE_OK_2;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szObjectID + ModifyInfo::getPacketSize();
    }

    // get packet's name
    string getPacketName() const {
        return "GCAttackMeleeOK2";
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


private:
    // ObjectID
    ObjectID_t m_ObjectID;
};


//////////////////////////////////////////////////////////////////////
//
// class GCAttackMeleeOK2Factory;
//
// Factory for GCAttackMeleeOK2
//
//////////////////////////////////////////////////////////////////////

class GCAttackMeleeOK2Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ATTACK_MELEE_OK_2;
    static constexpr std::string_view kName = "GCAttackMeleeOK2";
    static constexpr PacketSize_t kMaxSize{szObjectID + ModifyInfo::getPacketMaxSize()};

    // constructor
    GCAttackMeleeOK2Factory() {}

    // destructor
    virtual ~GCAttackMeleeOK2Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCAttackMeleeOK2();
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
