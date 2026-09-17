//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCSelectRankBonusFailed.h
// Written By  :  elca@ewestsoft.com
// Description :  Packet
//
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SELECT_RANK_BONUS_FAILED_H__
#define __GC_SELECT_RANK_BONUS_FAILED_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCSelectRankBonusFailed;
//
//////////////////////////////////////////////////////////////////////

class GCSelectRankBonusFailed : public Packet {
public:
    GCSelectRankBonusFailed();
    virtual ~GCSelectRankBonusFailed();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SELECT_RANK_BONUS_FAILED;
    }

    // get packet size
    PacketSize_t getPacketSize() const {
        return szDWORD + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCSelectRankBonusFailed";
    }

    // get packet's debug string
    string toString() const;

    // get/set skill type
    DWORD getRankBonusType() const {
        return m_RankBonusType;
    }
    void setRankBonusType(DWORD rankBonusType) {
        m_RankBonusType = rankBonusType;
    }

    // get/set description
    BYTE getDesc(void) const {
        return m_Desc;
    }
    void setDesc(BYTE desc) {
        m_Desc = desc;
    }

private:
    DWORD m_RankBonusType = 0;
    BYTE m_Desc = 0; // Failure code
};


//////////////////////////////////////////////////////////////////////
//
// class  GCSelectRankBonusFailedFactory;
//
// Factory for  GCSelectRankBonusFailed
//
//////////////////////////////////////////////////////////////////////

class GCSelectRankBonusFailedFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SELECT_RANK_BONUS_FAILED;
    static constexpr std::string_view kName = "GCSelectRankBonusFailed";
    static constexpr PacketSize_t kMaxSize{szDWORD + szBYTE};

    // constructor
    GCSelectRankBonusFailedFactory() {}

    // destructor
    virtual ~GCSelectRankBonusFailedFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCSelectRankBonusFailed();
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


#endif // __GC_LEARN_SKILL_FAILED_H__
