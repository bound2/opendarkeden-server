//////////////////////////////////////////////////////////////////////////////
// Filename    : CommonGuild.h
// Description : the guild ids that stand for a race's players outside any
//               guild. A castle held by one of them is its race's common
//               castle; a player who joins no guild reports its race's id.
//////////////////////////////////////////////////////////////////////////////

#ifndef __COMMON_GUILD_H__
#define __COMMON_GUILD_H__

#include "Types.h"

inline constexpr GuildID_t SlayerCommon = 99;
inline constexpr GuildID_t VampireCommon = 0;
inline constexpr GuildID_t OustersCommon = 66;

inline bool isCommonGuildID(GuildID_t guildID) {
    return guildID == SlayerCommon || guildID == VampireCommon || guildID == OustersCommon;
}

// The common guild of a race. A race that is neither Slayer nor Vampire is
// the Ousters.
inline GuildID_t commonGuildIDOf(Race_t race) {
    if (race == RACE_SLAYER)
        return SlayerCommon;
    if (race == RACE_VAMPIRE)
        return VampireCommon;
    return OustersCommon;
}

#endif // __COMMON_GUILD_H__
