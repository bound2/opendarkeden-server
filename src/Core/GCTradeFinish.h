////////////////////////////////////////////////////////////////////////////////
// Filename    : GCTradeFinish.h
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __GC_TRADE_FINISH_H__
#define __GC_TRADE_FINISH_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
// Trade code
////////////////////////////////////////////////////////////////////////////////

enum {
    // Code sent when allowing the trade
    GC_TRADE_FINISH_ACCEPT = 0,

    // Code sent when refusing the trade
    GC_TRADE_FINISH_REJECT,

    // Code sent when reconsidering the trade
    GC_TRADE_FINISH_RECONSIDER,

    // Code that asks for the trade to be carried out
    GC_TRADE_FINISH_EXECUTE,

    GC_TRADE_FINISH_MAX
};

////////////////////////////////////////////////////////////////////////////////
//
// class GCTradeFinish;
//
////////////////////////////////////////////////////////////////////////////////

class GCTradeFinish : public Packet {
public:
    GCTradeFinish(){};
    ~GCTradeFinish(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_TRADE_FINISH;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE;
    }
    string getPacketName() const {
        return "GCTradeFinish";
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
    ObjectID_t m_TargetObjectID; // ObjectID of the partner the trade is wanted with
    BYTE m_Code;                 // Trade code
};


////////////////////////////////////////////////////////////////////////////////
//
// class GCTradeFinishFactory;
//
////////////////////////////////////////////////////////////////////////////////

class GCTradeFinishFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_TRADE_FINISH;
    static constexpr std::string_view kName = "GCTradeFinish";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE};

    Packet* createPacket() override {
        return new GCTradeFinish();
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
