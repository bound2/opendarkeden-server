//////////////////////////////////////////////////////////////////////
//
// Filename    : LCRegisterPlayerError.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_REGISTER_PLAYER_ERROR_H__
#define __LC_REGISTER_PLAYER_ERROR_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class LCRegisterPlayerError;
//
//////////////////////////////////////////////////////////////////////

class LCRegisterPlayerError : public Packet {
public:
    LCRegisterPlayerError(){};
    ~LCRegisterPlayerError(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_REGISTER_PLAYER_ERROR;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "LCRegisterPlayerError";
    }

    // get packet's debug string
    string toString() const;

    BYTE getErrorID() const {
        return m_ErrorID;
    }
    void setErrorID(BYTE ErrorID) {
        m_ErrorID = ErrorID;
    }

private:
    // 에러 ID
    BYTE m_ErrorID;
};


//////////////////////////////////////////////////////////////////////
//
// class LCRegisterPlayerErrorFactory;
//
// Factory for LCRegisterPlayerError
//
//////////////////////////////////////////////////////////////////////

class LCRegisterPlayerErrorFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_REGISTER_PLAYER_ERROR;
    static constexpr std::string_view kName = "LCRegisterPlayerError";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    // create packet
    Packet* createPacket() override {
        return new LCRegisterPlayerError();
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
