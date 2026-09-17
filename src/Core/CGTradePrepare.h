////////////////////////////////////////////////////////////////////////////////
// Filename    : CGTradePrepare.h
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_TRADE_PREPARE_H__
#define __CG_TRADE_PREPARE_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
// Exchange code
////////////////////////////////////////////////////////////////////////////////

enum {
    // The player who first wants to exchange sends the packet with this code.
    CG_TRADE_PREPARE_CODE_REQUEST = 0,

    // The player who first asked for the exchange cancelled it.
    CG_TRADE_PREPARE_CODE_CANCEL,

    // When the player asked for an exchange accepts it
    CG_TRADE_PREPARE_CODE_ACCEPT,

    // When the player asked for an exchange does not accept it
    CG_TRADE_PREPARE_CODE_REJECT,

    // When the player asked for an exchange cannot exchange right now
    CG_TRADE_PREPARE_CODE_BUSY,

    CG_TRADE_PREPARE_CODE_MAX
};

////////////////////////////////////////////////////////////////////////////////
//
// class CGTradePrepare;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradePrepare : public Packet {
public:
    CGTradePrepare(){};
    virtual ~CGTradePrepare(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_TRADE_PREPARE;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE;
    }
    string getPacketName() const {
        return "CGTradePrepare";
    }
    string toString() const;

public:
    ObjectID_t getTargetObjectID() const {
        return m_TargetObjectID;
    }
    void setTargetObjectID(ObjectID_t id) {
        m_TargetObjectID = id;
    }

    BYTE getCode(void) const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

private:
    ObjectID_t m_TargetObjectID; // OID of the partner to exchange with
    BYTE m_Code;                 // Exchange code
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGTradePrepareFactory;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradePrepareFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_TRADE_PREPARE;
    static constexpr std::string_view kName = "CGTradePrepare";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE};

    Packet* createPacket() override {
        return new CGTradePrepare();
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
// class CGTradePrepareHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGTradePrepareHandler {
public:
    static void execute(CGTradePrepare* pPacket, Player* player);
    static void executeSlayer(CGTradePrepare* pPacket, Player* player);
    static void executeVampire(CGTradePrepare* pPacket, Player* player);
    static void executeOusters(CGTradePrepare* pPacket, Player* player);
    static void executeError(CGTradePrepare* pPacket, Player* player, BYTE ErrorCode);
};

#endif
