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

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
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
    // The count travels in a BYTE and the factory max budgets this many
    // bonuses.
    static constexpr size_t kMaxEntries = 12;

    BYTE getListNum() const {
        return m_SweeperBonusInfoList.size();
    }

    void addSweeperBonusInfo(SweeperBonusInfo* pSweeperBonusInfo) {
        if (m_SweeperBonusInfoList.size() >= kMaxEntries)
            throw InvalidProtocolException("too many sweeper bonuses");
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
    static constexpr PacketSize_t kMaxSize{szBYTE + SweeperBonusInfo::getMaxSize() * GCSweeperBonusInfo::kMaxEntries};

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
    // Define and return const static GCSystemMessagePacketMaxSize.
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
