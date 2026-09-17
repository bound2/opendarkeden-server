//////////////////////////////////////////////////////////////////////
//
// Filename    : CGPickupMoney.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_PICKUP_MONEY_H__
#define __CG_PICKUP_MONEY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class CGPickupMoney;
//
//////////////////////////////////////////////////////////////////////

class CGPickupMoney : public Packet {
public:
    // constructor
    CGPickupMoney();

    // destructor
    ~CGPickupMoney();

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_PICKUP_MONEY;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGPickupMoneyPacketSize.
    PacketSize_t getPacketSize() const {
        return szObjectID + szCoord + szCoord;
    }

    // get packet name
    string getPacketName() const {
        return "CGPickupMoney";
    }

    // get packet's debug string
    string toString() const;

public:
    // get / set ObjectID
    ObjectID_t getObjectID() {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get/set X Coordicate
    Coord_t getZoneX() const {
        return m_ZoneX;
    }
    void setZoneX(Coord_t ZoneX) {
        m_ZoneX = ZoneX;
    }

    // get/set Y Coordicate
    Coord_t getZoneY() const {
        return m_ZoneY;
    }
    void setZoneY(Coord_t ZoneY) {
        m_ZoneY = ZoneY;
    }

private:
    // ObjectID
    ObjectID_t m_ObjectID;

    // X and Y coordinates in the Zone where the item is.
    Coord_t m_ZoneX;
    Coord_t m_ZoneY;
};


//////////////////////////////////////////////////////////////////////
//
// class CGPickupMoneyFactory;
//
// Factory for CGPickupMoney
//
//////////////////////////////////////////////////////////////////////

class CGPickupMoneyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_PICKUP_MONEY;
    static constexpr std::string_view kName = "CGPickupMoney";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoord + szCoord};

    // create packet
    Packet* createPacket() override {
        return new CGPickupMoney();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's max body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGPickupMoneyPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGPickupMoneyHandler;
//
//////////////////////////////////////////////////////////////////////

class CGPickupMoneyHandler {
public:
    // execute packet's handler
    static void execute(CGPickupMoney* pPacket, Player* player);
};

#endif
