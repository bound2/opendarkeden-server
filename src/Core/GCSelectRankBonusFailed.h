//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCSelectRankBonusFailed.h
// Written By  :  elca@ewestsoft.com
// Description :  Å
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
    // ÀÔ·Â½ºÆ®¸²(¹öÆÛ)À¸·ÎºÎÅÍ µ¥ÀÌÅ¸¸¦ ÀÐ¾î¼­ ÆÐÅ¶À» ÃÊ±âÈ­ÇÑ´Ù.
    void read(SocketInputStream& iStream);

    // Ãâ·Â½ºÆ®¸²(¹öÆÛ)À¸·Î ÆÐÅ¶ÀÇ ¹ÙÀÌ³Ê¸® ÀÌ¹ÌÁö¸¦ º¸³½´Ù.
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
    DWORD m_RankBonusType;
    BYTE m_Desc; // ½ÇÆÐÄÚµå
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
