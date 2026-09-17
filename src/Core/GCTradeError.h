////////////////////////////////////////////////////////////////////////////////
// Filename    : GCTradeError.h
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __GC_TRADE_ERROR_H__
#define __GC_TRADE_ERROR_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
// Error code
////////////////////////////////////////////////////////////////////////////////

enum {
    // The target of the trade does not exist
    GC_TRADE_ERROR_CODE_TARGET_NOT_EXIST = 0,

    // The target of the trade is of another race
    GC_TRADE_ERROR_CODE_RACE_DIFFER,

    // The place the trade is attempted in is not a safe zone.
    GC_TRADE_ERROR_CODE_NOT_SAFE,

    // Trying to trade while riding a motorcycle.
    GC_TRADE_ERROR_CODE_MOTORCYCLE,

    // A trade cannot be made in wolf or bat form.
    GC_TRADE_ERROR_CODE_BAT_OR_WOLF,

    // Trying to start another trade while already trading
    GC_TRADE_ERROR_CODE_ALREADY_TRADING,

    // A trade packet arrived although no trade is in progress.
    GC_TRADE_ERROR_CODE_NOT_TRADING,

    // Does not hold the item it is trying to add to the trade
    GC_TRADE_ERROR_CODE_ADD_ITEM,

    // Does not hold the item it is trying to take out of the trade
    GC_TRADE_ERROR_CODE_REMOVE_ITEM,

    // Does not hold the money it is trying to add to the trade.
    GC_TRADE_ERROR_CODE_INCREASE_MONEY,

    // Does not hold the money it is trying to take out of the trade.
    GC_TRADE_ERROR_CODE_DECREASE_MONEY,

    // The trade failed because there was not enough room
    GC_TRADE_ERROR_CODE_NOT_ENOUGH_SPACE,

    // The trade failed because of the gift box trade condition
    GC_TRADE_ERROR_CODE_EVENT_GIFT_BOX,

    // An unknown error...
    GC_TRADE_ERROR_CODE_UNKNOWN,

    GC_TRADE_ERROR_CODE_MAX
};

////////////////////////////////////////////////////////////////////////////////
//
// class GCTradeError;
//
////////////////////////////////////////////////////////////////////////////////

class GCTradeError : public Packet {
public:
    GCTradeError(){};
    ~GCTradeError(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_TRADE_ERROR;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE;
    }
    string getPacketName() const {
        return "GCTradeError";
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
    ObjectID_t m_TargetObjectID; // Id of the trade target
    BYTE m_Code;                 // Code
};


////////////////////////////////////////////////////////////////////////////////
//
// class GCTradeErrorFactory;
//
////////////////////////////////////////////////////////////////////////////////

class GCTradeErrorFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_TRADE_ERROR;
    static constexpr std::string_view kName = "GCTradeError";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE};

    Packet* createPacket() override {
        return new GCTradeError();
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
