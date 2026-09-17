//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddVampirePortal.h
// Written By  : excel96
// Description :
// A vampire portal is currently implemented as a kind of effect; this packet
// tells the client to attach a vampire portal effect to the ground.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_VAMPIRE_PORTAL_H__
#define __GC_ADD_VAMPIRE_PORTAL_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCAddVampirePortal;
//////////////////////////////////////////////////////////////////////////////

class GCAddVampirePortal : public Packet {
public:
    GCAddVampirePortal(){};
    ~GCAddVampirePortal(){};

    // The owner name length travels in one byte and the factory max budgets
    // this many characters.
    static constexpr uint kMaxOwnerIDSize = 20;

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_VAMPIRE_PORTAL;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE + m_OwnerID.size() + szDuration + szCoord * 2 + szZoneID + szCoord * 2 + szBYTE;
    }
    string getPacketName() const {
        return "GCAddVampirePortal";
    }
    string toString() const;

public:
    ObjectID_t getObjectID(void) const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t d) {
        m_ObjectID = d;
    }

    string getOwnerID(void) const {
        return m_OwnerID;
    }
    void setOwnerID(string ownerID) {
        m_OwnerID = ownerID;
    }

    Duration_t getDuration() const {
        return m_Duration;
    }
    void setDuration(Duration_t d) {
        m_Duration = d;
    }

    Coord_t getX(void) const {
        return m_X;
    }
    void setX(Coord_t x) {
        m_X = x;
    }

    Coord_t getY(void) const {
        return m_Y;
    }
    void setY(Coord_t x) {
        m_Y = x;
    }

    ZoneID_t getTargetZoneID(void) const {
        return m_TargetZoneID;
    }
    void setTargetZoneID(ZoneID_t id) {
        m_TargetZoneID = id;
    }

    Coord_t getTargetX(void) const {
        return m_TargetX;
    }
    void setTargetX(Coord_t x) {
        m_TargetX = x;
    }

    Coord_t getTargetY(void) const {
        return m_TargetY;
    }
    void setTargetY(Coord_t x) {
        m_TargetY = x;
    }

    BYTE getCreateFlag(void) const {
        return m_CreateFlag;
    }
    void setCreateFlag(BYTE flag) {
        m_CreateFlag = flag;
    }


private:
    ObjectID_t m_ObjectID;   // OID of the effect
    string m_OwnerID;        // Owner of the portal
    Duration_t m_Duration;   // How long the portal lasts
    Coord_t m_X;             // x coordinate of the tile the portal is attached to
    Coord_t m_Y;             // y coordinate of the tile the portal is attached to
    ZoneID_t m_TargetZoneID; // Id of the portal's target zone
    Coord_t m_TargetX;       // x coordinate of the portal's target
    Coord_t m_TargetY;       // y coordinate of the portal's target
    BYTE m_CreateFlag;       // Was it just created? (0 means some time has passed since it was created...)
};

//////////////////////////////////////////////////////////////////////////////
// class GCAddVampirePortalFactory;
//////////////////////////////////////////////////////////////////////////////

class GCAddVampirePortalFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_VAMPIRE_PORTAL;
    static constexpr std::string_view kName = "GCAddVampirePortal";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE + GCAddVampirePortal::kMaxOwnerIDSize + szDuration +
                                           szCoord * 2 + szZoneID + szCoord * 2 + szBYTE};

    Packet* createPacket() override {
        return new GCAddVampirePortal();
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
