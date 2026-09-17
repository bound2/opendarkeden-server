//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNoticeEvent.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_NOTICE_EVENT_H__
#define __GC_NOTICE_EVENT_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Utility.h"

enum {
    NOTICE_EVENT_KICK_OUT_FROM_ZONE,          // Time until the zone throws players out (seconds)
    NOTICE_EVENT_CONTINUAL_GROUND_ATTACK,     // A continuous attack happens in the zone (seconds)
    NOTICE_EVENT_CONTINUAL_GROUND_ATTACK_END, // The continuous attack in the zone has ended.
    NOTICE_EVENT_MASTER_COMBAT_TIME,          // Time left to fight the master (seconds)
    NOTICE_EVENT_MASTER_COMBAT_END,           // The fight with the master has ended.
    NOTICE_EVENT_METEOR_STRIKE,               // Meteor strike (seconds)
    NOTICE_EVENT_METEOR_STRIKE_END,           // Meteor end
    NOTICE_EVENT_PREMIUM_HALF_START,          // Potions and serum are half price for premium users
    NOTICE_EVENT_PREMIUM_HALF_END,            // Event end
    NOTICE_EVENT_SHOP_TAX_CHANGE,             // The shop tax changed.(tax rate)

    // War related
    NOTICE_EVENT_WAR_OVER,      // War end (CastleZoneID)
    NOTICE_EVENT_RACE_WAR_OVER, // Race war end

    // When the character is created for the first time
    NOTICE_EVENT_WELCOME_MESSAGE,

    // Meet the quest grandmother -_-;
    NOTICE_EVENT_MEET_GRANDMA,
    NOTICE_EVENT_MEET_FISHSHOP_MASTER,

    NOTICE_EVENT_START_QUEST_ENDING,
    NOTICE_EVENT_RESULT_LOTTERY,

    NOTICE_EVENT_RUN_HORN,

    NOTICE_EVENT_MASTER_LAIR_OPEN,   // The master lair (Bathory lair) has opened.
    NOTICE_EVENT_MASTER_LAIR_CLOSED, // The master lair (Bathory lair) has closed.
    NOTICE_EVENT_MASTER_LAIR_COUNT,  // Five minutes are left to enter the master lair (Bathory lair).

    NOTICE_EVENT_CONTRACT_GNOMES_HORN, // You have to go to Sioram and make a contract before you can use it.
    NOTICE_EVENT_NOT_ENOUGH_MONEY,     // Not enough money.

    NOTICE_EVENT_MINI_GAME, // Minesweeper~

    NOTICE_EVENT_GET_RIFINIUM,   // Rifinium was obtained.
    NOTICE_EVENT_INVENTORY_FULL, // There is not enough free room in the inventory.

    NOTICE_EVENT_FLAG_WAR_READY,   // A Capture the Flag! event is being held.
    NOTICE_EVENT_FLAG_WAR_START,   // The event starts.
    NOTICE_EVENT_FLAG_WAR_FINISH,  // The event has ended. The items blow up in 3 minutes.
    NOTICE_EVENT_FLAG_POURED_ITEM, // A Capture the Flag event item has appeared.

    NOTICE_EVENT_ENTER_BEGINNER_ZONE, // Do you want to enter the beginner zone?
    NOTICE_EVENT_LOGIN_JUST_NOW,      // Just logged in.

    NOTICE_EVENT_LEVEL_WAR_ARRANGED, // The level war starts soon.
    NOTICE_EVENT_LEVEL_WAR_STARTED,  // The level war has started.

    NOTICE_EVENT_RACE_WAR_SOON, // The race war happens within 3 days.

    NOTICE_EVENT_LEVEL_WAR_OVER,      // The level war has ended.
    NOTICE_EVENT_NETMARBLE_CARD_FULL, // All 99 Netmarble cards have been collected.

    NOTICE_EVENT_HOLYDAY,       // Today is a national holiday.
    NOTICE_EVENT_CAN_PET_QUEST, // The pet quest can be done.

    NOTICE_EVENT_SEND_SMS, // Open the interface for sending an SMS

    NOTICE_EVENT_GDR_LAIR_ENDING_1, // First ending :)

    NOTICE_EVENT_GOLD_MEDALS, // Number of gold medals~
    NOTICE_EVENT_CROWN_PRICE, // Price of the laurel crown

    NOTICE_EVENT_GIVE_PRESENT_1,     // A level 1 gift box was given.
    NOTICE_EVENT_GIVE_PRESENT_2,     // A level 2 gift box was given.
    NOTICE_EVENT_GIVE_PRESENT_3,     // A level 3 gift box was given.
    NOTICE_EVENT_GIVE_PRESENT_4,     // A level 4 gift box was given.
    NOTICE_EVENT_GIVE_PRESENT_5,     // A level 5 gift box was given.
    NOTICE_EVENT_GIVE_PRESENT_6,     // A level 6 gift box was given.
    NOTICE_EVENT_GIVE_PRESENT_7,     // A level 7 gift box was given.
    NOTICE_EVENT_GIVE_PRESENT_8,     // A level 8 gift box was given.
    NOTICE_EVENT_CAN_OPEN_PRESENT_8, // The level 8 gift box can be opened.

    NOTICE_EVENT_RACE_WAR_IN_20, // The war starts in 20 minutes.
    NOTICE_EVENT_RACE_WAR_IN_5,  // The war starts in 5 minutes.

    NOTICE_EVENT_RACE_WAR_STARTED_IN_OTHER_SERVER, // The race war has started on the first server.

    NOTICE_EVENT_CANNOT_FIND_STORE,  // That seller cannot be found.
    NOTICE_EVENT_STORE_CLOSED,       // The shop has already closed.
    NOTICE_EVENT_ITEM_NOT_FOUND,     // The item has been sold already or withdrawn by the seller.
                                     //	NOTICE_EVENT_NOT_ENOUGH_MONEY,				// Not enough money to buy it.
    NOTICE_EVENT_TOO_MUCH_MONEY,     // The seller holds too much money, so it cannot be bought.
    NOTICE_EVENT_NO_INVENTORY_SPACE, // There is no empty slot in the inventory.

    NOTICE_EVENT_ALREADY_DISPLAYED, // The item is on display already.
    NOTICE_EVENT_CANNOT_SELL,       // The item cannot be sold.

    NOTICE_EVENT_MAX
};

//////////////////////////////////////////////////////////////////////////////
// class GCNoticeEvent
//////////////////////////////////////////////////////////////////////////////

class GCNoticeEvent : public Packet {
public:
    GCNoticeEvent() {
        m_Code = NOTICE_EVENT_MAX;
        m_Parameter = 0;
    }
    virtual ~GCNoticeEvent() {}

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketID_t getPacketID() const {
        return PACKET_GC_NOTICE_EVENT;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCNoticeEvent";
    }
    string toString() const;

public:
    WORD getCode(void) const {
        return m_Code;
    }
    void setCode(WORD code) {
        m_Code = code;
    }

    uint getParameter(void) const {
        return m_Parameter;
    }
    void setParameter(uint parameter) {
        m_Parameter = parameter;
    }

    void setParameter(WORD hiWord, WORD loWord) {
        m_Parameter = makeDWORD(hiWord, loWord);
    }

private:
    WORD m_Code;
    uint m_Parameter;
};


//////////////////////////////////////////////////////////////////////////////
// class GCNoticeEventFactory;
//////////////////////////////////////////////////////////////////////////////

class GCNoticeEventFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_NOTICE_EVENT;
    static constexpr std::string_view kName = "GCNoticeEvent";
    static constexpr PacketSize_t kMaxSize{szWORD + szuint};

    Packet* createPacket() override {
        return new GCNoticeEvent();
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


#endif
