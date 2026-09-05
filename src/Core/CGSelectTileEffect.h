//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSelectTileEffect.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_SELECT_TILE_EFFECT_H__
#define __CG_SELECT_TILE_EFFECT_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class CGSelectTileEffect
//////////////////////////////////////////////////////////////////////////////

class CGSelectTileEffect : public Packet {
public:
    CGSelectTileEffect(){};
    virtual ~CGSelectTileEffect(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_SELECT_TILE_EFFECT;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }
    string getPacketName() const {
        return "CGSelectTileEffect";
    }
    string toString() const;

public:
    ObjectID_t getEffectObjectID(void) const {
        return m_EffectObjectID;
    }
    void setEffectObjectID(ObjectID_t id) {
        m_EffectObjectID = id;
    }

private:
    ObjectID_t m_EffectObjectID; // 선택한 이펙트의 오브젝트 ID
};


//////////////////////////////////////////////////////////////////////////////
// class CGSelectTileEffectFactory
//////////////////////////////////////////////////////////////////////////////

class CGSelectTileEffectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SELECT_TILE_EFFECT;
    static constexpr std::string_view kName = "CGSelectTileEffect";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    Packet* createPacket() override {
        return new CGSelectTileEffect();
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
// class CGSelectTileEffectHandler
//////////////////////////////////////////////////////////////////////////////

class Effect;

class CGSelectTileEffectHandler {
public:
    static void execute(CGSelectTileEffect* pCGSelectTileEffect, Player* pPlayer);
    static void executeVampirePortal(CGSelectTileEffect* pCGSelectTileEffect, Player* pPlayer, Effect* pEffect);
};

#endif
