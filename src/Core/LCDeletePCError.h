//////////////////////////////////////////////////////////////////////
//
// Filename    : LCDeletePCError.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_DELETE_PC_ERROR_H__
#define __LC_DELETE_PC_ERROR_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class LCDeletePCError;
//
//////////////////////////////////////////////////////////////////////

class LCDeletePCError : public Packet {
public:
    LCDeletePCError(){};
    ~LCDeletePCError(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_DELETE_PC_ERROR;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "LCDeletePCError";
    }

    // get packet's debug string
    string toString() const;

    // get/set error message
    //	string getMessage() const  { return m_Message; }
    //	void setMessage(string message)  { m_Message = message; }
    // get /set ErrorID
    BYTE getErrorID() const {
        return m_ErrorID;
    }
    void setErrorID(BYTE ErrorID) {
        m_ErrorID = ErrorID;
    }

private:
    // 에러 메시지
    BYTE m_ErrorID;
};


//////////////////////////////////////////////////////////////////////
//
// class LCDeletePCErrorFactory;
//
// Factory for LCDeletePCError
//
//////////////////////////////////////////////////////////////////////

class LCDeletePCErrorFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_DELETE_PC_ERROR;
    static constexpr std::string_view kName = "LCDeletePCError";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    // create packet
    Packet* createPacket() override {
        return new LCDeletePCError();
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
