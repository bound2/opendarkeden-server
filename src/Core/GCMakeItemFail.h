//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMakeItemFail.h
// Written By  : elca@ewestsoft.com
// Description : 기술이 성공했을때 보내는 패킷을 위한 클래스 정의
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_MAKE_ITEM_FAIL_H__
#define __GC_MAKE_ITEM_FAIL_H__

// include files
#include "Exception.h"
#include "GCChangeInventoryItemNum.h"
#include "ModifyInfo.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCMakeItemFail;
//
// 게임서버에서 클라이언트로 자신의 기술이 성공을 알려주기 위한 클래스
//
//////////////////////////////////////////////////////////////////////

class GCMakeItemFail : public GCChangeInventoryItemNum, public ModifyInfo {
public:
    // constructor
    GCMakeItemFail();

    // destructor
    ~GCMakeItemFail();


public:
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MAKE_ITEM_FAIL;
    }

    // get packet's body size
    // 최적화시, 미리 계산된 정수를 사용한다.
    PacketSize_t getPacketSize() const {
        return GCChangeInventoryItemNum::getPacketSize() + ModifyInfo::getPacketSize();
    }

    // get packet's name
    string getPacketName() const {
        return "GCMakeItemFail";
    }

    // get packet's debug string
    string toString() const;

private:
};


//////////////////////////////////////////////////////////////////////
//
// class GCMakeItemFailFactory;
//
// Factory for GCMakeItemFail
//
//////////////////////////////////////////////////////////////////////

class GCMakeItemFailFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MAKE_ITEM_FAIL;
    static constexpr std::string_view kName = "GCMakeItemFail";
    static constexpr PacketSize_t kMaxSize{255 + ModifyInfo::getPacketMaxSize()};

    // constructor
    GCMakeItemFailFactory() {}

    // destructor
    virtual ~GCMakeItemFailFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCMakeItemFail();
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
    // PacketSize_t getPacketMaxSize() const  { return szSkillType + szCEffectID + szDuration + szBYTE + szBYTE*
    // m_ListNum* 2 ; }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
