//////////////////////////////////////////////////////////////////////////////
// Filename    : CGUseItemFromGQuestInventory.h
// Written By  : excel96
// Description :
// When an item in the inventory is used, the client sends X, Y and the ObjectID;
// the server then runs the code that matches the item's class.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_USE_ITEM_FROM_GQUEST_INVENTORY_H__
#define __CG_USE_ITEM_FROM_GQUEST_INVENTORY_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGUseItemFromGQuestInventory;
//////////////////////////////////////////////////////////////////////////////

class CGUseItemFromGQuestInventory : public Packet {
public:
    CGUseItemFromGQuestInventory(){};
    virtual ~CGUseItemFromGQuestInventory(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_USE_ITEM_FROM_GQUEST_INVENTORY;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }
    string getPacketName() const {
        return "CGUseItemFromGQuestInventory";
    }
    string toString() const;

public:
    BYTE getIndex() const {
        return m_Index;
    }
    void setIndex(BYTE Index) {
        m_Index = Index;
    }

private:
    BYTE m_Index = 0; // Index of the item
};


//////////////////////////////////////////////////////////////////////////////
// class CGUseItemFromGQuestInventoryFactory;
//////////////////////////////////////////////////////////////////////////////

class CGUseItemFromGQuestInventoryFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_USE_ITEM_FROM_GQUEST_INVENTORY;
    static constexpr std::string_view kName = "CGUseItemFromGQuestInventory";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    Packet* createPacket() override {
        return new CGUseItemFromGQuestInventory();
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
// class CGUseItemFromGQuestInventoryHandler;
//////////////////////////////////////////////////////////////////////////////

class GQuestInventory;
class Item;

class CGUseItemFromGQuestInventoryHandler {
public:
    static void execute(CGUseItemFromGQuestInventory* pPacket, Player* pPlayer);
};

#endif
