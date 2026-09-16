//////////////////////////////////////////////////////////////////////////////
// Filename    : PlayerRace.h
// Description : Maps a player creature's race onto the race table its rows
//               live in.
//////////////////////////////////////////////////////////////////////////////

#ifndef __PLAYER_RACE_H__
#define __PLAYER_RACE_H__

#include "Types.h"
#include "repository/CharacterRace.h"

// Every persistent player row is addressed by the race table it belongs to.
// The three player races map one to one onto those tables; anything else is
// answered as Slayer, the table a character without a race would be read from.
inline CharacterRace characterRaceOf(Race_t race) {
    switch (race) {
    case RACE_VAMPIRE:
        return CHARACTER_RACE_VAMPIRE;
    case RACE_OUSTERS:
        return CHARACTER_RACE_OUSTERS;
    default:
        return CHARACTER_RACE_SLAYER;
    }
}

#endif
