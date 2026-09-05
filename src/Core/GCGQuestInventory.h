//////////////////////////////////////////////////////////////////////////////
// Filename    : GCGQuestInventory.h
// Written By  : elca@ewestsoft.com
// Description :
// 기술이 성공했을때 보내는 패킷을 위한 클래스 정의
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_GQUEST_INVENTORY_H__
#define __GC_GQUEST_INVENTORY_H__

#include <list>

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

#define MAX_GQUEST_INVENTORY_ITEM_NUM 100

//////////////////////////////////////////////////////////////////////////////
// class GCGQuestInventory;
// 게임서버에서 클라이언트로 자신의 기술이 성공을 알려주기 위한 클래스
//////////////////////////////////////////////////////////////////////////////

class GCGQuestInventory : public Packet {
public:
    GCGQuestInventory();
    ~GCGQuestInventory();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_GQUEST_INVENTORY;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE + szItemType * m_ItemList.size();
    }
    string getPacketName() const {
        return "GCGQuestInventory";
    }
    string toString() const;

public:
    list<ItemType_t>& getItemList() {
        return m_ItemList;
    }
    const list<ItemType_t>& getItemList() const {
        return m_ItemList;
    }

private:
    list<ItemType_t> m_ItemList;
};


//////////////////////////////////////////////////////////////////////////////
// class GCGQuestInventoryFactory;
//////////////////////////////////////////////////////////////////////////////

class GCGQuestInventoryFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GQUEST_INVENTORY;
    static constexpr std::string_view kName = "GCGQuestInventory";
    static constexpr PacketSize_t kMaxSize{szBYTE + szItemType * MAX_GQUEST_INVENTORY_ITEM_NUM};

    GCGQuestInventoryFactory() {}
    virtual ~GCGQuestInventoryFactory() {}

public:
    Packet* createPacket() override {
        return new GCGQuestInventory();
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
