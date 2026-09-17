//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSelectWayPoint.h
// Written By  : excel96
// Description :
// Packet sent when a slayer wants to travel elsewhere through a personal portal
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_SELECT_WAYPOINT_H__
#define __CG_SELECT_WAYPOINT_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class CGSelectWayPoint;
//////////////////////////////////////////////////////////////////////////////

class CGSelectWayPoint : public Packet {
public:
    CGSelectWayPoint(){};
    virtual ~CGSelectWayPoint(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_SELECT_WAYPOINT;
    }
    PacketSize_t getPacketSize() const {
        return szZoneID + szCoord * 2;
    }
    string getPacketName() const {
        return "CGSelectWayPoint";
    }
    string toString() const;

public:
    ZoneID_t getZoneID() const {
        return m_ZoneID;
    }
    void setZoneID(ZoneID_t ZoneID) {
        m_ZoneID = ZoneID;
    }

    Coord_t getX(void) const {
        return m_X;
    }
    void setX(Coord_t X) {
        m_X = X;
    }

    Coord_t getY(void) const {
        return m_Y;
    }
    void setY(Coord_t Y) {
        m_Y = Y;
    }

private:
    ZoneID_t m_ZoneID = 0; // Id of the zone to go to
    Coord_t m_X = 0;       // X coordinate in the zone to go to
    Coord_t m_Y = 0;       // Y coordinate in the zone to go to
};

//////////////////////////////////////////////////////////////////////////////
// class CGSelectWayPointFactory;
//////////////////////////////////////////////////////////////////////////////

class CGSelectWayPointFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SELECT_WAYPOINT;
    static constexpr std::string_view kName = "CGSelectWayPoint";
    static constexpr PacketSize_t kMaxSize{szZoneID + szCoord * 2};

    Packet* createPacket() override {
        return new CGSelectWayPoint();
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
// class CGSelectWayPointHandler;
//////////////////////////////////////////////////////////////////////////////

class CGSelectWayPointHandler {
public:
    static void execute(CGSelectWayPoint* pCGSelectWayPoint, Player* pPlayer);
    static void executeEnterQuestZone(CGSelectWayPoint* pCGSelectWayPoint, Player* pPlayer, int targetDynamicZoneType);
};

#endif
