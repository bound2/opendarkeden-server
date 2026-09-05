//////////////////////////////////////////////////////////////////////
//
// Filename    : GCUnburrowFail.h
// Written By  : crazydog
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_UNBURROW_FAIL_H__
#define __GC_UNBURROW_FAIL_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class GCUnburrowFail;
//
//////////////////////////////////////////////////////////////////////

class GCUnburrowFail : public Packet {
public:
    // constructor
    GCUnburrowFail() {}
    ~GCUnburrowFail(){};


public:
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_UNBURROW_FAIL;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "GCUnburrowFail";
    }

    // get packet's debug string
    string toString() const;


public:
private:
};


//////////////////////////////////////////////////////////////////////
//
// class  GCUnburrowFailFactory;
//
// Factory for  GCUnburrowFail
//
//////////////////////////////////////////////////////////////////////

class GCUnburrowFailFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_UNBURROW_FAIL;
    static constexpr std::string_view kName = "GCUnburrowFail";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new GCUnburrowFail();
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


#endif
