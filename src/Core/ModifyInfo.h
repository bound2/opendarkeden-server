//////////////////////////////////////////////////////////////////////////////
// Filename    : ModifyInfo.h
// Written By  : elca@ewestsoft.com
// Description :
// Packet that flies to the client when the player's state changes.
// Most of the packets that carry a player's changed information
// inherit from this packet.
//////////////////////////////////////////////////////////////////////////////

#ifndef __MODIFY_INFO_H__
#define __MODIFY_INFO_H__

#include <list>

#include "Exception.h"
#include "Packet.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// State change types
//////////////////////////////////////////////////////////////////////////////
enum ModifyType {
    MODIFY_BASIC_STR = 0,           // Change the basic STR
    MODIFY_CURRENT_STR,             // Change the current STR
    MODIFY_MAX_STR,                 // Change the max STR
    MODIFY_STR_EXP,                 // Change the STR experience
    MODIFY_BASIC_DEX,               // Change the basic DEX
    MODIFY_CURRENT_DEX,             // Change the current DEX
    MODIFY_MAX_DEX,                 // Change the max DEX
    MODIFY_DEX_EXP,                 // Change the DEX experience
    MODIFY_BASIC_INT,               // Change the basic INT
    MODIFY_CURRENT_INT,             // Change the current INT
    MODIFY_MAX_INT,                 // Change the max INT
    MODIFY_INT_EXP,                 // Change the INT experience
    MODIFY_CURRENT_HP,              // Change the current HP
    MODIFY_MAX_HP,                  // Change the max HP
    MODIFY_CURRENT_MP,              // Change the current MP
    MODIFY_MAX_MP,                  // Change the max MP
    MODIFY_MIN_DAMAGE,              // Change the min damage
    MODIFY_MAX_DAMAGE,              // Change the max damage
    MODIFY_DEFENSE,                 // Change the defence
    MODIFY_PROTECTION,              // Change the protection
    MODIFY_TOHIT,                   // Change the to-hit
    MODIFY_VISION,                  // Change the vision
    MODIFY_FAME,                    // Change the fame
    MODIFY_GOLD,                    // Change the gold
    MODIFY_SWORD_DOMAIN_LEVEL,      // Change the sword domain level
    MODIFY_SWORD_DOMAIN_EXP,        // Change the sword domain experience
    MODIFY_SWORD_DOMAIN_GOAL_EXP,   // Change the sword domain goal experience
    MODIFY_BLADE_DOMAIN_LEVEL,      // Change the blade domain level
    MODIFY_BLADE_DOMAIN_EXP,        // Change the blade domain experience
    MODIFY_BLADE_DOMAIN_GOAL_EXP,   // Change the blade domain goal experience
    MODIFY_HEAL_DOMAIN_LEVEL,       // Change the heal domain level
    MODIFY_HEAL_DOMAIN_EXP,         // Change the heal domain experience
    MODIFY_HEAL_DOMAIN_GOAL_EXP,    // Change the heal domain goal experience
    MODIFY_ENCHANT_DOMAIN_LEVEL,    // Change the enchant domain level
    MODIFY_ENCHANT_DOMAIN_EXP,      // Change the enchant domain experience
    MODIFY_ENCHANT_DOMAIN_GOAL_EXP, // Change the enchant domain goal experience
    MODIFY_GUN_DOMAIN_LEVEL,        // Change the gun domain level
    MODIFY_GUN_DOMAIN_EXP,          // Change the gun domain experience
    MODIFY_GUN_DOMAIN_GOAL_EXP,     // Change the gun domain goal experience
    MODIFY_ETC_DOMAIN_LEVEL,        // Change the etc domain level
    MODIFY_ETC_DOMAIN_EXP,          // Change the etc domain experience
    MODIFY_ETC_DOMAIN_GOAL_EXP,     // Change the etc domain goal experience
    MODIFY_SKILL_LEVEL,             // Change a particular skill level
    MODIFY_LEVEL,                   // Change the vampire level
    MODIFY_EFFECT_STAT,             // Change an effect's state
    MODIFY_DURATION,                // Change an effect's duration
    MODIFY_BULLET,                  // Change the number of bullets in the gun being held
    MODIFY_BONUS_POINT,             // Change the vampire's bonus points
    MODIFY_DURABILITY,              // Change the durability of one of the items being held
    MODIFY_NOTORIETY,               // Change the notoriety
    MODIFY_VAMP_GOAL_EXP,           // Change the vampire's goal experience
    MODIFY_SILVER_DAMAGE,           // Change the silver damage
    MODIFY_ATTACK_SPEED,            // Change the attack speed
    MODIFY_ALIGNMENT,               // Alignment
    MODIFY_SILVER_DURABILITY,       // Change the silver coating amount
    MODIFY_REGEN_RATE,              // Change the regeneration rate per unit of time
    MODIFY_GUILDID,                 // Change the guild id
    MODIFY_RANK,                    // Rank step
    MODIFY_RANK_EXP,                // Rank experience
    MODIFY_OUSTERS_GOAL_EXP,        // Change the Ousters' goal experience
    MODIFY_SKILL_BONUS_POINT,       // Change the Ousters' skill bonus points

    MODIFY_ELEMENTAL_FIRE,
    MODIFY_ELEMENTAL_WATER,
    MODIFY_ELEMENTAL_EARTH,
    MODIFY_ELEMENTAL_WIND,

    MODIFY_SKILL_EXP, // Change the slayer skill experience

    MODIFY_PET_HP,
    MODIFY_PET_EXP, // Pet experience

    MODIFY_LAST_TARGET, // Change the last attacked target
    MODIFY_UNIONID,     // Guild union id
    MODIFY_UNIONGRADE,  // Position in the guild union

    MODIFY_ADVANCEMENT_CLASS_LEVEL,    // Advancement level
    MODIFY_ADVANCEMENT_CLASS_GOAL_EXP, // Advancement experience

    MODIFY_MAX
};

const string ModifyType2String[] = {"BASIC_STR",
                                    "CURRENT_STR",
                                    "MAX_STR",
                                    "STR_EXP",
                                    "BASIC_DEX",
                                    "CURRENT_DEX",
                                    "MAX_DEX",
                                    "DEX_EXP",
                                    "BASIC_INT",
                                    "CURRENT_INT",
                                    "MAX_INT",
                                    "INT_EXP",
                                    "CURRENT_HP",
                                    "MAX_HP",
                                    "CURRENT_MP",
                                    "MAX_MP",
                                    "MIN_DAMAGE",
                                    "MAX_DAMAGE",
                                    "DEFENSE",
                                    "PROTECTION",
                                    "TOHIT",
                                    "VISION",
                                    "FAME",
                                    "GOLD",
                                    "SWORD_DOMAIN_LEVEL",
                                    "SWORD_DOMAIN_EXP",
                                    "SWORD_DOMAIN_GOAL_EXP",
                                    "BLADE_DOMAIN_LEVEL",
                                    "BLADE_DOMAIN_EXP",
                                    "BLADE_DOMAIN_GOAL_EXP",
                                    "HEAL_DOMAIN_LEVEL",
                                    "HEAL_DOMAIN_EXP",
                                    "HEAL_DOMAIN_GOAL_EXP",
                                    "ENCHANT_DOMAIN_LEVEL",
                                    "ENCHANT_DOMAIN_EXP",
                                    "ENCHANT_DOMAIN_GOAL_EXP",
                                    "GUN_DOMAIN_LEVEL",
                                    "GUN_DOMAIN_EXP",
                                    "GUN_DOMAIN_GOAL_EXP",
                                    "ETC_DOMAIN_LEVEL",
                                    "ETC_DOMAIN_EXP",
                                    "ETC_DOMAIN_GOAL_EXP",
                                    "SKILL_LEVEL",
                                    "LEVEL",
                                    "EFFECT_STAT",
                                    "DURATION",
                                    "BULLET",
                                    "BONUS_POINT",
                                    "DURABILITY",
                                    "NOTORIETY",
                                    "VAMP_EXP",
                                    "SILVER_DAMAGE",
                                    "ATTACK_SPEED",
                                    "ALIGNMENT",
                                    "SILVER_DURABILITY",
                                    "REGEN_RATE",
                                    "GUILDID",
                                    "RANK",
                                    "RANK_EXP",
                                    "MODIFY_OUSTERS_EXP",
                                    "MODIFY_SKILL_BONUS_POINT",

                                    "MODIFY_ELEMENTAL_FIRE",
                                    "MODIFY_ELEMENTAL_WATER",
                                    "MODIFY_ELEMENTAL_EARTH",
                                    "MODIFY_ELEMENTAL_WIND",

                                    "MODIFY_SKILL_EXP", // Change the slayer skill experience

                                    "MODIFY_PET_HP",
                                    "MODIFY_PET_EXP", // Pet experience

                                    "MODIFY_LAST_TARGET", // Change the last attacked target
                                    "MODIFY_UNIONID",
                                    "MODIFY_UNIONGRADE",

                                    "MODIFY_ADVANCEMENT_CLASS_LEVEL",    // Advancement level
                                    "MODIFY_ADVANCEMENT_CLASS_GOAL_EXP", // Advancement experience

                                    "MAX"};

// A tag that names no modify type prints as its number.
inline string modifyType2String(BYTE type) {
    if (type >= MODIFY_MAX)
        return std::to_string((int)type);
    return ModifyType2String[type];
}

//////////////////////////////////////////////////////////////////////////////
// When the changed value fits in 2 bytes, this struct is used.
//////////////////////////////////////////////////////////////////////////////
typedef struct _SHORTDATA {
    BYTE type;
    ushort value;

} SHORTDATA;

//////////////////////////////////////////////////////////////////////////////
// When the changed value fits in 4 bytes, this struct is used.
//////////////////////////////////////////////////////////////////////////////
typedef struct _LONGDATA {
    BYTE type;
    DWORD value;

} LONGDATA;


//////////////////////////////////////////////////////////////////////////////
// class ModifyInfo;
// Object the game server uses to tell the client about its own changed
// data. It is carried in ModifyInformation, SkillToObjectOK and the like.
//////////////////////////////////////////////////////////////////////////////

class ModifyInfo : public Packet {
public:
    ModifyInfo();
    virtual ~ModifyInfo() noexcept;

public:
    // Each list is counted in a BYTE, and getPacketMaxSize() budgets this
    // many entries in each.
    static constexpr uint kMaxCount = 255;

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketSize_t getPacketSize() const {
        return (PacketSize_t)(szBYTE * 2 + m_ShortList.size() * (szBYTE + szshort) +
                              m_LongList.size() * (szBYTE + szDWORD));
    }
    static constexpr PacketSize_t getPacketMaxSize() {
        return szBYTE * 2 + kMaxCount * (szBYTE + szshort + szBYTE + szDWORD);
    }
    string toString() const;

public:
    BYTE getShortCount(void) const {
        return (BYTE)m_ShortList.size();
    }
    BYTE getLongCount(void) const {
        return (BYTE)m_LongList.size();
    }

    void addShortData(ModifyType type, ushort value);
    void addLongData(ModifyType type, ulong value);

    void popShortData(SHORTDATA& rData);
    void popLongData(LONGDATA& rData);

    void clearList(void) {
        m_ShortList.clear();
        m_LongList.clear();
    }

protected:
    list<SHORTDATA> m_ShortList;
    list<LONGDATA> m_LongList;
};

#endif
