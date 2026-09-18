//////////////////////////////////////////////////////////////////////////////
// Filename    : FlagSet.h
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __FLAGSET_H__
#define __FLAGSET_H__

#include <string>

#include "Exception.h"
#include "Types.h"

#define FLAG_SIZE_MAX 3 * 8

enum FlagSetType {
    FLAGSET_RECEIVE_NEWBIE_ITEM,         // 0 - received the newbie item
    FLAGSET_RECEIVE_NEWBIE_ITEM_FIGHTER, // 1 - sword, blade
    FLAGSET_RECEIVE_NEWBIE_ITEM_CLERIC,  // 2 - heal, enchant
    FLAGSET_RECEIVE_NEWBIE_ITEM_GUNNER,  // 3 - gun

    FLAGSET_TRADE_GIFT_BOX_2002_12, // 4 - traded the red gift box of the Christmas event
    FLAGSET_RECEIVE_GREEN_GIFT_BOX, // 5 - received a green gift box from someone else

    FLAGSET_RECEIVE_NEWBIE_ITEM_AUTO, // 6 - should receive the newbie item automatically

    FLAGSET_RECEIVE_PREMIUM_EVENT_ITEM_2003_3, // 7 - gift given to premium users

    FLAGSET_IS_COUPLE,  // 7 - already in a couple
    FLAGSET_WAS_COUPLE, // 8 - was in a couple; used by the couple event

    FLAGSET_NOT_JUST_CREATED, // 9 - not a just-created character

    FLAGSET_CLEAR_RANK_BONUS_5,  // 10 - has the level 5 rank skill ever been reset?
    FLAGSET_CLEAR_RANK_BONUS_10, // 11 - has the level 10 rank skill ever been reset?
    FLAGSET_CLEAR_RANK_BONUS_15, // 12 - has the level 15 rank skill ever been reset?
    FLAGSET_CLEAR_RANK_BONUS_20, // 13 - has the level 20 rank skill ever been reset?

    FLAGSET_GNOMES_HORN, // 14 - has the contract for the earth spirit's horn been made

    FLAGSET_SWAP_COAT,    // 15 - swapped the armor coat
    FLAGSET_SWAP_TROUSER, // 16 - swapped the armor trousers
    FLAGSET_SWAP_WEAPON,  // 17 - swapped the weapon

    FLAGSET_MAX
};

//////////////////////////////////////////////////////////////////////////////
// class Flag
//////////////////////////////////////////////////////////////////////////////

class FlagSet {
    ///// Member methods /////

public:
    FlagSet();
    ~FlagSet();

public:
    void create(const string& owner);
    void load(const string& owner);
    void save(const string& owner);
    void destroy(const string& owner);

public:
    bool isOn(int index);
    bool turnOn(int index);
    bool turnOff(int index);

public:
    string toString(void);
    static FlagSet fromString(const string& text);

    static void initialize(void);

protected:
    bool isValidIndex(int index);
    BYTE* getData() {
        return &m_pData[0];
    }


    ///// Member data /////

protected:
    BYTE m_pData[FLAG_SIZE_MAX / 8];

    static string m_pLookup[256];
    static bool m_bInit;
};


#endif
