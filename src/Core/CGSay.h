//////////////////////////////////////////////////////////////////////
//
// Filename    : CGSay.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_SAY_H__
#define __CG_SAY_H__

// include files


#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class CGSay;
//
// 클라이언트가 서버에게 보내는 Say 패킷이다.
// 내부에 Say String 만을 데이타 필드로 가진다.
//
//////////////////////////////////////////////////////////////////////

class Player;
class Creature;
class GamePlayer;

class CGSay : public Packet {
public:
    CGSay(){};
    virtual ~CGSay(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_SAY;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szuint + de::wire::stringWireSize(m_Message);
    }

    // get packet name
    string getPacketName() const {
        return "CGSay";
    }

    // get packet's debug string
    string toString() const;

    // get/set text color
    uint getColor() const {
        return m_Color;
    }
    void setColor(uint color) {
        m_Color = color;
    }

    // get/set chatting message
    const string& getMessage() const {
        return m_Message;
    }
    void setMessage(const string& msg) {
        m_Message = msg;
    }


private:
    // text color
    uint m_Color;

    // chatting message
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class CGSayFactory;
//
// Factory for CGSay
//
//////////////////////////////////////////////////////////////////////

class CGSayFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SAY;
    static constexpr std::string_view kName = "CGSay";
    static constexpr PacketSize_t kMaxSize{szuint + szBYTE + 128};

    // create packet
    Packet* createPacket() override {
        return new CGSay();
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
    // message 의 최대 크기에 대한 설정이 필요하다.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGSayHandler;
//
//////////////////////////////////////////////////////////////////////

class CGSayHandler {
public:
    // execute packet's handler
    static void execute(CGSay* pPacket, Player* pPlayer);
};

#endif
