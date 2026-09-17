//////////////////////////////////////////////////////////////////////////////
// Filename    : GCShopBuyFail.h
// Written By  : excel96
// Description :
// A player tried to buy an item from a shop and,
// this packet is sent when the normal purchase fails.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_SHOP_BUY_FAIL_H__
#define __GC_SHOP_BUY_FAIL_H__

#include "Packet.h"
#include "PacketFactory.h"

enum GCShopBuyFailCode {
    // Not enough money.
    GC_SHOP_BUY_FAIL_NOT_ENOUGH_MONEY = 0,

    // Not enough room.
    GC_SHOP_BUY_FAIL_NOT_ENOUGH_SPACE,

    // The NPC does not exist.
    GC_SHOP_BUY_FAIL_NPC_NOT_EXIST,

    // What was sent as an NPC is not an NPC.
    GC_SHOP_BUY_FAIL_NOT_NPC,

    // There is no item at the position given.
    GC_SHOP_BUY_FAIL_ITEM_NOT_EXIST,

    GC_SHOP_BUY_FAIL_NOT_ENOUGH_BLACK_STAR,
    GC_SHOP_BUY_FAIL_NOT_ENOUGH_RED_STAR,
    GC_SHOP_BUY_FAIL_NOT_ENOUGH_BLUE_STAR,
    GC_SHOP_BUY_FAIL_NOT_ENOUGH_GREEN_STAR,
    GC_SHOP_BUY_FAIL_NOT_ENOUGH_CYAN_STAR,

    GC_SHOP_BUY_FAIL_MAX
};

const string GCShopBuyFailCode2String[] = {"NOT_ENOUGH_MONEY",
                                           "NOT_ENOUGH_SPACE",
                                           "NPC_NOT_EXIST",
                                           "NOT_NPC",
                                           "ITEM_NOT_EXIST",
                                           "NOT_ENOUGH_BLACK_STAR",
                                           "NOT_ENOUGH_RED_STAR",
                                           "NOT_ENOUGH_BLUE_STAR",
                                           "NOT_ENOUGH_GREEN_STAR",
                                           "NOT_ENOUGH_CYAN_STAR",
                                           "MAX"};

//////////////////////////////////////////////////////////////////////////////
// class GCShopBuyFail;
//////////////////////////////////////////////////////////////////////////////
class GCShopBuyFail : public Packet {
public:
    GCShopBuyFail();
    ~GCShopBuyFail();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_SHOP_BUY_FAIL;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + // NPC object id
               szBYTE +     // fail code
               szuint;      // the amount the refusal names
    }
    string getPacketName() const {
        return "GCShopBuyFail";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t creatureID) {
        m_ObjectID = creatureID;
    }

    BYTE getCode(void) const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

    uint getAmount(void) const {
        return m_Amount;
    }
    void setAmount(uint amount) {
        m_Amount = amount;
    }

private:
    ObjectID_t m_ObjectID;
    BYTE m_Code;
    uint m_Amount;
};

//////////////////////////////////////////////////////////////////////////////
// class GCShopBuyFailFactory;
//////////////////////////////////////////////////////////////////////////////
class GCShopBuyFailFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SHOP_BUY_FAIL;
    static constexpr std::string_view kName = "GCShopBuyFail";
    static constexpr PacketSize_t kMaxSize{szObjectID + // NPC object id
                                           szBYTE +     // fail code
                                           szuint};     // the amount the refusal names

    Packet* createPacket() override {
        return new GCShopBuyFail();
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
