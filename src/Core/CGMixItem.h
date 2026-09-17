//////////////////////////////////////////////////////////////////////////////
// Filename    : CGMixItem.h
// Written By  : excel96
// Description :
// When an item in the inventory is used, the client sends X, Y and the ObjectID;
// the server then runs the code that matches the item's class.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_MIX_ITEM_H__
#define __CG_MIX_ITEM_H__

#include "Assert1.h"
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGMixItem;
//////////////////////////////////////////////////////////////////////////////

class CGMixItem : public Packet {
public:
    CGMixItem(){};
    ~CGMixItem(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_MIX_ITEM;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szCoordInven + szCoordInven + (szObjectID * 2);
    }
    string getPacketName() const {
        return "CGMixItem";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    CoordInven_t getX() const {
        return m_InvenX;
    }
    void setX(CoordInven_t InvenX) {
        m_InvenX = InvenX;
    }

    CoordInven_t getY() const {
        return m_InvenY;
    }
    void setY(CoordInven_t InvenY) {
        m_InvenY = InvenY;
    }

    ObjectID_t getTargetObjectID(uint index) const {
        Assert(index < 2);
        return m_TargetObjectID[index];
    }
    void setTargetObjectID(uint index, ObjectID_t oid) {
        Assert(index < 2);
        m_TargetObjectID[index] = oid;
    }

private:
    ObjectID_t m_ObjectID = 0; // Object id of the item
    CoordInven_t m_InvenX = 0; // Inventory X coordinate of the item
    CoordInven_t m_InvenY = 0; // Inventory Y coordinate of the item

    ObjectID_t m_TargetObjectID[2] = {0, 0}; // Object IDs of the two items to combine
};


//////////////////////////////////////////////////////////////////////////////
// class CGMixItemFactory;
//////////////////////////////////////////////////////////////////////////////

class CGMixItemFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_MIX_ITEM;
    static constexpr std::string_view kName = "CGMixItem";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoordInven + szCoordInven + (szObjectID * 2)};

    Packet* createPacket() override {
        return new CGMixItem();
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
// class CGMixItemHandler;
//////////////////////////////////////////////////////////////////////////////

class Inventory;
class Item;

class CGMixItemHandler {
public:
    static void execute(CGMixItem* pPacket, Player* pPlayer);

public:
    static void executeMix(CGMixItem* pPacket, Player* pPlayer, Item* pItem);
    static void executeDetach(CGMixItem* pPacket, Player* pPlayer, Item* pItem);
    static void executeClearOption(CGMixItem* pPacket, Player* pPlayer, Item* pItem);
};

#endif
