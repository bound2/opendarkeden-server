//////////////////////////////////////////////////////////////////////
//
// Filename    : CGPartyPosition
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_PARTY_POSITION_H__
#define __CG_PARTY_POSITION_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"


//////////////////////////////////////////////////////////////////////
//
// class CGPartyPosition;
//
//////////////////////////////////////////////////////////////////////

class CGPartyPosition : public Packet {
public:
    // constructor
    CGPartyPosition();

    // destructor
    ~CGPartyPosition();


public:
    // Initialize from the incoming stream.
    void read(SocketInputStream& iStream);

    // Write this packet to the outgoing stream.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_PARTY_POSITION;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szZoneID + szZoneCoord * 2 + szHP * 2;
    }

    // get packet name
    string getPacketName() const {
        return "CGPartyPosition";
    }

    // get packet's debug string
    string toString() const;

public:
    void setZoneID(ZoneID_t zoneID) {
        m_ZoneID = zoneID;
    }
    ZoneID_t getZoneID() const {
        return m_ZoneID;
    }

    void setXY(ZoneCoord_t X, ZoneCoord_t Y) {
        m_X = X;
        m_Y = Y;
    }
    ZoneCoord_t getX() const {
        return m_X;
    }
    ZoneCoord_t getY() const {
        return m_Y;
    }

    void setHP(HP_t hp) {
        m_HP = hp;
    }
    HP_t getHP() const {
        return m_HP;
    }

    void setMaxHP(HP_t hp) {
        m_MaxHP = hp;
    }
    HP_t getMaxHP() const {
        return m_MaxHP;
    }

private:
    ZoneID_t m_ZoneID;
    ZoneCoord_t m_X, m_Y;
    HP_t m_MaxHP, m_HP;
};


//////////////////////////////////////////////////////////////////////
//
// class CGPartyPositionFactory;
//
// Factory for CGPartyPosition
//
//////////////////////////////////////////////////////////////////////

class CGPartyPositionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_PARTY_POSITION;
    static constexpr std::string_view kName = "CGPartyPosition";
    static constexpr PacketSize_t kMaxSize{szZoneID + szZoneCoord * 2 + szHP * 2};

    // constructor
    CGPartyPositionFactory() {}

    // destructor
    virtual ~CGPartyPositionFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGPartyPosition();
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
// class CGPartyPositionHandler;
//
//////////////////////////////////////////////////////////////////////

class CGPartyPositionHandler {
public:
    // execute packet's handler
    static void execute(CGPartyPosition* pCGPartyPosition, Player* player);
};

#endif
