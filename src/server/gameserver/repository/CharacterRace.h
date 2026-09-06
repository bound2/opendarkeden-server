#ifndef __CHARACTER_RACE_H__
#define __CHARACTER_RACE_H__

// Which race table one character's row lives in — shared by every
// repository that addresses the Slayer/Vampire/Ousters tables.
enum CharacterRace { CHARACTER_RACE_SLAYER = 0, CHARACTER_RACE_VAMPIRE = 1, CHARACTER_RACE_OUSTERS = 2 };

// The table name for a race, for building SQL inside the MySQL
// implementations. Repository-layer helper — game logic never sees it.
inline const char* characterRaceTable(CharacterRace race) {
    return race == CHARACTER_RACE_SLAYER ? "Slayer" : race == CHARACTER_RACE_VAMPIRE ? "Vampire" : "Ousters";
}

#endif
