////////////////////////////////////////////////////////////////////////////////
// Filename    : CGGetEventItem.h
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_GET_EVENT_ITEM_H__
#define __CG_GET_EVENT_ITEM_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
// Donation type
////////////////////////////////////////////////////////////////////////////////
enum EventType {
    EVENT_TYPE_200501_COMBACK_ITEM = 0,       // Event that gives a ring item to a returning user
    EVENT_TYPE_200501_COMBACK_PREMIUM_ITEM,   // Event that gives an item when a returning user pays
    EVENT_TYPE_200501_COMBACK_RECOMMEND_ITEM, // Event that gives an item to the referrer when a returning user pays

    EVENT_TYPE_MAX
};

////////////////////////////////////////////////////////////////////////////////
//
// class CGGetEventItem
//
////////////////////////////////////////////////////////////////////////////////
class CGGetEventItem : public Packet {
public:
    CGGetEventItem(){};
    ~CGGetEventItem(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_GET_EVENT_ITEM;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }
    string getPacketName() const {
        return "CGGetEventItem";
    }
    string toString() const;

public:
    // get / set Event Type
    BYTE getEventType() const {
        return m_EventType;
    }
    void setEventType(BYTE eventType) {
        m_EventType = eventType;
    }

private:
    BYTE m_EventType = 0; // Event type
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGGetEventItemFactory
//
////////////////////////////////////////////////////////////////////////////////

class CGGetEventItemFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_GET_EVENT_ITEM;
    static constexpr std::string_view kName = "CGGetEventItem";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    Packet* createPacket() override {
        return new CGGetEventItem();
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
// class CGGetEventItemHandler
//
////////////////////////////////////////////////////////////////////////////////
class CGGetEventItemHandler {
public:
    static void execute(CGGetEventItem* pPacket, Player* player);

    static void executeCombackItem(CGGetEventItem* pPacket, Player* pPlayer);
    static void executeCombackPremiumItem(CGGetEventItem* pPacket, Player* pPlayer);
    static void executeCombackRecommendItem(CGGetEventItem* pPacket, Player* pPlayer);
};

#endif
