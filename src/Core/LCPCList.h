//////////////////////////////////////////////////////////////////////
//
// Filename    : LCPCList.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_PC_LIST_H__
#define __LC_PC_LIST_H__

// include files
#include "PCInfo.h"
#include "PCOustersInfo.h"
#include "PCSlayerInfo.h"
#include "PCVampireInfo.h"
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class LCPCList;
//
//////////////////////////////////////////////////////////////////////

class LCPCList : public Packet {
public:
    // constructor
    // PCInfo* 배열에 각각 NULL을 지정한다.
    LCPCList();

    // destructor
    // PCInfo* 배열에 할당된 객체를 삭제한다.
    ~LCPCList();

    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_PC_LIST;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const;

    // get packet's name
    string getPacketName() const {
        return "LCPCList";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set pc info
    PCInfo* getPCInfo(Slot slot) const {
        if (m_pPCInfos[slot] == NULL)
            throw NoSuchElementException("no such PC exist in that slot");

        return m_pPCInfos[slot];
    }

    void setPCInfo(Slot slot, PCInfo* pPCInfo) {
        if (m_pPCInfos[slot] != NULL)
            throw DuplicatedException("PCInfo duplicated.");
        m_pPCInfos[slot] = pPCInfo;
    }

private:
    // 캐릭터 정보
    PCInfo* m_pPCInfos[SLOT_MAX];
};


//////////////////////////////////////////////////////////////////////
//
// class LCPCListFactory;
//
// Factory for LCPCList
//
//////////////////////////////////////////////////////////////////////

class LCPCListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_PC_LIST;
    static constexpr std::string_view kName = "LCPCList";
    // Slayer info is the largest of the three races, so the packet is
    // biggest with SLOT_MAX slayers.
    static constexpr PacketSize_t kMaxSize{PCSlayerInfo::getMaxSize() * SLOT_MAX + SLOT_MAX};

    // create packet
    Packet* createPacket() override {
        return new LCPCList();
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
//
//////////////////////////////////////////////////////////////////////

#endif
