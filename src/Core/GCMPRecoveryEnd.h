//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMPRecoveryEnd.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_MP_RECOVERY_END_H__
#define __GC_MP_RECOVERY_END_H__

// include files
#include "EffectInfo.h"
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCMPRecoveryEnd;
//
////////////////////////////////////////////////////////////////////

class GCMPRecoveryEnd : public Packet {
public:
    GCMPRecoveryEnd();

    virtual ~GCMPRecoveryEnd();

    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MP_RECOVERY_END;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // const static GCMPRecoveryEndPacketSize 를 정의, 리턴하라.
    PacketSize_t getPacketSize() const {
        return szMP;
    }

    // get packet's name
    string getPacketName() const {
        return "GCMPRecoveryEnd";
    }

    // get packet's debug string
    string toString() const;

public:
    // get /set CurrentMP
    MP_t getCurrentMP() const {
        return m_CurrentMP;
    }
    void setCurrentMP(MP_t CurrentMP) {
        m_CurrentMP = CurrentMP;
    }

private:
    // 현재 체력
    MP_t m_CurrentMP;
};


//////////////////////////////////////////////////////////////////////
//
// class GCMPRecoveryEndFactory;
//
// Factory for GCMPRecoveryEnd
//
//////////////////////////////////////////////////////////////////////

class GCMPRecoveryEndFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MP_RECOVERY_END;
    static constexpr std::string_view kName = "GCMPRecoveryEnd";
    static constexpr PacketSize_t kMaxSize{szMP};

    // create packet
    Packet* createPacket() override {
        return new GCMPRecoveryEnd();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // const static GCMPRecoveryEndPacketSize 를 정의, 리턴하라.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
