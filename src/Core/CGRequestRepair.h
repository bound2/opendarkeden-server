//-----------------------------------------------------------------------------
//
// Filename    : CGRequestRepair.h
// Written By  : 김성민
// Description :
//
//-----------------------------------------------------------------------------

#ifndef __CG_REQUEST_REPAIR_H__
#define __CG_REQUEST_REPAIR_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//--------------------------------------------------------------------------------
//
// class CGRequestRepair;
//
//--------------------------------------------------------------------------------

class CGRequestRepair : public Packet {
public:
    CGRequestRepair(){};
    virtual ~CGRequestRepair(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_REQUEST_REPAIR;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "CGRequestRepair";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set ObjectID
    ObjectID_t getObjectID() {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

private:
    // Item Object ID
    ObjectID_t m_ObjectID;
};


//-----------------------------------------------------------------------------
//
// class CGRequestRepairFactory;
//
// Factory for CGRequestRepair
//
//-----------------------------------------------------------------------------

class CGRequestRepairFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_REQUEST_REPAIR;
    static constexpr std::string_view kName = "CGRequestRepair";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // create packet
    Packet* createPacket() override {
        return new CGRequestRepair();
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


//--------------------------------------------------------------------------------
//
// class CGRequestRepairHandler;
//
//--------------------------------------------------------------------------------

class CGRequestRepairHandler {
public:
    // execute packet's handler
    static void execute(CGRequestRepair* pPacket, Player* player);
    static void executeNormal(CGRequestRepair* pPacket, Player* player);
    static void executeMotorcycle(CGRequestRepair* pPacket, Player* player);
    static void executeAll(CGRequestRepair* pPacket, Player* player);
};

#endif
