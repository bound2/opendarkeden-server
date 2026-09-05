
//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddStoreItem.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_STORE_ITEM_H__
#define __GC_ADD_STORE_ITEM_H__

#include "Packet.h"
#include "PacketFactory.h"
#include "StoreInfo.h"

//////////////////////////////////////////////////////////////////////////////
// class GCAddStoreItem;
//////////////////////////////////////////////////////////////////////////////

class GCAddStoreItem : public Packet {
public:
    GCAddStoreItem() {}
    virtual ~GCAddStoreItem() noexcept;

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_STORE_ITEM;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE + m_Item.getSize();
    }
    string getPacketName() const {
        return "GCAddStoreItem";
    }
    string toString() const;

    ObjectID_t getOwnerObjectID() const {
        return m_OwnerObjectID;
    }
    void setOwnerObjectID(ObjectID_t oid) {
        m_OwnerObjectID = oid;
    }

    BYTE getIndex() const {
        return m_Index;
    }
    void setIndex(BYTE index) {
        m_Index = index;
    }

    StoreItemInfo& getItem() {
        return m_Item;
    }

private:
    ObjectID_t m_OwnerObjectID;
    BYTE m_Index;
    StoreItemInfo m_Item;
};

//////////////////////////////////////////////////////////////////////////////
// class GCAddStoreItemFactory;
//////////////////////////////////////////////////////////////////////////////

class GCAddStoreItemFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_STORE_ITEM;
    static constexpr std::string_view kName = "GCAddStoreItem";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE + StoreItemInfo::getMaxSize()};

    Packet* createPacket() override {
        return new GCAddStoreItem();
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
