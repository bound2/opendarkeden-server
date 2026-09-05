//////////////////////////////////////////////////////////////////////////////
// Filename    : GCMyStoreInfo.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_MY_STORE_INFO_H__
#define __GC_MY_STORE_INFO_H__

#include "Packet.h"
#include "PacketFactory.h"
#include "StoreInfo.h"

//////////////////////////////////////////////////////////////////////////////
// class GCMyStoreInfo;
//////////////////////////////////////////////////////////////////////////////

class GCMyStoreInfo : public Packet {
public:
    GCMyStoreInfo() : m_OpenUI(1) {}
    virtual ~GCMyStoreInfo();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_MY_STORE_INFO;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE + m_pInfo->getSize(false);
    }
    string getPacketName() const {
        return "GCMyStoreInfo";
    }
    string toString() const;

public:
    BYTE getOpenUI() const {
        return m_OpenUI;
    }
    void setOpenUI(BYTE ui) {
        m_OpenUI = ui;
    }

    StoreInfo* getStoreInfo() const {
        return m_pInfo;
    }
    void setStoreInfo(StoreInfo* pInfo) {
        m_pInfo = pInfo;
    }

private:
    BYTE m_OpenUI;
    StoreInfo* m_pInfo;
};

//////////////////////////////////////////////////////////////////////////////
// class GCMyStoreInfoFactory;
//////////////////////////////////////////////////////////////////////////////

class GCMyStoreInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MY_STORE_INFO;
    static constexpr std::string_view kName = "GCMyStoreInfo";
    static constexpr PacketSize_t kMaxSize{szBYTE + StoreInfo::getMaxSize()};

    Packet* createPacket() override {
        return new GCMyStoreInfo();
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
