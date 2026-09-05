//////////////////////////////////////////////////////////////////////
//
// Filename    : CGStoreClose.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_STORE_CLOSE_H__
#define __CG_STORE_CLOSE_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGStoreClose;
//
//////////////////////////////////////////////////////////////////////

class CGStoreClose : public Packet {
public:
    CGStoreClose(){};
    virtual ~CGStoreClose(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_STORE_CLOSE;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet name
    string getPacketName() const {
        return "CGStoreClose";
    }

    // get packet's debug string
    string toString() const;
};


//////////////////////////////////////////////////////////////////////
//
// class CGStoreCloseFactory;
//
// Factory for CGStoreClose
//
//////////////////////////////////////////////////////////////////////

class CGStoreCloseFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_STORE_CLOSE;
    static constexpr std::string_view kName = "CGStoreClose";
    static constexpr PacketSize_t kMaxSize{0};

    // constructor
    CGStoreCloseFactory() {}

    // destructor
    virtual ~CGStoreCloseFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGStoreClose();
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
// class CGStoreCloseHandler;
//
//////////////////////////////////////////////////////////////////////

class CGStoreCloseHandler {
public:
    // execute packet's handler
    static void execute(CGStoreClose* pCGStoreClose, Player* pPlayer);
};

#endif
