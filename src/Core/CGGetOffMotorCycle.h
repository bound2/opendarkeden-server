//////////////////////////////////////////////////////////////////////
//
// Filename    : CGGetOffMotorCycle.h
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_GET_OFF_MOTORCYCLE_H__
#define __CG_GET_OFF_MOTORCYCLE_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGGetOffMotorCycle;
//
//////////////////////////////////////////////////////////////////////

class CGGetOffMotorCycle : public Packet {
public:
    // constructor
    CGGetOffMotorCycle();

    // destructor
    ~CGGetOffMotorCycle();


public:
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_GET_OFF_MOTORCYCLE;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "CGGetOffMotorCycle";
    }

    // get/set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get packet's debug string
    string toString() const;

private:
    // ObjectID
    ObjectID_t m_ObjectID;
};


//////////////////////////////////////////////////////////////////////
//
// class CGGetOffMotorCycleFactory;
//
// Factory for CGGetOffMotorCycle
//
//////////////////////////////////////////////////////////////////////

class CGGetOffMotorCycleFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_GET_OFF_MOTORCYCLE;
    static constexpr std::string_view kName = "CGGetOffMotorCycle";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // constructor
    CGGetOffMotorCycleFactory() {}

    // destructor
    virtual ~CGGetOffMotorCycleFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGGetOffMotorCycle();
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
// class CGGetOffMotorCycleHandler;
//
//////////////////////////////////////////////////////////////////////

class CGGetOffMotorCycleHandler {
public:
    // execute packet's handler
    static void execute(CGGetOffMotorCycle* pCGGetOffMotorCycle, Player* pPlayer);
};

#endif
