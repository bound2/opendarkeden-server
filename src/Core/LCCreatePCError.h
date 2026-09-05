//////////////////////////////////////////////////////////////////////////////
// Filename    : LCCreatePCError.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __LC_CREATE_PC_ERROR_H__
#define __LC_CREATE_PC_ERROR_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class LCCreatePCError;
//
// PC Creation 이 실패했을 경우, 로그인 서버는 클라이언트에게 이 패킷을
// 보낸다.
//////////////////////////////////////////////////////////////////////////////

class LCCreatePCError : public Packet {
public:
    LCCreatePCError(){};
    ~LCCreatePCError(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_LC_CREATE_PC_ERROR;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }
    string getPacketName() const {
        return "LCCreatePCError";
    }
    string toString() const;

public:
    BYTE getErrorID() const {
        return m_ErrorID;
    }
    void setErrorID(BYTE ErrorID) {
        m_ErrorID = ErrorID;
    }

private:
    BYTE m_ErrorID;
};

//////////////////////////////////////////////////////////////////////////////
// class LCCreatePCErrorFactory;
//////////////////////////////////////////////////////////////////////////////

class LCCreatePCErrorFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_CREATE_PC_ERROR;
    static constexpr std::string_view kName = "LCCreatePCError";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    // create packet
    Packet* createPacket() override {
        return new LCCreatePCError();
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
