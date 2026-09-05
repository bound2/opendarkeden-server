////////////////////////////////////////////////////////////////////////////////
// Filename    : CGTradeRemoveItem.h
// Written By  : 김성민
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_TRADE_REMOVE_ITEM_H__
#define __CG_TRADE_REMOVE_ITEM_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
//
// class CGTradeRemoveItem;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradeRemoveItem : public Packet {
public:
    CGTradeRemoveItem(){};
    virtual ~CGTradeRemoveItem(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_TRADE_REMOVE_ITEM;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID * 2;
    }
    string getPacketName() const {
        return "CGTradeRemoveItem";
    }
    string toString() const;

public:
    ObjectID_t getTargetObjectID() const {
        return m_TargetObjectID;
    }
    void setTargetObjectID(ObjectID_t id) {
        m_TargetObjectID = id;
    }

    ObjectID_t getItemObjectID() const {
        return m_ItemObjectID;
    }
    void setItemObjectID(ObjectID_t id) {
        m_ItemObjectID = id;
    }

private:
    ObjectID_t m_TargetObjectID; // 교환을 원하는 상대방의 ObjectID
    ObjectID_t m_ItemObjectID;   // 교환 리스트에 추가할 아이템의 OID
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGTradeRemoveItemFactory;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradeRemoveItemFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_TRADE_REMOVE_ITEM;
    static constexpr std::string_view kName = "CGTradeRemoveItem";
    static constexpr PacketSize_t kMaxSize{szObjectID * 2};

    Packet* createPacket() override {
        return new CGTradeRemoveItem();
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
// class CGTradeRemoveItemHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradeRemoveItemHandler {
public:
    static void execute(CGTradeRemoveItem* pPacket, Player* player);
    static void executeSlayer(CGTradeRemoveItem* pPacket, Player* player);
    static void executeVampire(CGTradeRemoveItem* pPacket, Player* player);
    static void executeOusters(CGTradeRemoveItem* pPacket, Player* player);
    static void executeError(CGTradeRemoveItem* pPacket, Player* player, BYTE ErrorCode);
};

#endif
