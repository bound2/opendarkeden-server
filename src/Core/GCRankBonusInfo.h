//--------------------------------------------------------------------------------
//
// Filename    : GCRankBonusInfo.h
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __GC_RANK_BONUS_INFO_H__
#define __GC_RANK_BONUS_INFO_H__

// include files
#include "Assert1.h"
#include "Packet.h"
#include "PacketFactory.h"

const DWORD EndOfRankBonus = 9999;


class GCRankBonusInfo : public Packet {
public:
    // constructor
    GCRankBonusInfo();

    // destructor
    ~GCRankBonusInfo();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_RANK_BONUS_INFO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + (szDWORD * m_RankBonusInfoList.size());
    }

    // get packet name
    string getPacketName() const {
        return "GCRankBonusInfo";
    }

    // get packet's debug string
    string toString() const;

    //--------------------------------------------------
    // methods
    //--------------------------------------------------
public:
    // The count travels in a BYTE and the factory max budgets this many
    // bonuses.
    static constexpr size_t kMaxEntries = 100;

    BYTE getListNum() const {
        return m_RankBonusInfoList.size();
    }

    // add
    void addListElement(DWORD rankBonusType) {
        if (m_RankBonusInfoList.size() >= kMaxEntries)
            throw InvalidProtocolException("too many rank bonuses");
        m_RankBonusInfoList.push_back(rankBonusType);
    }

    // pop front Element in Status List
    DWORD popFrontListElement() {
        if (!m_RankBonusInfoList.empty()) {
            DWORD temp = m_RankBonusInfoList.front();
            m_RankBonusInfoList.pop_front();
            return temp;
        } else
            return EndOfRankBonus;
    }

private:
    // Rank Bonus List
    list<DWORD> m_RankBonusInfoList;
};


//--------------------------------------------------------------------------------
//
// class GCRankBonusInfoFactory;
//
// Factory for GCRankBonusInfo
//
//--------------------------------------------------------------------------------

class GCRankBonusInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_RANK_BONUS_INFO;
    static constexpr std::string_view kName = "GCRankBonusInfo";
    static constexpr PacketSize_t kMaxSize{szBYTE + (szDWORD * GCRankBonusInfo::kMaxEntries)};

    // create packet
    Packet* createPacket() override {
        return new GCRankBonusInfo();
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
    // Define and return const static GCRankBonusInfoPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
