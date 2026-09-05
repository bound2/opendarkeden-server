//////////////////////////////////////////////////////////////////////
//
// Filename    : CLChangeServer.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CL_CHANGE_SERVER_H__
#define __CL_CHANGE_SERVER_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class CLChangeServer;
//
//////////////////////////////////////////////////////////////////////

class CLChangeServer : public Packet {
public:
    CLChangeServer(){};
    virtual ~CLChangeServer(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_CHANGE_SERVER;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szServerGroupID;
    }

    // get packet name
    string getPacketName() const {
        return "CLChangeServer";
    }

    // get / set ServerGroupID
    ServerGroupID_t getServerGroupID() const {
        return m_ServerGroupID;
    }
    void setServerGroupID(ServerGroupID_t ServerGroupID) {
        m_ServerGroupID = ServerGroupID;
    }

    // get packet's debug string
    string toString() const {
        return "CLChangeServer";
    }

private:
    ServerGroupID_t m_ServerGroupID;
};


//////////////////////////////////////////////////////////////////////
//
// class CLChangeServerFactory;
//
// Factory for CLChangeServer
//
//////////////////////////////////////////////////////////////////////

class CLChangeServerFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_CHANGE_SERVER;
    static constexpr std::string_view kName = "CLChangeServer";
    static constexpr PacketSize_t kMaxSize{szServerGroupID};

    // create packet
    Packet* createPacket() override {
        return new CLChangeServer();
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
// class CLChangeServerHandler;
//
//////////////////////////////////////////////////////////////////////

class CLChangeServerHandler {
public:
    // execute packet's handler
    static void execute(CLChangeServer* pPacket, Player* player);
};

#endif
