////////////////////////////////////////////////////////////////////////////////
// Filename    : GCTradeVerify.h
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __GC_TRADE_VERIFY_H__
#define __GC_TRADE_VERIFY_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
// Error code
////////////////////////////////////////////////////////////////////////////////

enum {
    // Verify adding an item to the trade
    GC_TRADE_VERIFY_CODE_ADD_ITEM_WHEN_ACCEPT,

    // Verify removing an item from the trade
    GC_TRADE_VERIFY_CODE_REMOVE_ITEM,

    // Verify adding money to the trade
    GC_TRADE_VERIFY_CODE_MONEY_INCREASE,

    // Verify removing money from the trade
    GC_TRADE_VERIFY_CODE_MONEY_DECREASE,

    // Verify confirming the trade
    GC_TRADE_VERIFY_CODE_FINISH_ACCEPT,

    // Verify cancelling the trade
    GC_TRADE_VERIFY_CODE_FINISH_REJECT,

    // Verify reconsidering the trade
    GC_TRADE_VERIFY_CODE_FINISH_RECONSIDER,

    // Verify moving an item to the mouse from the inventory while trading
    GC_TRADE_VERIFY_CODE_MOUSE_TO_INVENTORY_OK,
    GC_TRADE_VERIFY_CODE_MOUSE_TO_INVENTORY_FAIL,

    // Verify moving an item from the inventory to the mouse while trading
    GC_TRADE_VERIFY_CODE_INVENTORY_TO_MOUSE_OK,
    GC_TRADE_VERIFY_CODE_INVENTORY_TO_MOUSE_FAIL,

    // For now, when it is a gift box...
    GC_TRADE_VERIFY_CODE_ADD_ITEM_OK,
    GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL,

    // An error.
    GC_TRADE_VERIFY_CODE_MAX
};

////////////////////////////////////////////////////////////////////////////////
//
// class GCTradeVerify;
//
////////////////////////////////////////////////////////////////////////////////

class GCTradeVerify : public Packet {
public:
    GCTradeVerify(){};
    ~GCTradeVerify(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_TRADE_VERIFY;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }
    string getPacketName() const {
        return "GCTradeVerify";
    }
    string toString() const;

public:
    BYTE getCode() const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

private:
    BYTE m_Code; // Code
};


////////////////////////////////////////////////////////////////////////////////
//
// class GCTradeVerifyFactory;
//
////////////////////////////////////////////////////////////////////////////////

class GCTradeVerifyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_TRADE_VERIFY;
    static constexpr std::string_view kName = "GCTradeVerify";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    Packet* createPacket() override {
        return new GCTradeVerify();
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
