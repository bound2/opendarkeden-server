//////////////////////////////////////////////////////////////////////
//
// Filename    : CGAbsorbSoul
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_ABSORB_SOUL_H__
#define __CG_ABSORB_SOUL_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"


//////////////////////////////////////////////////////////////////////
//
// class CGAbsorbSoul;
//
//////////////////////////////////////////////////////////////////////

class CGAbsorbSoul : public Packet {
public:
    // constructor
    CGAbsorbSoul();

    // destructor
    ~CGAbsorbSoul();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_ABSORB_SOUL;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID + szZoneCoord + szZoneCoord + szObjectID + szCoordInven + szCoordInven + szCoordInven +
               szCoordInven;
    }

    // get packet name
    string getPacketName() const {
        return "CGAbsorbSoul";
    }

    // get packet's debug string
    string toString() const;

    // get/set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    ZoneCoord_t getTargetZoneX() const {
        return m_TargetZoneX;
    }
    void setTargetZoneX(ZoneCoord_t x) {
        m_TargetZoneX = x;
    }

    ZoneCoord_t getTargetZoneY() const {
        return m_TargetZoneY;
    }
    void setTargetZoneY(ZoneCoord_t y) {
        m_TargetZoneY = y;
    }

    ObjectID_t getInvenObjectID() const {
        return m_InvenObjectID;
    }
    void setInvenObjectID(ObjectID_t invenObjectID) {
        m_InvenObjectID = invenObjectID;
    }

    Coord_t getInvenX() const {
        return m_InvenX;
    }
    void setInvenX(CoordInven_t x) {
        m_InvenX = x;
    }

    Coord_t getInvenY() const {
        return m_InvenY;
    }
    void setInvenY(CoordInven_t y) {
        m_InvenY = y;
    }

    Coord_t getTargetInvenX() const {
        return m_TargetInvenX;
    }
    void setTargetInvenX(CoordInven_t x) {
        m_TargetInvenX = x;
    }

    Coord_t getTargetInvenY() const {
        return m_TargetInvenY;
    }
    void setTargetInvenY(CoordInven_t y) {
        m_TargetInvenY = y;
    }

private:
    ObjectID_t m_ObjectID = 0; // ObjectID

    ZoneCoord_t m_TargetZoneX = 0;
    ZoneCoord_t m_TargetZoneY = 0;

    ObjectID_t m_InvenObjectID = 0;  // ObjectID of the larva
    CoordInven_t m_InvenX = 0;       // X coordinate
    CoordInven_t m_InvenY = 0;       // Y coordinate
    CoordInven_t m_TargetInvenX = 0; // Target X coordinate
    CoordInven_t m_TargetInvenY = 0; // Target Y coordinate
};


//////////////////////////////////////////////////////////////////////
//
// class CGAbsorbSoulFactory;
//
// Factory for CGAbsorbSoul
//
//////////////////////////////////////////////////////////////////////

class CGAbsorbSoulFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_ABSORB_SOUL;
    static constexpr std::string_view kName = "CGAbsorbSoul";
    static constexpr PacketSize_t kMaxSize{szObjectID + szZoneCoord + szZoneCoord + szObjectID + szCoordInven +
                                           szCoordInven + szCoordInven + szCoordInven};

    // constructor
    CGAbsorbSoulFactory() {}

    // destructor
    virtual ~CGAbsorbSoulFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGAbsorbSoul();
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
// class CGAbsorbSoulHandler;
//
//////////////////////////////////////////////////////////////////////

class CGAbsorbSoulHandler {
public:
    // execute packet's handler
    static void execute(CGAbsorbSoul* pCGAbsorbSoul, Player* player);
};

#endif
