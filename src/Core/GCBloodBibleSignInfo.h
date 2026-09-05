//////////////////////////////////////////////////////////////////////////////
// Filename    : GCBloodBibleSignInfo.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_BLOOD_BIBLE_SIGN_INFO_H__
#define __GC_BLOOD_BIBLE_SIGN_INFO_H__

#include "BloodBibleSignInfo.h"
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCBloodBibleSignInfo;
//////////////////////////////////////////////////////////////////////////////

class GCBloodBibleSignInfo : public Packet {
public:
    GCBloodBibleSignInfo() {}
    virtual ~GCBloodBibleSignInfo();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_BLOOD_BIBLE_SIGN_INFO;
    }
    PacketSize_t getPacketSize() const {
        return m_pInfo->getSize();
    }
    string getPacketName() const {
        return "GCBloodBibleSignInfo";
    }
    string toString() const;

public:
    BloodBibleSignInfo* getSignInfo() const {
        return m_pInfo;
    }
    void setSignInfo(BloodBibleSignInfo* pInfo) {
        m_pInfo = pInfo;
    }

private:
    BloodBibleSignInfo* m_pInfo;
};

//////////////////////////////////////////////////////////////////////////////
// class GCBloodBibleSignInfoFactory;
//////////////////////////////////////////////////////////////////////////////

class GCBloodBibleSignInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_BLOOD_BIBLE_SIGN_INFO;
    static constexpr std::string_view kName = "GCBloodBibleSignInfo";
    static constexpr PacketSize_t kMaxSize{BloodBibleSignInfo::getMaxSize()};

    Packet* createPacket() override {
        return new GCBloodBibleSignInfo();
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
