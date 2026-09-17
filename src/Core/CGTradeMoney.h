////////////////////////////////////////////////////////////////////////////////
// Filename    : CGTradeMoney.h
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_TRADE_MONEY_H__
#define __CG_TRADE_MONEY_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
// Exchange code
////////////////////////////////////////////////////////////////////////////////

enum {
    // Increases the amount of money to exchange.
    CG_TRADE_MONEY_INCREASE = 0,

    // Decreases the amount of money to exchange.
    CG_TRADE_MONEY_DECREASE
};

////////////////////////////////////////////////////////////////////////////////
//
// class CGTradeMoney;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradeMoney : public Packet {
public:
    CGTradeMoney(){};
    virtual ~CGTradeMoney(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_TRADE_MONEY;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szGold + szBYTE;
    }
    string getPacketName() const {
        return "CGTradeMoney";
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
    ObjectID_t m_TargetObjectID; // ObjectID of the partner to exchange with
    Gold_t m_Gold;               // Amount wanted
    BYTE m_Code;                 // Code
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGTradeMoneyFactory;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradeMoneyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_TRADE_MONEY;
    static constexpr std::string_view kName = "CGTradeMoney";
    static constexpr PacketSize_t kMaxSize{szObjectID + szGold + szBYTE};

    Packet* createPacket() override {
        return new CGTradeMoney();
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
// class CGTradeMoneyHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradeMoneyHandler {
public:
    static void execute(CGTradeMoney* pPacket, Player* player);
    static void executeSlayer(CGTradeMoney* pPacket, Player* player);
    static void executeVampire(CGTradeMoney* pPacket, Player* player);
    static void executeOusters(CGTradeMoney* pPacket, Player* player);
    static void executeError(CGTradeMoney* pPacket, Player* player, BYTE ErrorCode);
};

#endif
