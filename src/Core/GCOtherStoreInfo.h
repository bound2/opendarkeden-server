//////////////////////////////////////////////////////////////////////////////
// Filename    : GCOtherStoreInfo.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_OTHER_STORE_INFO_H__
#define __GC_OTHER_STORE_INFO_H__

#include "Packet.h"
#include "PacketFactory.h"
#include "StoreInfo.h"

//////////////////////////////////////////////////////////////////////////////
// class GCOtherStoreInfo;
//////////////////////////////////////////////////////////////////////////////

class GCOtherStoreInfo : public Packet {
public:
    GCOtherStoreInfo() {}
    virtual ~GCOtherStoreInfo();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_OTHER_STORE_INFO;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE + m_pInfo->getSize(true);
    }
    string getPacketName() const {
        return "GCOtherStoreInfo";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t oid) {
        m_ObjectID = oid;
    }

    BYTE isRequested() const {
        return m_IsRequested;
    }
    void setRequested(BYTE flag) {
        m_IsRequested = flag;
    }

    StoreInfo* getStoreInfo() const {
        return m_pInfo;
    }
    void setStoreInfo(StoreInfo* pInfo) {
        m_pInfo = pInfo;
    }

private:
    ObjectID_t m_ObjectID;
    BYTE m_IsRequested;
    StoreInfo* m_pInfo;
};

//////////////////////////////////////////////////////////////////////////////
// class GCOtherStoreInfoFactory;
//////////////////////////////////////////////////////////////////////////////

class GCOtherStoreInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_OTHER_STORE_INFO;
    static constexpr std::string_view kName = "GCOtherStoreInfo";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE + StoreInfo::getMaxSize()};

    Packet* createPacket() override {
        return new GCOtherStoreInfo();
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
