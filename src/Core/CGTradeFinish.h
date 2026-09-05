////////////////////////////////////////////////////////////////////////////////
// Filename    : CGTradeFinish.h
// Written By  : 김성민
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_TRADE_FINISH_H__
#define __CG_TRADE_FINISH_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
// 교환 코드
////////////////////////////////////////////////////////////////////////////////

enum {
    // 교환을 허락할 때 보내는 코드
    CG_TRADE_FINISH_ACCEPT = 0,

    // 교환을 거부할 때 보내는 코드
    CG_TRADE_FINISH_REJECT,

    // 교환을 재고려할 때 보내는 코드
    CG_TRADE_FINISH_RECONSIDER,


    CG_TRADE_FINISH_MAX
};

////////////////////////////////////////////////////////////////////////////////
//
// class CGTradeFinish;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradeFinish : public Packet {
public:
    CGTradeFinish(){};
    virtual ~CGTradeFinish(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_TRADE_FINISH;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE;
    }
    string getPacketName() const {
        return "CGTradeFinish";
    }
    string toString() const;

public:
    ObjectID_t getTargetObjectID() const {
        return m_TargetObjectID;
    }
    void setTargetObjectID(ObjectID_t id) {
        m_TargetObjectID = id;
    }

    BYTE getCode() const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

private:
    ObjectID_t m_TargetObjectID; // 교환을 원하는 상대방의 ObjectID
    BYTE m_Code;                 // 교환 코드
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGTradeFinishFactory;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradeFinishFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_TRADE_FINISH;
    static constexpr std::string_view kName = "CGTradeFinish";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE};

    Packet* createPacket() override {
        return new CGTradeFinish();
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
// class CGTradeFinishHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradeFinishHandler {
public:
    static void execute(CGTradeFinish* pPacket, Player* player);
    static void executeSlayer(CGTradeFinish* pPacket, Player* player);
    static void executeVampire(CGTradeFinish* pPacket, Player* player);
    static void executeOusters(CGTradeFinish* pPacket, Player* player);
    static void executeError(CGTradeFinish* pPacket, Player* player, BYTE ErrorCode);
};

#endif
