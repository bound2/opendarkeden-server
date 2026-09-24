//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildUnionTeardown.cpp
// Description : decideUnionTeardown
//////////////////////////////////////////////////////////////////////////////

#include "GuildUnionTeardown.h"

namespace {

void addOnce(std::vector<GuildID_t>& ids, GuildID_t id) {
    for (size_t i = 0; i < ids.size(); i++) {
        if (ids[i] == id)
            return;
    }

    ids.push_back(id);
}

bool holds(const std::vector<GuildID_t>& ids, GuildID_t id) {
    for (size_t i = 0; i < ids.size(); i++) {
        if (ids[i] == id)
            return true;
    }

    return false;
}

} // namespace

UnionTeardown decideUnionTeardown(bool unionKnown, GuildID_t unionMasterGuildID,
                                  const std::vector<GuildID_t>& memberGuilds, GuildID_t removedGuildID) {
    UnionTeardown teardown;

    if (!unionKnown)
        return teardown;

    // The master guild owns the union and mastery cannot be handed on: the
    // union tables name one master and nothing says which member would
    // inherit it. So the union goes with its master and every member guild is
    // let out, each of them told that it no longer belongs to one.
    if (removedGuildID == unionMasterGuildID) {
        teardown.action = UnionTeardown::DISSOLVE;

        for (size_t m = 0; m < memberGuilds.size(); m++) {
            // A member row naming the master guild is a row that should not
            // exist; the dissolve removes it along with the rest.
            addOnce(teardown.membersToRemove, memberGuilds[m]);
            addOnce(teardown.guildsToNotify, memberGuilds[m]);
        }

        addOnce(teardown.guildsToNotify, unionMasterGuildID);

        return teardown;
    }

    // A guild with no member row is not in this union, whatever brought it
    // here.
    if (!holds(memberGuilds, removedGuildID))
        return teardown;

    addOnce(teardown.membersToRemove, removedGuildID);
    addOnce(teardown.guildsToNotify, removedGuildID);
    addOnce(teardown.guildsToNotify, unionMasterGuildID);

    // The master guild on its own is not a union, so the last member leaving
    // takes the union with it.
    bool othersRemain = false;
    for (size_t m = 0; m < memberGuilds.size() && !othersRemain; m++)
        othersRemain = memberGuilds[m] != removedGuildID;

    teardown.action = othersRemain ? UnionTeardown::REMOVE_MEMBER : UnionTeardown::DISSOLVE;

    return teardown;
}
