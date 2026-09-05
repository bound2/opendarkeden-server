//////////////////////////////////////////////////////////////////////
//
// Filename    : LCCreatePCOK.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_CREATE_PC_OK_H__
#define __LC_CREATE_PC_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class LCCreatePCOK;
//
// 로그인서버가 클라이언트에게 로그인 성공을 알려주는 패킷이다.
//
//////////////////////////////////////////////////////////////////////

class LCCreatePCOK : public Packet {
public:
    LCCreatePCOK(){};
    ~LCCreatePCOK(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream) {}

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const {}


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_CREATE_PC_OK;
    }

    // get packet body size
    // *OPTIMIZATION HINT*
    // const static LCCreatePCOKPacketSize 를 정의, 리턴하라.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "LCCreatePCOK";
    }

    // get packet's debug string
    string toString() const {
        return "LCCreatePCOK";
    }
};


//////////////////////////////////////////////////////////////////////
//
// class LCCreatePCOKFactory;
//
// Factory for LCCreatePCOK
//
//////////////////////////////////////////////////////////////////////

class LCCreatePCOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_CREATE_PC_OK;
    static constexpr std::string_view kName = "LCCreatePCOK";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new LCCreatePCOK();
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
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
