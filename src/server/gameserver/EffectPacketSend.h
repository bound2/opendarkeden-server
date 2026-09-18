//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectPacketSend.h
// Written by  :
// Description : Defense reduction effect caused by PacketSend
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_PACKET_SEND__
#define __EFFECT_PACKET_SEND__

#include "Effect.h"

class Zone;
class Packet;

//////////////////////////////////////////////////////////////////////////////
// class EffectPacketSend
//////////////////////////////////////////////////////////////////////////////

class EffectPacketSend : public Effect {
public:
    EffectPacketSend(Zone* pZone, ZoneCoord_t X, ZoneCoord_t Y);

public:
    // Not used.
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_PK_ZONE_REGEN;
    }

    void unaffect();

    string toString() const;

public:
    Packet* getPacket() const {
        return m_pPacket;
    }
    void setPacket(Packet* pPacket) {
        m_pPacket = pPacket;
    }

private:
    Packet* m_pPacket;
    ZoneCoord_t m_X, m_Y;
};

#endif // __EFFECT_PACKET_SEND__
