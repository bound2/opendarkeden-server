//////////////////////////////////////////////////////////////////////
//
// Filename    : GCPhoneConnectionFailed.h
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_PHONE_CONNECTION_FAILED_H__
#define __GC_PHONE_CONNECTION_FAILED_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCPhoneConnectionFailed;
//
// 게임 서버에서 특정 사용자가 움직였다는 정보를 클라이언트로 보내줄
// 때 사용하는 패킷 객체이다.(CreatureID,X,Y,DIR) 을 포함한다.
//
//////////////////////////////////////////////////////////////////////

class GCPhoneConnectionFailed : public Packet {
public:
    // constructor
    GCPhoneConnectionFailed();

    // destructor
    ~GCPhoneConnectionFailed();


public:
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_PHONE_CONNECTION_FAILED;
    }

    // get packet's body size
    // 최적화시, 미리 계산된 정수를 사용한다.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "GCPhoneConnectionFailed";
    }

    // get packet's debug string
    string toString() const;

private:
};

//////////////////////////////////////////////////////////////////////
//
// class GCPhoneConnectionFailedFactory;
//
// Factory for GCPhoneConnectionFailed
//
//////////////////////////////////////////////////////////////////////

class GCPhoneConnectionFailedFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_PHONE_CONNECTION_FAILED;
    static constexpr std::string_view kName = "GCPhoneConnectionFailed";
    static constexpr PacketSize_t kMaxSize{0};

    // constructor
    GCPhoneConnectionFailedFactory() {}

    // destructor
    virtual ~GCPhoneConnectionFailedFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCPhoneConnectionFailed();
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
