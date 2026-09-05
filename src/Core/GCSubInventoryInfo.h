//////////////////////////////////////////////////////////////////////////////
// Filename    : GCSubInventoryInfo.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_SUB_INVENTORY_INFO_H__
#define __GC_SUB_INVENTORY_INFO_H__

#include "InventoryInfo.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

class GCSubInventoryInfo : public Packet {
public:
    GCSubInventoryInfo();
    virtual ~GCSubInventoryInfo();

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_SUB_INVENTORY_INFO;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + m_pInventoryInfo->getSize();
    }
    string getPacketName() const {
        return "GCSubInventoryInfo";
    }
    string toString() const;

public:
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }

    void setInventoryInfo(InventoryInfo* pInventoryInfo) {
        m_pInventoryInfo = pInventoryInfo;
    }
    InventoryInfo* getInventoryInfo() const {
        return m_pInventoryInfo;
    }

private:
    ObjectID_t m_ObjectID;
    InventoryInfo* m_pInventoryInfo;
};

class GCSubInventoryInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SUB_INVENTORY_INFO;
    static constexpr std::string_view kName = "GCSubInventoryInfo";
    static constexpr PacketSize_t kMaxSize{szObjectID + InventoryInfo::getMaxSize()};

    Packet* createPacket() override {
        return new GCSubInventoryInfo();
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

#endif // __GC_SUB_INVENTORY_INFO_H__
