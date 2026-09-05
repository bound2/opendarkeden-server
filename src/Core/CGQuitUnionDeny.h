//////////////////////////////////////////////////////////////////////
//
// Filename    : CGQuitUnionDeny.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_QUIT_UNION_DENY_H__
#define __CG_QUIT_UNION_DENY_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGQuitUnionDeny;
//
//////////////////////////////////////////////////////////////////////

class CGQuitUnionDeny : public Packet {
public:
    CGQuitUnionDeny(){};
    ~CGQuitUnionDeny(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_QUIT_UNION_DENY;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID;
    }

    // get packet name
    string getPacketName() const {
        return "CGQuitUnionDeny";
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
// class CGQuitUnionDenyFactory;
//
// Factory for CGQuitUnionDeny
//
//////////////////////////////////////////////////////////////////////

class CGQuitUnionDenyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_QUIT_UNION_DENY;
    static constexpr std::string_view kName = "CGQuitUnionDeny";
    static constexpr PacketSize_t kMaxSize{szGuildID};

    // constructor
    CGQuitUnionDenyFactory() {}

    // destructor
    virtual ~CGQuitUnionDenyFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGQuitUnionDeny();
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
// class CGQuitUnionDenyHandler;
//
//////////////////////////////////////////////////////////////////////

class CGQuitUnionDenyHandler {
public:
    // execute packet's handler
    static void execute(CGQuitUnionDeny* pCGQuitUnionDeny, Player* pPlayer);
};

#endif
