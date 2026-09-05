////////////////////////////////////////////////////////////////////////////////
// Filename    : GCTradeMoney.h
// Written By  : 김성민
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __GC_TRADE_MONEY_H__
#define __GC_TRADE_MONEY_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
// 교환 코드
////////////////////////////////////////////////////////////////////////////////

enum {
    // 상대방이 교환할 돈의 액수를 늘렸다.
    GC_TRADE_MONEY_INCREASE = 0,

    // 상대방이 교환할 돈의 액수를 줄였다.
    GC_TRADE_MONEY_DECREASE,

    // 실제로 인벤토리에서 빼낸 금액
    GC_TRADE_MONEY_INCREASE_RESULT,

    // 실제로 인벤토리에다 더한 금액
    GC_TRADE_MONEY_DECREASE_RESULT
};

////////////////////////////////////////////////////////////////////////////////
//
// class GCTradeMoney;
//
////////////////////////////////////////////////////////////////////////////////

class GCTradeMoney : public Packet {
public:
    GCTradeMoney(){};
    ~GCTradeMoney(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_TRADE_MONEY;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szGold + szBYTE;
    }
    string getPacketName() const {
        return "GCTradeMoney";
    }
    string toString() const;

public:
    ObjectID_t getTargetObjectID() const {
        return m_TargetObjectID;
    }
    void setTargetObjectID(ObjectID_t id) {
        m_TargetObjectID = id;
    }

    Gold_t getAmount() const {
        return m_Gold;
    }
    void setAmount(Gold_t gold) {
        m_Gold = gold;
    }

    BYTE getCode() const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

private:
    ObjectID_t m_TargetObjectID; // 교환을 원하는 상대방의 ObjectID
    Gold_t m_Gold;               // 원하는 액수
    BYTE m_Code;                 // 코드
};


////////////////////////////////////////////////////////////////////////////////
//
// class GCTradeMoneyFactory;
//
////////////////////////////////////////////////////////////////////////////////

class GCTradeMoneyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_TRADE_MONEY;
    static constexpr std::string_view kName = "GCTradeMoney";
    static constexpr PacketSize_t kMaxSize{szObjectID + szGold + szBYTE};

    Packet* createPacket() override {
        return new GCTradeMoney();
    }
    string getPacketName() const override {
        return string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////

#endif
