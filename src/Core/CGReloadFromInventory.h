//////////////////////////////////////////////////////////////////////
//
// Filename    : CGReloadFromInventory.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_RELOAD_FROM_INVENTORY_H__
#define __CG_RELOAD_FROM_INVENTORY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class CGReloadFromInventory;
//
//////////////////////////////////////////////////////////////////////

class CGReloadFromInventory : public Packet {
public:
    // constructor
    CGReloadFromInventory();

    // destructor
    ~CGReloadFromInventory();

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_RELOAD_FROM_INVENTORY;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGReloadFromInventoryPacketSize.
    PacketSize_t getPacketSize() const {
        return szObjectID + szCoordInven + szCoordInven;
    }

    // get packet name
    string getPacketName() const {
        return "CGReloadFromInventory";
    }

    // get packet's debug string
    string toString() const;

public:
    // get / set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get / set Inventory X
    CoordInven_t getX() const {
        return m_InvenX;
    }
    void setX(CoordInven_t InvenX) {
        m_InvenX = InvenX;
    }

    // get / set Inventory Y
    CoordInven_t getY() const {
        return m_InvenY;
    }
    void setY(CoordInven_t InvenY) {
        m_InvenY = InvenY;
    }


private:
    // ObjectID
    ObjectID_t m_ObjectID = 0;

    // X and Y coordinates in the inventory
    CoordInven_t m_InvenX = 0;
    CoordInven_t m_InvenY = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class CGReloadFromInventoryFactory;
//
// Factory for CGReloadFromInventory
//
//////////////////////////////////////////////////////////////////////

class CGReloadFromInventoryFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_RELOAD_FROM_INVENTORY;
    static constexpr std::string_view kName = "CGReloadFromInventory";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoordInven + szCoordInven};

    // create packet
    Packet* createPacket() override {
        return new CGReloadFromInventory();
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
    // Define and return const static CGReloadFromInventoryPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGReloadFromInventoryHandler;
//
//////////////////////////////////////////////////////////////////////

class CGReloadFromInventoryHandler {
public:
    // execute packet's handler
    static void execute(CGReloadFromInventory* pPacket, Player* player);
};

#endif
