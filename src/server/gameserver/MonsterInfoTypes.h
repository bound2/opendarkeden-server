#ifndef __MONSTER_INFO_TYPES__
#define __MONSTER_INFO_TYPES__


//////////////////////////////////////////////////////////////////////////////
// Class Monster;
// Creatures controlled by AI that are mainly the combat targets of PCs.
//////////////////////////////////////////////////////////////////////////////
enum {
    CLAN_NONE,                    // Belongs to no clan            0
    CLAN_VAMPIRE_MONSTER,         // Wandering vampire monster     1
    CLAN_VAMPIRE_BATHORY_MONSTER, // Bathory clan monster          2
    CLAN_VAMPIRE_TEPEZ_MONSTER,   // Tepez clan monster            3
    CLAN_SLAYER_MONSTER,          // Slayer monster                4

    CLAN_MAX
};

extern int DefaultClanID[CLAN_MAX];

//////////////////////////////////////////////////////////////////////////////
// RegenType - how a monster appears
//
// The checking order matters too:
// Hide is the default and has the largest value, so it must be checked last.
//////////////////////////////////////////////////////////////////////////////
enum RegenType {
    REGENTYPE_PORTAL,    // Appears by coming through a portal
    REGENTYPE_INVISIBLE, // Appears in the invisible state
    REGENTYPE_BAT,       // Appears in bat form
    REGENTYPE_HIDE,      // Appears hidden under the ground

    REGENTYPE_MAX
};

#endif
