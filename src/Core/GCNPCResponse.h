//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNPCResponse.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_NPC_RESPONSE_H__
#define __GC_NPC_RESPONSE_H__

#include "Packet.h"
#include "PacketFactory.h"

enum {
    // Code that makes the client open the right interface
    NPC_RESPONSE_INTERFACE_REPAIR = 0,
    NPC_RESPONSE_INTERFACE_STASHOPEN,         // 1
    NPC_RESPONSE_INTERFACE_SILVER_COATING,    // 2
    NPC_RESPONSE_INTERFACE_CREATE_GUILD,      // 3
    NPC_RESPONSE_INTERFACE_DESTROY_GUILD,     // 4
    NPC_RESPONSE_INTERFACE_NEWBIE_ITEM,       // 5
    NPC_RESPONSE_INTERFACE_TUTORIAL_COMPUTER, // 6
    NPC_RESPONSE_INTERFACE_TUTORIAL_BRIEFING, // 7
    NPC_RESPONSE_INTERFACE_TUTORIAL_BOOKCASE, // 8
    NPC_RESPONSE_INTERFACE_HELICOPTER,        // 9

    // ...
    NPC_RESPONSE_QUIT_DIALOGUE, // 10
    NPC_RESPONSE_HEAL,          // 11

    // Response code for the client's packet
    NPC_RESPONSE_REPAIR_OK,                  // 12
    NPC_RESPONSE_REPAIR_FAIL_ITEM_NOT_EXIST, // 13
    NPC_RESPONSE_REPAIR_FAIL_ITEM_TYPE,      // 14
    NPC_RESPONSE_REPAIR_FAIL_MONEY,          // 15

    NPC_RESPONSE_STASH_SELL_OK,         // 16
    NPC_RESPONSE_STASH_SELL_FAIL_MAX,   // 17
    NPC_RESPONSE_STASH_SELL_FAIL_MONEY, // 18

    NPC_RESPONSE_SILVER_COATING_OK,                  // 19
    NPC_RESPONSE_SILVER_COATING_FAIL_ITEM_NOT_EXIST, // 20
    NPC_RESPONSE_SILVER_COATING_FAIL_ITEM_TYPE,      // 21
    NPC_RESPONSE_SILVER_COATING_FAIL_MONEY,          // 22

    NPC_RESPONSE_DONATION_OK,         // 23
    NPC_RESPONSE_DONATION_FAIL_MONEY, // 24

    // Temporary id
    // There is no way to hand out footballs in the World Cup patch, so NPC Response is used instead.
    NPC_RESPONSE_DECREASE_BALL, // 25

    // Guild related
    NPC_RESPONSE_TEAM_REGIST_FAIL_ALREADY_JOIN, // Let me have a look. It says you already belong to the <team_name> team	//
                                                // 26
    NPC_RESPONSE_TEAM_REGIST_FAIL_QUIT_TIMEOUT, // You left another team not long ago. Think a little more carefully and
                                                // act	// 27
    NPC_RESPONSE_TEAM_REGIST_FAIL_CANCEL_TIMEOUT, // Your team was cancelled not long ago. Grow yourself a little more so that
    // you meet the conditions. Watch for the chance more carefully	//
    // 28
    NPC_RESPONSE_TEAM_REGIST_FAIL_LEVEL, // Fine, but the makings of a leader look a little short. Hone your skill and come back	// 29
    NPC_RESPONSE_TEAM_REGIST_FAIL_MONEY, // Founding a team takes a lot of money. You do not look like you have it...	// 30
    NPC_RESPONSE_TEAM_REGIST_FAIL_FAME, // <player_name>, is it.. I have not even heard that name yet. That means you
    // are a greenhorn. Hone your skill and come back	// 31
    NPC_RESPONSE_TEAM_REGIST_FAIL_NAME, // That team name is taken already, think of another one	// 32
    NPC_RESPONSE_TEAM_REGIST_FAIL_DENY, // Rejected.	// 33

    NPC_RESPONSE_TEAM_STARTING_FAIL_ALREADY_JOIN, // You are already a member of another team.	// 34
    NPC_RESPONSE_TEAM_STARTING_FAIL_QUIT_TIMEOUT, // You left another team not long ago. Think a little more carefully
                                                  // and act	// 35
    NPC_RESPONSE_TEAM_STARTING_FAIL_CANCEL_TIMEOUT, // Your team was cancelled not long ago. Grow yourself a little more so that
    // you meet the conditions. Watch for the chance more carefully
    // // 36
    NPC_RESPONSE_TEAM_STARTING_FAIL_LEVEL, // You still look short of a good deal. Hone your skill and come back.	//
                                           // 37
    NPC_RESPONSE_TEAM_STARTING_FAIL_MONEY, // <player_name>, you need more money to register a team	// 38
    NPC_RESPONSE_TEAM_STARTING_FAIL_FAME,  // <player_name>, is it.. I have not even heard that name yet. That means you
    // are a greenhorn. Hone your skill and come back	// 39
    NPC_RESPONSE_TEAM_STARTING_FAIL_DENY, // Rejected.	// 40

    NPC_RESPONSE_CLAN_REGIST_FAIL_ALREADY_JOIN, // Let me have a look. It says you are already sworn to the <clan_name>
                                                // clan	// 41
    NPC_RESPONSE_CLAN_REGIST_FAIL_QUIT_TIMEOUT, // You left another clan not long ago. Moving from tree to tree
                                                // is not a good idea. Take care	// 42
    NPC_RESPONSE_CLAN_REGIST_FAIL_CANCEL_TIMEOUT, // Your clan was cancelled not long ago. Watch for the
                                                  // chance more carefully	// 43
    NPC_RESPONSE_CLAN_REGIST_FAIL_LEVEL, // Fine, but the makings of a chief look a little short. Hone your skill and come back.	//
                                         // 44
    NPC_RESPONSE_CLAN_REGIST_FAIL_MONEY, // Registering a clan takes a lot of money. You do not look like you have it...	// 45
    NPC_RESPONSE_CLAN_REGIST_FAIL_FAME, // <player_name>, is it.. still a young vampire it seems. Drink more blood and
                                        // come and find me.	// 46
    NPC_RESPONSE_CLAN_REGIST_FAIL_NAME, // 	// 47
    NPC_RESPONSE_CLAN_REGIST_FAIL_DENY, // Rejected.	// 48

    NPC_RESPONSE_CLAN_STARTING_FAIL_ALREADY_JOIN,   // You are already a member of another clan.	// 49
    NPC_RESPONSE_CLAN_STARTING_FAIL_QUIT_TIMEOUT,   // You left another clan not long ago. Moving from tree to tree
                                                    // is not a good idea. Take care	// 50
    NPC_RESPONSE_CLAN_STARTING_FAIL_CANCEL_TIMEOUT, // Your clan was cancelled not long ago. Watch for the chance
                                                    // more carefully	// 51
    NPC_RESPONSE_CLAN_STARTING_FAIL_LEVEL, // The makings of a good helper look a little short. Go and train a little more and
                                           // come back	// 52
    NPC_RESPONSE_CLAN_STARTING_FAIL_MONEY, // However good the skill, a clan whose money runs short collapses
                                           // easily	// 53
    NPC_RESPONSE_CLAN_STARTING_FAIL_FAME,  // <player_name>, is it.. I have not even heard that name yet. That means you
    // are a greenhorn. Hone your skill and come back	// 54
    NPC_RESPONSE_CLAN_STARTING_FAIL_DENY, //  Rejected.	// 55

    NPC_RESPONSE_GUILD_SHOW_REGIST,        // Open the guild registration window	// 56
    NPC_RESPONSE_GUILD_SHOW_STARTING_JOIN, // Open the guild starting member join window	// 57
    NPC_RESPONSE_GUILD_SHOW_JOIN,          // Open the guild join window	// 58
    NPC_RESPONSE_GUILD_SHOW_QUIT,          // Open the guild leave window	// 59
    NPC_RESPONSE_GUILD_ERROR,              // Guild error	// 60

    NPC_RESPONSE_TRADE_GIFT_BOX_OK,            // Gift box trade succeeded	// 61
    NPC_RESPONSE_TRADE_GIFT_BOX_NO_ITEM,       // No gift box	// 62
    NPC_RESPONSE_TRADE_GIFT_BOX_ALREADY_TRADE, // The gift box has been traded once already.	// 63
    NPC_RESPONSE_TRADE_GIFT_BOX_ERROR,         // Other error from the gift box trade	// 64

    NPC_RESPONSE_REWARD_OK,     // The reward was received.	// 65
    NPC_RESPONSE_REWARD_FAIL,   // The reward cannot be received.	// 66
    NPC_RESPONSE_NO_EMPTY_SLOT, // There is no empty slot.	// 67

    NPC_RESPONSE_SHOW_TAX_BALANCE,  // Show the balance of the tax collected in the castle.	// 68
    NPC_RESPONSE_WITHDRAW_TAX_OK,   // The guild master succeeded in withdrawing the tax.	// 69
    NPC_RESPONSE_WITHDRAW_TAX_FAIL, // The guild master failed to withdraw the tax.	// 70

    NPC_RESPONSE_NO_GUILD,         // You do not belong to a team (clan).	// 71
    NPC_RESPONSE_NOT_GUILD_MASTER, // You are not the team (clan) master.	// 72
    NPC_RESPONSE_HAS_NO_CASTLE,    // The team (clan) has no castle.	// 73
    NPC_RESPONSE_NOT_YOUR_CASTLE,  // The castle does not belong to the team (clan).	// 74

    // War related
    NPC_RESPONSE_NOT_ENOUGH_MONEY, // Not enough money.(really a shortage of the war application fee, but used generally -_-;)	// 75
    NPC_RESPONSE_WAR_SCHEDULE_FULL,      // The war schedule is full.	// 76
    NPC_RESPONSE_WAR_ALREADY_REGISTERED, // You have applied for a war already.	// 77
    NPC_RESPONSE_WAR_REGISTRATION_OK,    // Registered on the war schedule.	// 78
    NPC_RESPONSE_ALREADY_HAS_CASTLE,     // You have a castle already.	// 79
    NPC_RESPONSE_WAR_UNAVAILABLE,        // The war application cannot be made.	// 80

    // War participation related
    NPC_RESPONSE_RACE_WAR_JOIN_FAILED, // The race war for <user>'s level band is full.	// 81
    NPC_RESPONSE_RACE_WAR_JOIN_OK,     // The race war application was made.	// 82
    NPC_RESPONSE_RACE_WAR_GO_FIRST_SERVER, // The race war can only be applied for and joined on the first server of each world.	//
                                           // 83

    // Event gift item
    NPC_RESPONSE_GIVE_EVENT_ITEM_FAIL_NOW, // The event item cannot be received now.	// 84
    NPC_RESPONSE_GIVE_EVENT_ITEM_FAIL,     // The event item cannot be received.	// 85
    NPC_RESPONSE_GIVE_EVENT_ITEM_OK,       // The item for the event was received.	// 86
    NPC_RESPONSE_GIVE_PREMIUM_USER_ONLY,   // Only premium service users can receive it.	// 87

    // Couple application related
    NPC_RESPONSE_WAIT_FOR_MEET_COUPLE,   // Enter the name of the one to become a couple with	// 88
    NPC_RESPONSE_COUPLE_MEET_SUCCESS,    // The couple was formed.	// 89
    NPC_RESPONSE_COUPLE_CANNOT_MEET,     // The couple cannot be formed.	// 90
    NPC_RESPONSE_MEET_WAIT_TIME_EXPIRED, // The application was cancelled because the time ran out.	// 91

    NPC_RESPONSE_WAIT_FOR_APART_COUPLE,   // Enter the name of the one to part from	// 92
    NPC_RESPONSE_COUPLE_APART_SUCCESS,    // You have parted.	// 93
    NPC_RESPONSE_NOT_COUPLE,              // You cannot part because you are not a couple.	// 94
    NPC_RESPONSE_APART_WAIT_TIME_EXPIRED, // The application was cancelled because the time ran out.	// 95

    NPC_RESPONSE_APART_COUPLE_FORCE, // Enter the name of the one to part from one-sidedly.	// 96

    NPC_RESPONSE_QUEST,      // Quest related message --;	// 97
    NPC_RESPONSE_LOTTERY,    // Show the lottery	// 98
    NPC_RESPONSE_CANNOT_BUY, // The item bought cannot be found now.	// 99

    NPC_RESPONSE_CLEAR_RANK_BONUS_OK,      // The skill of the rank you chose has been deleted.	// 100
    NPC_RESPONSE_NO_RANK_BONUS,            // You do not have that qualification.	// 101
    NPC_RESPONSE_ALREADY_CLEAR_RANK_BONUS, // The skill of the rank you chose has been deleted before.	// 102

    NPC_RESPONSE_GNOME_CONTRACT_OK, // A contract with the earth spirit was made.	// 103
    NPC_RESPONSE_DOWN_SKILL,        // Choose the skill to take down>.<	// 104

    // Ousters guild related
    NPC_RESPONSE_GUILD_REGIST_FAIL_ALREADY_JOIN, // Let me have a look. It says you already belong to the <guild_name> guild
                                                 // // 105
    NPC_RESPONSE_GUILD_REGIST_FAIL_QUIT_TIMEOUT, // You left another guild not long ago. Think a little more carefully
                                                 // and act	// 106
    NPC_RESPONSE_GUILD_REGIST_FAIL_CANCEL_TIMEOUT, // Your guild was cancelled not long ago. Grow yourself a little more so
    // that you meet the registration conditions, and watch for the
    // chance more carefully	// 107
    NPC_RESPONSE_GUILD_REGIST_FAIL_LEVEL, // Fine, but the makings of a leader look a little short. Hone your skill and come back	// 108
    NPC_RESPONSE_GUILD_REGIST_FAIL_MONEY, // Founding a guild takes a lot of money. You do not look like you have it...	// 109
    NPC_RESPONSE_GUILD_REGIST_FAIL_FAME, // <player_name>, is it.. I have not even heard that name yet. That means you
    // are a greenhorn. Hone your skill and come back	// 110
    NPC_RESPONSE_GUILD_REGIST_FAIL_NAME, // That guild name is taken already, think of another one	// 111
    NPC_RESPONSE_GUILD_REGIST_FAIL_DENY, // Rejected.	// 112

    NPC_RESPONSE_GUILD_STARTING_FAIL_ALREADY_JOIN, // You are already a member of another guild.	// 113
    NPC_RESPONSE_GUILD_STARTING_FAIL_QUIT_TIMEOUT, // You left another guild not long ago. Think a little more carefully
                                                   // and act	// 114
    NPC_RESPONSE_GUILD_STARTING_FAIL_CANCEL_TIMEOUT, // Your guild was cancelled not long ago. Grow yourself a little more so that
    // you meet the conditions. Watch for the chance more carefully
    // // 115
    NPC_RESPONSE_GUILD_STARTING_FAIL_LEVEL, // You still look short of a good deal. Hone your skill and come back.	//
                                            // 116
    NPC_RESPONSE_GUILD_STARTING_FAIL_MONEY, // <player_name>, you need more money to register a guild	// 117
    NPC_RESPONSE_GUILD_STARTING_FAIL_FAME, // <player_name>, is it.. I have not even heard that name yet. That means you
    // are a greenhorn. Hone your skill and come back	// 118
    NPC_RESPONSE_GUILD_STARTING_FAIL_DENY, // Rejected.	// 119

    NPC_RESPONSE_TOO_MANY_GUILD_REGISTERED, // Too many guilds have applied, so no more can apply.	// 120
    NPC_RESPONSE_REINFORCE_DENYED, // The defence side's application was refused already, so it cannot be made again.	// 121
    NPC_RESPONSE_ALREADY_REINFORCE_ACCEPTED, // A guild whose defence-side application was accepted exists already, so none can be made.	//
                                             // 122
    NPC_RESPONSE_NO_WAR_REGISTERED, // No guild has applied for the siege, so the defence side cannot apply.	// 123
    NPC_RESPONSE_CANNOT_ACCEPT,     // The defence side's application cannot be accepted.
    NPC_RESPONSE_ACCEPT_OK,         // The application to take part was accepted.
    NPC_RESPONSE_CANNOT_DENY,       // The defence side's application cannot be refused.
    NPC_RESPONSE_DENY_OK,           // The application to take part was refused.

    NPC_RESPONSE_SHOW_TAX_RATIO,        // Show the castle's tax rate.
    NPC_RESPONSE_MODIFY_TAX_RATIO_OK,   // The castle's tax rate was adjusted.
    NPC_RESPONSE_MODIFY_TAX_RATIO_FAIL, // The castle's tax rate was adjusted.

    NPC_RESPONSE_SWAP_ADVANCEMENT_ITEM, // Trade it for an advancement item.
    NPC_RESPONSE_NOT_ADVANCED,          // The item cannot be traded without an advancement.

    NPC_RESPONSE_SHOW_DONATION_DIALOG,               // Open the donation window.
    NPC_RESPONSE_SHOW_DONATION_COMPLETE_DIALOG,      // The money you donated will be put to good use for the neighbours
                                                     // who need it. Thank you for taking part in the collection.
    NPC_RESPONSE_SHOW_CONFIRM_GET_EVENT_ITEM_DIALOG, // Open the dialog that confirms collecting the event item.
    NPC_RESPONSE_SHOW_COMMON_MESSAGE_DIALOG,         // Open the various message dialogs.

    NPC_RESPONSE_SHOW_DONATION_WEDDING_COMPLETE_DIALOG, //
    NPC_RESPONSE_SHOW_ALEADY_DONATED_DIALOG,            // You have already paid the congratulatory money.

    NPC_RESPONSE_MAX // 139
};

enum CoupleMessage {
    COUPLE_MESSAGE_NOT_EVENT_TERM = 1, // It is not the couple event period.
    COUPLE_MESSAGE_ALREADY_WAITING,    // Already waiting for a partner.
    COUPLE_MESSAGE_LOGOFF,             // The partner is not logged in.
    COUPLE_MESSAGE_DIFFERENT_RACE,     // You are of different races.
    COUPLE_MESSAGE_SAME_SEX,           // A couple can only be formed between a man and a woman.
    COUPLE_MESSAGE_NOT_PAY_PLAYER,     // Not a paying user.
    COUPLE_MESSAGE_ALREADY_COUPLE,     // You are a couple already.
    COUPLE_MESSAGE_WAS_COUPLE,         // You have been a couple before.
    COUPLE_MESSAGE_NOT_ENOUGH_GOLD,    // Not enough money.
    COUPLE_MESSAGE_NOT_ENOUGH_ATTR,    // The attributes are too low.
    COUPLE_MESSAGE_NOT_ENOUGH_LEVEL,   // The level is too low.
    COUPLE_MESSAGE_INVENTORY_FULL,     // There is not enough room for the couple ring.
    COUPLE_MESSAGE_NO_WAITING,         // There is no partner waiting for you.
    COUPLE_MESSAGE_NOT_COUPLE,         // You are not a couple.

    COUPLE_MESSAGE_MAX
};

enum QuestMessage {
    START_SUCCESS = 0,
    START_FAIL_QUEST_NUM_EXCEEDED,    // 1
    START_FAIL_DUPLICATED_QUEST_ID,   // 2
    START_FAIL_PC,                    // 3
    COMPLETE_SUCCESS,                 // 4
    COMPLETE_FAIL_NOT_COMPLETE,       // 5
    COMPLETE_FAIL_NO_INVENTORY_SPACE, // 6
    COMPLETE_FAIL_NOT_IN_QUEST,       // 7
    COMPLETE_FAIL_TIME_EXPIRED,       // 8
    START_FAIL_CANNOT_APPLY_QUEST,    // 9
    CANCEL_SUCCESS,                   // 10
    CANCEL_NOT_IN_QUEST,              // 11
    COMPLETE_FAIL_INVALID_NPC,        // 12
    FAIL_BUG                          // 13
};

enum CommonMessage {
    YOU_CAN_GET_EVENT_200501_COMBACK_ITEM = 0,       // 0	The comeback user item can be received.
    YOU_CAN_GET_EVENT_200501_COMBACK_PREMIUM_ITEM,   // 1	The comeback user payment item can be received.
    YOU_CAN_GET_EVENT_200501_COMBACK_RECOMMEND_ITEM, // 2	The comeback user recommendation item can be received.
    YOU_GET_EVENT_ITEM,                              // 3	The event item was received.
    NOT_ENOUGH_INVENTORY_SPACE,                      // 4	There is not enough room in the inventory.
    ALEADY_GET_EVENT_ITEM,                           // 5	The event item has already been collected.
    FAIL_GET_EVENT_ITEM,                             // 6	Collecting the event item failed
    YOU_ARE_NOT_EVENT_TARGET,                        // 7	Not an event target.

    COMMON_MESSAGE_MAX
};

//////////////////////////////////////////////////////////////////////////////
// class GCNPCResponse
//////////////////////////////////////////////////////////////////////////////

class GCNPCResponse : public Packet {
public:
    GCNPCResponse() {
        m_Code = NPC_RESPONSE_MAX;
        m_Parameter = 0;
    }
    virtual ~GCNPCResponse() {}

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketID_t getPacketID() const {
        return PACKET_GC_NPC_RESPONSE;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCNPCResponse";
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

private:
    WORD m_Code;
    uint m_Parameter;
};


//////////////////////////////////////////////////////////////////////////////
// class GCNPCResponseFactory;
//////////////////////////////////////////////////////////////////////////////

class GCNPCResponseFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_NPC_RESPONSE;
    static constexpr std::string_view kName = "GCNPCResponse";
    static constexpr PacketSize_t kMaxSize{szWORD + szuint};

    Packet* createPacket() override {
        return new GCNPCResponse();
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
