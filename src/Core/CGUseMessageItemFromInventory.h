//////////////////////////////////////////////////////////////////////////////
// Filename    : CGUseMessageItemFromInventory.h
// Written By  : excel96
// Description :
// When an item in the inventory is used, the client sends X, Y and the ObjectID;
// the server then runs the code that matches the item's class.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_USE_MESSAGE_ITEM_FROM_INVENTORY_H__
#define __CG_USE_MESSAGE_ITEM_FROM_INVENTORY_H__

#include "CGUseItemFromInventory.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////////////
// class CGUseMessageItemFromInventory;
//////////////////////////////////////////////////////////////////////////////

class CGUseMessageItemFromInventory : public CGUseItemFromInventory {
public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_USE_MESSAGE_ITEM_FROM_INVENTORY;
    }
    PacketSize_t getPacketSize() const {
        return CGUseItemFromInventory::getPacketSize() + de::wire::stringWireSize(m_Message);
    }
    string getPacketName() const {
        return "CGUseMessageItemFromInventory";
    }
    string toString() const;

public:
    const string& getMessage() const {
        return m_Message;
    }
    void setMessage(const string& msg) {
        m_Message = msg;
    }

private:
    string m_Message;
};


//////////////////////////////////////////////////////////////////////////////
// class CGUseMessageItemFromInventoryFactory;
//////////////////////////////////////////////////////////////////////////////

class CGUseMessageItemFromInventoryFactory : public CGUseItemFromInventoryFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_USE_MESSAGE_ITEM_FROM_INVENTORY;
    static constexpr std::string_view kName = "CGUseMessageItemFromInventory";
    static constexpr PacketSize_t kMaxSize{CGUseItemFromInventoryFactory::kMaxSize + szBYTE + 128};

    Packet* createPacket() override {
        return new CGUseMessageItemFromInventory();
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
// class CGUseMessageItemFromInventoryHandler;
//////////////////////////////////////////////////////////////////////////////

class CGUseMessageItemFromInventoryHandler {
public:
    static void execute(CGUseMessageItemFromInventory* pPacket, Player* player);

protected:
    static void executeEventTree(CGUseMessageItemFromInventory* pPacket, Player* player);
    // add by Coffee
    static void executeEventFromMessage(CGUseMessageItemFromInventory* pPacket, Player* player);
};

#endif
