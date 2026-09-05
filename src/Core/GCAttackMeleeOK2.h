//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAttackMeleeOK2.h
// Written By  : elca@ewestsoft.com
// Description : 기술이 성공했을때 보내는 패킷을 위한 클래스 정의
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_ATTACK_MELEE_OK_2_H__
#define __GC_ATTACK_MELEE_OK_2_H__

// include files
#include "Exception.h"
#include "ModifyInfo.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCAttackMeleeOK2;
//
// 게임서버에서 클라이언트로 자신의 기술이 성공을 알려주기 위한 클래스
//
//////////////////////////////////////////////////////////////////////

class GCAttackMeleeOK2 : public ModifyInfo {
public:
    // constructor
    GCAttackMeleeOK2();

    // destructor
    ~GCAttackMeleeOK2();


public:
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ATTACK_MELEE_OK_2;
    }

    // get packet's body size
    // 최적화시, 미리 계산된 정수를 사용한다.
    PacketSize_t getPacketSize() const {
        return szObjectID + ModifyInfo::getPacketSize();
    }

    // get packet's name
    string getPacketName() const {
        return "GCAttackMeleeOK2";
    }

    // get packet's debug string
    string toString() const;

    // get / set CEffectID
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
// class GCAttackMeleeOK2Factory;
//
// Factory for GCAttackMeleeOK2
//
//////////////////////////////////////////////////////////////////////

class GCAttackMeleeOK2Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ATTACK_MELEE_OK_2;
    static constexpr std::string_view kName = "GCAttackMeleeOK2";
    static constexpr PacketSize_t kMaxSize{szObjectID + ModifyInfo::getPacketMaxSize()};

    // constructor
    GCAttackMeleeOK2Factory() {}

    // destructor
    virtual ~GCAttackMeleeOK2Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCAttackMeleeOK2();
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
