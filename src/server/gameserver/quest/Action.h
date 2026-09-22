//////////////////////////////////////////////////////////////////////////////
// Filename    : Action.h
// Written By  :
// Description :
// Class representing the action to run when a trigger's condition is satisfied.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_H__
#define __ACTION_H__

#include "Assert.h"
#include "Exception.h"
#include "PropertyBuffer.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class Action
//////////////////////////////////////////////////////////////////////////////

namespace de {
class GameContext;
}

class Creature;

class Action {
public:
    enum ActionTypes {
        ACTION_SET_POSITION,            // Set the position.
        ACTION_WANDER,                  // Wander within a set range.
        ACTION_SAY,                     // Say a given script.
        ACTION_RANDOM_SAY,              // Broadcast a given script.
        ACTION_ASK,                     // Ask the player a question.
        ACTION_QUIT_DIALOGUE,           // Make the client close the dialogue window.
        ACTION_PREPARE_SHOP,            // Prepare the shop.
        ACTION_SELL,                    // Sell an item to the player.
        ACTION_BUY,                     // Buy goods from the player.
        ACTION_REGEN_SHOP,              // Restock the items in the shop.
        ACTION_PREPARE_TEACH,           // Prepare to teach a skill.
        ACTION_TEACH_SKILL,             // Teach a skill to the player.
        ACTION_HEAL,                    // Heal the player.
        ACTION_REDEEM_MOTORCYCLE,       // Return the motorcycle.
        ACTION_SEARCH_MOTORCYCLE,       // Tell the player where their motorcycle is.
        ACTION_REPAIR,                  // Repair the player's item.
        ACTION_STASH_SELL,              // Open the stash purchase interface.
        ACTION_STASH_OPEN,              // Open the stash interface.
        ACTION_RESTORE,                 // Turn a Vampire back into a Slayer.
        ACTION_SET_RESURRECT_ZONE,      // Set the resurrection position.
        ACTION_SILVER_COATING,          // Silver-coat an item.
        ACTION_CREATE_GUILD,            // Open the guild creation interface.
        ACTION_DESTROY_GUILD,           // Open the guild disband interface.
        ACTION_TUTORIAL,                // Open the tutorial interface.
        ACTION_GIVE_NEWBIE_ITEM,        // Open the newbie item selection interface.
        ACTION_ACTIVATE_PORTAL,         // Activate the portal.
        ACTION_TURN_ON_FLAG,            // Turn the flag on and save it.
        ACTION_TURN_OFF_FLAG,           // Turn the flag off and save it.
        ACTION_SYSTEM_MESSAGE,          // Send a system message.
        ACTION_TAKE_DONATION,           // Take a donation.
        ACTION_CALL_HELICOPTER,         // Open the helicopter interface.
        ACTION_REGEN_EVENT_SHOP,        // Refresh the event shop.
        ACTION_SAY_DYNAMIC,             // Say a given script.
        ACTION_ASK_DYNAMIC,             // Ask the player a question.
        ACTION_GIVE_SPECIAL_EVENT_ITEM, //
        ACTION_REDISTRIBUTE_ATTR,       //
        ACTION_GIVE_GARBAGE_ITEM,       // Give a garbage item.

        // ACTION_TALK ,
        // ACTION_RANDOM_TALK ,
        // ACTION_GIVE ,
        // ACTION_TAKE ,
        // ACTION_DROP ,
        // ACTION_CREATE_ITEM ,
        // ACTION_DESTROY_ITEM ,
        // ACTION_SET_ATTRIBUTE ,
        // ACTION_SET_DISCOUNT_RATE ,
        // ACTION_DIE ,
        // ACTION_USE_SKILL ,
        // ACTION_USE_ITEM ,
        // ACTION_REMOVE_QUEST ,
        // ACTION_AFTER ,

        // Monster AI specific actions
        ACTION_ATTACK,
        ACTION_RETREAT,
        ACTION_MOVE,
        ACTION_ATTACK_MOVE,
        ACTION_STOP,
        ACTION_HOLD_POSITION,
        ACTION_PATROL,

        ACTION_TRADE_EVENT_ITEM,
        ACTION_SHOW_GUILD_DIALOG,

        ACTION_TRADE_LAIR_ITEM,

        ACTION_SIMPLE_QUEST_LIST,
        ACTION_SIMPLE_QUEST_COMPLETE,
        ACTION_SIMPLE_QUEST_REGEN,

        ACTION_TRADE_GIFT_BOX,

        ACTION_GIVE_TEST_SERVER_REWARD,

        // Action for entering a castle
        // 2003. 1.20. by bezz,Sequoia
        ACTION_ENTER_CASTLE_WITH_FEE,

        // Show the tax balance accumulated in the castle.
        ACTION_SHOW_TAX_BALANCE,

        ACTION_ASK_VARIABLE,

        // War registration actions
        ACTION_WAR_REGISTRATION,
        ACTION_SHOW_WAR_SCHEDULE,

        // Race war registration
        ACTION_JOIN_RACE_WAR,

        // Event gift items
        ACTION_GIVE_EVENT_ITEM,         // Given per character
        ACTION_GIVE_ACCOUNT_EVENT_ITEM, // Given per account

        // Couple manager initialization actions
        ACTION_INIT_PARTNER_WAITING_MANAGER,
        ACTION_WAIT_FOR_MEET_COUPLE,
        ACTION_WAIT_FOR_APART_COUPLE,
        ACTION_ACCEPT_COUPLE_REQUEST,
        ACTION_ACCEPT_APART_COUPLE,

        ACTION_FORCE_APART_COUPLE,

        ACTION_ENTER_PK_ZONE,
        ACTION_WARP_TO_RESURRECT_POSITION,

        ACTION_SELECT_QUEST,
        ACTION_QUEST_REWARD,

        ACTION_WARP_TO_NOVICE_ZONE,
        ACTION_CANCEL_QUEST,

        ACTION_INIT_SIMPLE_QUEST,
        ACTION_WANDER_ZONE,

        ACTION_INIT_EVENT_QUEST,
        ACTION_ASK_BY_QUEST_LEVEL,
        ACTION_GIVE_EVENT_QUEST,
        ACTION_ADVANCE_EVENT_QUEST,
        ACTION_REWARD_EVENT_QUEST,
        ACTION_CANCEL_EVENT_QUEST_SCRIPT,
        ACTION_GIVE_LOTTO,
        ACTION_GIVE_EVENT_QUEST_SCRIPT,
        ACTION_START_EVENT_QUEST,
        ACTION_WARP_IN_ZONE,
        ACTION_EVENT_MEET,
        ACTION_GIVE_FINAL_LOTTO_SCRIPT,
        ACTION_GIVE_QUEST_ITEM,
        ACTION_TAKE_OUT_GOODS,
        ACTION_CLEAR_RANK_BONUS,
        ACTION_CONTRACT_GNOMES_HORN,
        ACTION_DOWN_SKILL,
        ACTION_MINI_GAME,
        ACTION_GIVE_ITEM,
        ACTION_ACTIVATE_MAZE_EXIT,
        ACTION_ACTIVATE_MAZE_ENTER,
        ACTION_ACTIVATE_MAZE_RETURN,

        ACTION_SYSTEM_MESSAGE_PER_RACE, // Send a system message.

        ACTION_WARP_LEVEL_WAR_ZONE,

        ACTION_CHECK_PARTNER_NAME, // Check the partner name

        ACTION_START_PET_QUEST, // Start the second pet quest.

        ACTION_PET_WITHDRAW, // Withdraw the pet
        ACTION_PET_DEPOSIT,  // Deposit the pet

        ACTION_ENTER_EVENT_ZONE, // Enter the event zone.
        ACTION_ENTER_GDR_LAIR,   // Enter the Gilles de Rais lair from Illusion's Way

        ACTION_TRADE_GQUEST_EVENT_ITEM, // Event quest item

        ACTION_ENTER_SIEGE, // Join the siege war (move to the siege area)

        ACTION_REGISTER_SIEGE,     // Register for the siege
        ACTION_REGISTER_REINFORCE, // Register to fight for the defenders
        ACTION_ACCEPT_REINFORCE,   // Accept a defender participation request
        ACTION_DENY_REINFORCE,     // Deny a defender participation request

        ACTION_RECALL_SIEGE,       // Bring guild members to the siege war
        ACTION_SELECT_BLOOD_BIBLE, // Lend out the Blood Bible
        ACTION_CLEAR_BLOOD_BIBLE,  // Remove the Blood Bible

        ACTION_MODIFY_TAX_RATIO,      // Adjust the tax rate
        ACTION_SWAP_ADVANCEMENT_ITEM, // Exchange an advancement item

        ACTION_SHOW_DONATION_DIALOG,               // Show the donation window
        ACTION_ENTER_QUEST_ZONE,                   // Enter a DynamicZone
        ACTION_SHOW_CONFIRM_GET_EVENT_ITEM_DIALOG, // Event item receipt confirmation window

        ACTION_MAX
    };

public:
    virtual ~Action() {}
    virtual ActionType_t getActionType() const = 0;
    virtual void read(PropertyBuffer& buffer) = 0;
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL) = 0;
    virtual string toString() const = 0;

    // The factory that creates an action hands it the context the
    // action reads its managers from.
    void setContext(de::GameContext& context) {
        m_pContext = &context;
    }

protected:
    de::GameContext& context() const {
        Assert(m_pContext != nullptr);
        return *m_pContext;
    }

private:
    de::GameContext* m_pContext = nullptr;
};

#endif
