//////////////////////////////////////////////////////////////////////////////
// Filename    : CastleOwnerDecision.h
// Description : who a castle passes to when a castle war is won and when the
//               guild holding it is deleted. Both changes run on the castle
//               zone's thread, posted there by the thread that decided them,
//               so each is settled against the castle and the guild table as
//               they stand when it runs. Kept apart from the managers so the
//               rule can be exercised without a zone, a guild or a castle.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CASTLE_OWNER_DECISION_H__
#define __CASTLE_OWNER_DECISION_H__

#include <optional>

#include "CommonGuild.h"
#include "Types.h"

struct CastleOwner {
    Race_t race;
    GuildID_t guildID;

    bool operator==(const CastleOwner& other) const {
        return race == other.race && guildID == other.guildID;
    }
};

// The owner a won castle war hands the castle to: the winning guild, or, when
// that guild was deleted between the war's end and the change, the common
// guild of the winner's race. A common guild id always exists. The deletion
// of a guild turns the castles it holds common; a deletion that runs before
// the war's change finds the castle still held by the old owner and leaves
// it, so the war's change makes the same decision itself.
inline CastleOwner castleWarWinnerOwner(Race_t winnerRace, GuildID_t winnerGuildID, bool winnerGuildExists) {
    if (isCommonGuildID(winnerGuildID) || winnerGuildExists)
        return CastleOwner{winnerRace, winnerGuildID};
    return CastleOwner{winnerRace, commonGuildIDOf(winnerRace)};
}

// The owner a castle passes to when the guild deletedGuildID is deleted: its
// race's common guild, if the castle is still held by the deleted guild when
// the change runs, and no change otherwise -- a war's end or a GM may have
// handed the castle on since the deletion was decided. The Ousters common
// guild stands for any race that is neither Slayer nor Vampire.
inline std::optional<CastleOwner> castleOwnerAfterGuildDeleted(Race_t castleRace, GuildID_t castleGuildID,
                                                               GuildID_t deletedGuildID) {
    if (castleGuildID != deletedGuildID || isCommonGuildID(deletedGuildID))
        return std::nullopt;
    if (castleRace == RACE_SLAYER || castleRace == RACE_VAMPIRE)
        return CastleOwner{castleRace, commonGuildIDOf(castleRace)};
    return CastleOwner{RACE_OUSTERS, OustersCommon};
}

#endif // __CASTLE_OWNER_DECISION_H__
