//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSelectRankBonus.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_SELECT_RANK_BONUS_H__
#define __CG_SELECT_RANK_BONUS_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class CGSelectRankBonus;
//////////////////////////////////////////////////////////////////////////////

class CGSelectRankBonus : public Packet {
public:
    CGSelectRankBonus(){};
    virtual ~CGSelectRankBonus(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_SELECT_RANK_BONUS;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szDWORD;
    }

    // get packet name
    string getPacketName() const {
        return "CGSelectRankBonus";
    }

    // get packet's debug string
    string toString() const;

public:
    DWORD getRankBonusType() const {
        return m_RankBonusType;
    }
    void setRankBonusType(DWORD rankBonusType) {
        m_RankBonusType = rankBonusType;
    }

private:
    DWORD m_RankBonusType = 0; // Rank Bonus Type
};

//////////////////////////////////////////////////////////////////////
// class CGSelectRankBonusFactory;
//////////////////////////////////////////////////////////////////////

class CGSelectRankBonusFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SELECT_RANK_BONUS;
    static constexpr std::string_view kName = "CGSelectRankBonus";
    static constexpr PacketSize_t kMaxSize{szDWORD};

    // create packet
    Packet* createPacket() override {
        return new CGSelectRankBonus();
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
// class CGSelectRankBonusHandler;
//////////////////////////////////////////////////////////////////////

class CGSelectRankBonusHandler {
public:
    // execute packet's handler
    static void execute(CGSelectRankBonus* pCGSelectRankBonus, Player* pPlayer);
};

#endif
