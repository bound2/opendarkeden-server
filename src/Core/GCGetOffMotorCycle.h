//////////////////////////////////////////////////////////////////////
//
// Filename    : GCGetOffMotorCycle.h
// Written By  : elca@ewestsoft.com
// Description : 기술이 성공했을때 보내는 패킷을 위한 클래스 정의
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_GET_OFF_MOTORCYCLE_H__
#define __GC_GET_OFF_MOTORCYCLE_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCGetOffMotorCycle;
//
// 게임서버에서 클라이언트로 자신의 기술이 성공을 알려주기 위한 클래스
//
//////////////////////////////////////////////////////////////////////

class GCGetOffMotorCycle : public Packet {
public:
    // constructor
    GCGetOffMotorCycle();

    // destructor
    ~GCGetOffMotorCycle();


public:
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_GET_OFF_MOTORCYCLE;
    }

    // get packet's body size
    // 최적화시, 미리 계산된 정수를 사용한다.
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet's name
    string getPacketName() const {
        return "GCGetOffMotorCycle";
    }

    // get packet's debug string
    string toString() const;

    // get / set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

private:
    // ObjectID
    ObjectID_t m_ObjectID;
};


//////////////////////////////////////////////////////////////////////
//
// class GCGetOffMotorCycleFactory;
//
// Factory for GCGetOffMotorCycle
//
//////////////////////////////////////////////////////////////////////

class GCGetOffMotorCycleFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GET_OFF_MOTORCYCLE;
    static constexpr std::string_view kName = "GCGetOffMotorCycle";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // constructor
    GCGetOffMotorCycleFactory() {}

    // destructor
    virtual ~GCGetOffMotorCycleFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCGetOffMotorCycle();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
