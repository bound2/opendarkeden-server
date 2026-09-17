//////////////////////////////////////////////////////////////////////////////
// Filename    : CGUseItemFromInventory.h
// Written By  : excel96
// Description :
// When an item in the inventory is used, the client sends X, Y and the ObjectID;
// the server then runs the code that matches the item's class.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_USE_ITEM_FROM_INVENTORY_H__
#define __CG_USE_ITEM_FROM_INVENTORY_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGUseItemFromInventory;
//////////////////////////////////////////////////////////////////////////////

class CGUseItemFromInventory : public Packet {
public:
    CGUseItemFromInventory(){};
    virtual ~CGUseItemFromInventory(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_USE_ITEM_FROM_INVENTORY;
    }
    // m_InventoryItemObjectID is not on the wire (read/write skip it)
    PacketSize_t getPacketSize() const {
        return szObjectID + szCoordInven + szCoordInven;
    }
    string getPacketName() const {
        return "CGUseItemFromInventory";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    ObjectID_t getInventoryItemObjectID() {
        return m_InventoryItemObjectID;
    }
    void setInventoryItemObjectID(ObjectID_t InventoryItemObjectID) {
        m_InventoryItemObjectID = InventoryItemObjectID;
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

private:
    ObjectID_t m_ObjectID; // Object id of the item
    // Object id of the sub inventory item. 0 means it is used from the main inventory
    ObjectID_t m_InventoryItemObjectID;
    CoordInven_t m_InvenX; // Inventory X coordinate of the item
    CoordInven_t m_InvenY; // Inventory Y coordinate of the item
};


//////////////////////////////////////////////////////////////////////////////
// class CGUseItemFromInventoryFactory;
//////////////////////////////////////////////////////////////////////////////

class CGUseItemFromInventoryFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_USE_ITEM_FROM_INVENTORY;
    static constexpr std::string_view kName = "CGUseItemFromInventory";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoordInven + szCoordInven};

    Packet* createPacket() override {
        return new CGUseItemFromInventory();
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
// class CGUseItemFromInventoryHandler;
//////////////////////////////////////////////////////////////////////////////

class Inventory;
class Item;

class CGUseItemFromInventoryHandler {
public:
    static void execute(CGUseItemFromInventory* pPacket, Player* pPlayer);

protected:
    static void executePotion(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeMagazine(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeETC(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeSerum(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeVampireETC(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeSlayerPortalItem(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeOustersSummonItem(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeKeyItem(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeFirecraker(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeDyePotion(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeResurrectItem(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeTranslator(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeEffectItem(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executePetItem(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executePetFood(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeEventGiftBox(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeTrapItem(CGUseItemFromInventory* pPacket, Player* pPlayer);
    static void executeForceScroll(CGUseItemFromInventory* pPacket, Player* pPlayer);
};

#endif
