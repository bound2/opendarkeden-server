//////////////////////////////////////////////////////////////////////////////
// Filename    : CGUseItemFromGQuestInventory.h
// Written By  : excel96
// Description :
// 인벤토리 안의 아이템을 사용할 때, 클라이언트가 X, Y 및 ObjectID를
// 보내면 아이템 클래스에 따라서, 서버가 이에 맞는 코드를 처리한다.
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
    BYTE m_Index; // 아이템의 index
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
