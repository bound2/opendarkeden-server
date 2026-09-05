//////////////////////////////////////////////////////////////////////
//
// Filename    : CGRequestUnion.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_REQUEST_UNION_H__
#define __CG_REQUEST_UNION_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGRequestUnion;
//
//////////////////////////////////////////////////////////////////////

class CGRequestUnion : public Packet {
public:
    CGRequestUnion(){};
    virtual ~CGRequestUnion(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_REQUEST_UNION;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID;
    }

    // get packet name
    string getPacketName() const {
        return "CGRequestUnion";
    }

    // get packet's debug string
    string toString() const;

    // get/set GuildID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t GuildID) {
        m_GuildID = GuildID;
    }


private:
    // Guild ID
    GuildID_t m_GuildID;
};


//////////////////////////////////////////////////////////////////////
//
// class CGRequestUnionFactory;
//
// Factory for CGRequestUnion
//
//////////////////////////////////////////////////////////////////////

class CGRequestUnionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_REQUEST_UNION;
    static constexpr std::string_view kName = "CGRequestUnion";
    static constexpr PacketSize_t kMaxSize{szGuildID};

    // constructor
    CGRequestUnionFactory() {}

    // destructor
    virtual ~CGRequestUnionFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGRequestUnion();
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
// class CGRequestUnionHandler;
//
//////////////////////////////////////////////////////////////////////

class CGRequestUnionHandler {
public:
    // execute packet's handler
    static void execute(CGRequestUnion* pCGRequestUnion, Player* pPlayer);
};

#endif
