//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRemoveEffect.h
// Written By  : crazydog
// Description : Effect removal.
//
//////////////////////////////////////////////////////////////////////

#ifndef __REMOVE_EFFECT_H__
#define __REMOVE_EFFECT_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCRemoveEffect;
//
// Object the game server uses to tell the client about its own changed data
// It is carried in RemoveEffectrmation, SkillToObjectOK and the like.
//
//////////////////////////////////////////////////////////////////////

class GCRemoveEffect : public Packet {
public:
    // constructor
    GCRemoveEffect();

    // destructor
    ~GCRemoveEffect();

public:
    PacketID_t getPacketID() const {
        return PACKET_GC_REMOVE_EFFECT;
    }
    string getPacketName() const {
        return "GCRemoveEffect";
    }


    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;

    // The effect list is counted in a BYTE, and the factory max budgets this
    // many ids.
    static constexpr uint kMaxCount = 255;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return (PacketSize_t)(szObjectID + szBYTE + szEffectID * m_EffectList.size());
    }
    static constexpr PacketSize_t getPacketMaxSize() {
        return szObjectID + szBYTE + szEffectID * kMaxCount;
    }

    // get packet's debug string
    string toString() const;

    // get ListNumber
    BYTE getListNum() const {
        return (BYTE)m_EffectList.size();
    }

    // get&set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t id) {
        m_ObjectID = id;
    }

    // add / delete / clear S List
    void addEffectList(EffectID_t id);

    // ClearList
    void clearList() {
        m_EffectList.clear();
    }

    // pop front Element in Status List
    WORD popFrontListElement() {
        if (m_EffectList.empty())
            throw InvalidProtocolException("no effect left");
        EffectID_t effectID = m_EffectList.front();
        m_EffectList.pop_front();
        return effectID;
    }

protected:
    ObjectID_t m_ObjectID = 0;

    // Status List
    list<EffectID_t> m_EffectList;
};

//////////////////////////////////////////////////////////////////////
//
// class GCRemoveEffectFactory;
//
// Factory for GCRemoveEffect
//
//////////////////////////////////////////////////////////////////////

class GCRemoveEffectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_REMOVE_EFFECT;
    static constexpr std::string_view kName = "GCRemoveEffect";
    static constexpr PacketSize_t kMaxSize{GCRemoveEffect::getPacketMaxSize()};

    // constructor
    GCRemoveEffectFactory() {}

    // destructor
    virtual ~GCRemoveEffectFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCRemoveEffect();
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
