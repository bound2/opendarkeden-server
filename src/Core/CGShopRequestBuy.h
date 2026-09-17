////////////////////////////////////////////////////////////////////////////////
// Filename    : CGShopRequestBuy.h
// Description :
// Sent when a player looks at a shop NPC's display window and wants to buy
// an item. The server checks that the player has enough money and enough
// room in the inventory, then hands the item over to the player.
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_SHOP_REQUEST_BUY_H__
#define __CG_SHOP_REQUEST_BUY_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
//
// class CGShopRequestBuy;
//
////////////////////////////////////////////////////////////////////////////////

class CGShopRequestBuy : public Packet {
public:
    CGShopRequestBuy(){};
    virtual ~CGShopRequestBuy(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_SHOP_REQUEST_BUY;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szShopRackType + szBYTE + szItemNum + szCoord * 2;
    }
    string getPacketName() const {
        return "CGShopRequestBuy";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    ShopRackType_t getShopType(void) const {
        return m_RackType;
    }
    void setShopType(ShopRackType_t type) {
        m_RackType = type;
    }

    BYTE getShopIndex(void) const {
        return m_RackIndex;
    }
    void setShopIndex(BYTE index) {
        m_RackIndex = index;
    }

    ItemNum_t getItemNum(void) const {
        return m_Num;
    }
    void setItemNum(ItemNum_t num) {
        m_Num = num;
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
    void setY(Coord_t y) {
        m_Y = y;
    }

private:
    ObjectID_t m_ObjectID = 0;     // NPC ID
    ShopRackType_t m_RackType = 0; // Rack type
    BYTE m_RackIndex = 0;          // Rack index
    ItemNum_t m_Num = 0;           // Number of item
    Coord_t m_X = 0;               // Coordinates inside the player's inventory
    Coord_t m_Y = 0;
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGShopRequestBuyFactory;
//
////////////////////////////////////////////////////////////////////////////////

class CGShopRequestBuyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SHOP_REQUEST_BUY;
    static constexpr std::string_view kName = "CGShopRequestBuy";
    static constexpr PacketSize_t kMaxSize{szObjectID + szShopRackType + szBYTE + szItemNum + szCoord * 2};

    Packet* createPacket() override {
        return new CGShopRequestBuy();
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


////////////////////////////////////////////////////////////////////////////////
//
// class CGShopRequestBuyHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGShopRequestBuyHandler {
public:
    static void execute(CGShopRequestBuy* pPacket, Player* player);
    static void executeNormal(CGShopRequestBuy* pPacket, Player* player);
    static void executeMotorcycle(CGShopRequestBuy* pPacket, Player* player);
    static void executeEvent(CGShopRequestBuy* pPacket, Player* player);
};

#endif
