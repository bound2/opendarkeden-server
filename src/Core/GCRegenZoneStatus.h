//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRegenZoneStatus.h
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_REGEN_ZONE_STATUS_H__
#define __GC_REGEN_ZONE_STATUS_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class GCRegenZoneStatus;
//
//
//////////////////////////////////////////////////////////////////////

class GCRegenZoneStatus : public Packet {
public:
    // The eight regen zones write() emits and read() takes back.
    static constexpr uint kZoneCount = 8;

    GCRegenZoneStatus() {}
    ~GCRegenZoneStatus(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_REGEN_ZONE_STATUS;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE * kZoneCount;
    }

    // get packet name
    string getPacketName() const {
        return "GCRegenZoneStatus";
    }

    // get packet's debug string
    string toString() const;

    BYTE getStatus(uint index) const {
        if (index >= kZoneCount)
            throw InvalidProtocolException("regen zone index out of range");
        return m_Statuses[index];
    }
    void setStatus(uint index, BYTE status) {
        if (index >= kZoneCount)
            throw InvalidProtocolException("regen zone index out of range");
        m_Statuses[index] = status;
    }

private:
    BYTE m_Statuses[kZoneCount] = {};
};


//////////////////////////////////////////////////////////////////////
//
// class GCRegenZoneStatusFactory;
//
// Factory for GCRegenZoneStatus
//
//////////////////////////////////////////////////////////////////////

class GCRegenZoneStatusFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_REGEN_ZONE_STATUS;
    static constexpr std::string_view kName = "GCRegenZoneStatus";
    static constexpr PacketSize_t kMaxSize{szBYTE * GCRegenZoneStatus::kZoneCount};

    // create packet
    Packet* createPacket() override {
        return new GCRegenZoneStatus();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's max body size
    // *OPTIMIZATION HINT*
    // const static GCRegenZoneStatusPacketMaxSize 를 정의, 리턴하라.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
