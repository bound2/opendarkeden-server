//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildUnionTeardown.h
// Description : what becomes of a guild union when one of its guilds goes
//               away, kept apart from the union manager so it can be
//               exercised with neither the guild tables nor a database.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GUILD_UNION_TEARDOWN_H__
#define __GUILD_UNION_TEARDOWN_H__

#include <vector>

#include "Types.h"

// A union is one master guild plus the guilds that joined it. The master
// guild owns the union (GuildUnionInfo.MasterGuildID) and has no
// GuildUnionMember row of its own; every other guild in the union has one.
struct UnionTeardown {
    enum Action {
        // No union holds this guild: nothing to remove.
        NOTHING,
        // Drop this guild's member row. The union carries on.
        REMOVE_MEMBER,
        // The union goes: the member rows named below are removed and the
        // union's own row with them.
        DISSOLVE
    };

    Action action = NOTHING;

    // The guilds whose member rows this teardown removes, in the order it
    // removes them. No id appears twice.
    std::vector<GuildID_t> membersToRemove;

    // The guilds whose union standing changed and whose members must be told
    // so. No id appears twice.
    std::vector<GuildID_t> guildsToNotify;
};

// `memberGuilds` are the union's GuildUnionMember rows; the master guild is
// not among them. `unionKnown` is false when no union holds the guild at all,
// which makes the answer NOTHING whatever the other arguments say.
UnionTeardown decideUnionTeardown(bool unionKnown, GuildID_t unionMasterGuildID,
                                  const std::vector<GuildID_t>& memberGuilds, GuildID_t removedGuildID);

#endif
