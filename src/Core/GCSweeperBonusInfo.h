//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSweeperBonusInfo.h
// Written By  :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SWEEPER_BONUS_INFO_H__
#define __GC_SWEEPER_BONUS_INFO_H__

// include files
#include <list>

#include "Packet.h"
#include "PacketFactory.h"
#include "SweeperBonusInfo.h"

typedef list<SweeperBonusInfo*> SweeperBonusInfoList;
typedef SweeperBonusInfoList::const_iterator SweeperBonusInfoListConstItor;

//////////////////////////////////////////////////////////////////////
//
// class GCSweeperBonusInfo;
//
//////////////////////////////////////////////////////////////////////

class GCSweeperBonusInfo : public Packet {
public:
    // constructor
    GCSweeperBonusInfo();

    // destructor
    ~GCSweeperBonusInfo();

    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SWEEPER_BONUS_INFO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const;

    // get packet name
    string getPacketName() const {
        return "GCSweeperBonusInfo";
    }

    // get packet's debug string
    string toString() const;

public:
    BYTE getListNum() const {
        return m_SweeperBonusInfoList.size();
    }

    void addSweeperBonusInfo(SweeperBonusInfo* pSweeperBonusInfo) {
        m_SweeperBonusInfoList.push_back(pSweeperBonusInfo);
    }

    void clearSweeperBonusInfoList();

    SweeperBonusInfo* popFrontSweeperBonusInfoList() {
        if (!m_SweeperBonusInfoList.empty()) {
            SweeperBonusInfo* pSweeperBonusInfo = m_SweeperBonusInfoList.front();
            m_SweeperBonusInfoList.pop_front();
            return pSweeperBonusInfo;
        }
        return NULL;
    }


private:
    SweeperBonusInfoList m_SweeperBonusInfoList;
};


//////////////////////////////////////////////////////////////////////
//
// class GCSweeperBonusInfoFactory;
//
// Factory for GCSweeperBonusInfo
//
//////////////////////////////////////////////////////////////////////

class GCSweeperBonusInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SWEEPER_BONUS_INFO;
    static constexpr std::string_view kName = "GCSweeperBonusInfo";
    static constexpr PacketSize_t kMaxSize{szBYTE + SweeperBonusInfo::getMaxSize() * 12};

    // create packet
    Packet* createPacket() override {
        return new GCSweeperBonusInfo();
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
    // const static GCSystemMessagePacketMaxSize 를 정의, 리턴하라.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class GCSweeperBonusInfo;
//
//////////////////////////////////////////////////////////////////////

#endif
