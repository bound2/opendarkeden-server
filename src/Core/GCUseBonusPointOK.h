//////////////////////////////////////////////////////////////////////
//
// Filename    : GCUseBonusPointOK.h
// Written By  : crazydog
// Description : vamp가 bonus사용을 허가 받다.
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_USE_BONUS_POINT_OK_H__
#define __GC_USE_BONUS_POINT_OK_H__

// include files
#include "ModifyInfo.h"
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCUseBonusPointOK;
//
//////////////////////////////////////////////////////////////////////

class GCUseBonusPointOK : public ModifyInfo {
public:
    // Constructor
    GCUseBonusPointOK();

    // Desctructor
    ~GCUseBonusPointOK();

    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_USE_BONUS_POINT_OK;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // const static GCUseBonusPointOKPacketSize 를 정의해서 리턴하라.
    PacketSize_t getPacketSize() const {
        return ModifyInfo::getPacketSize();
    }

    // get packet name
    string getPacketName() const {
        return "GCUseBonusPointOK";
    }

    // get packet's debug string
    string toString() const;
};


//////////////////////////////////////////////////////////////////////
//
// class GCUseBonusPointOKFactory;
//
// Factory for GCUseBonusPointOK
//
//////////////////////////////////////////////////////////////////////

class GCUseBonusPointOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_USE_BONUS_POINT_OK;
    static constexpr std::string_view kName = "GCUseBonusPointOK";
    static constexpr PacketSize_t kMaxSize{ModifyInfo::getPacketMaxSize()};

    // create packet
    Packet* createPacket() override {
        return new GCUseBonusPointOK();
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
    // *OPTIMIZATION HINT*
    // const static GCUseBonusPointOKPacketSize 를 정의해서 리턴하라.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
