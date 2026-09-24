// What becomes of a guild union when one of its guilds goes away
// (src/server/gameserver/guild/GuildUnionTeardown.cpp): which member rows
// the teardown removes, whether the union survives it, and which guilds are
// told. The union manager itself is not linked - it needs the guild tables,
// the PC finder and a socket, which is why the decision was split out of it.

#include <vector>

#include <gtest/gtest.h>

#include "GuildUnionTeardown.h"

namespace {

const GuildID_t kMasterGuild = 3110;
const GuildID_t kMemberA = 3105;
const GuildID_t kMemberB = 3107;
const GuildID_t kStranger = 9999;

typedef std::vector<GuildID_t> Guilds;

//////////////////////////////////////////////////////////////////////////////
// A guild in no union
//////////////////////////////////////////////////////////////////////////////

TEST(GuildUnionTeardownTest, NoUnionRemovesNothing) {
    const UnionTeardown teardown = decideUnionTeardown(false, 0, Guilds(), kMemberA);

    EXPECT_EQ(UnionTeardown::NOTHING, teardown.action);
    EXPECT_TRUE(teardown.membersToRemove.empty());
    EXPECT_TRUE(teardown.guildsToNotify.empty());
}

// A union exists, but this guild is neither its master nor one of its
// members: a row that names it elsewhere is not this union's business.
TEST(GuildUnionTeardownTest, StrangerToTheUnionRemovesNothing) {
    const Guilds members{kMemberA, kMemberB};

    const UnionTeardown teardown = decideUnionTeardown(true, kMasterGuild, members, kStranger);

    EXPECT_EQ(UnionTeardown::NOTHING, teardown.action);
    EXPECT_TRUE(teardown.membersToRemove.empty());
    EXPECT_TRUE(teardown.guildsToNotify.empty());
}

//////////////////////////////////////////////////////////////////////////////
// A member guild leaves
//////////////////////////////////////////////////////////////////////////////

TEST(GuildUnionTeardownTest, MemberLeavesAndTheUnionStays) {
    const Guilds members{kMemberA, kMemberB};

    const UnionTeardown teardown = decideUnionTeardown(true, kMasterGuild, members, kMemberA);

    EXPECT_EQ(UnionTeardown::REMOVE_MEMBER, teardown.action);
    EXPECT_EQ(Guilds{kMemberA}, teardown.membersToRemove);
    EXPECT_EQ((Guilds{kMemberA, kMasterGuild}), teardown.guildsToNotify);
}

// The master guild alone is not a union, so the last member out takes the
// union with him - and both are still told.
TEST(GuildUnionTeardownTest, LastMemberLeavingDissolvesTheUnion) {
    const Guilds members{kMemberA};

    const UnionTeardown teardown = decideUnionTeardown(true, kMasterGuild, members, kMemberA);

    EXPECT_EQ(UnionTeardown::DISSOLVE, teardown.action);
    EXPECT_EQ(Guilds{kMemberA}, teardown.membersToRemove);
    EXPECT_EQ((Guilds{kMemberA, kMasterGuild}), teardown.guildsToNotify);
}

// A member named twice by the rows is removed once and notified once.
TEST(GuildUnionTeardownTest, DuplicateMemberRowsCollapse) {
    const Guilds members{kMemberA, kMemberA};

    const UnionTeardown teardown = decideUnionTeardown(true, kMasterGuild, members, kMemberA);

    EXPECT_EQ(UnionTeardown::DISSOLVE, teardown.action);
    EXPECT_EQ(Guilds{kMemberA}, teardown.membersToRemove);
    EXPECT_EQ((Guilds{kMemberA, kMasterGuild}), teardown.guildsToNotify);
}

//////////////////////////////////////////////////////////////////////////////
// The master guild goes
//////////////////////////////////////////////////////////////////////////////

// Mastery cannot be handed on: no column says which member would inherit it,
// so the union goes and every member is let out and told.
TEST(GuildUnionTeardownTest, MasterLeavingDissolvesTheUnion) {
    const Guilds members{kMemberA, kMemberB};

    const UnionTeardown teardown = decideUnionTeardown(true, kMasterGuild, members, kMasterGuild);

    EXPECT_EQ(UnionTeardown::DISSOLVE, teardown.action);
    EXPECT_EQ((Guilds{kMemberA, kMemberB}), teardown.membersToRemove);
    EXPECT_EQ((Guilds{kMemberA, kMemberB, kMasterGuild}), teardown.guildsToNotify);
}

// A union whose member rows have all gone still has its own row, and the
// master's departure still takes it.
TEST(GuildUnionTeardownTest, MasterLeavingAnEmptyUnionDissolvesIt) {
    const UnionTeardown teardown = decideUnionTeardown(true, kMasterGuild, Guilds(), kMasterGuild);

    EXPECT_EQ(UnionTeardown::DISSOLVE, teardown.action);
    EXPECT_TRUE(teardown.membersToRemove.empty());
    EXPECT_EQ(Guilds{kMasterGuild}, teardown.guildsToNotify);
}

// A member row naming the master guild should not exist; the dissolve takes
// it with the rest and names the guild once.
TEST(GuildUnionTeardownTest, MasterWithAMemberRowOfItsOwnIsNamedOnce) {
    const Guilds members{kMasterGuild, kMemberA};

    const UnionTeardown teardown = decideUnionTeardown(true, kMasterGuild, members, kMasterGuild);

    EXPECT_EQ(UnionTeardown::DISSOLVE, teardown.action);
    EXPECT_EQ((Guilds{kMasterGuild, kMemberA}), teardown.membersToRemove);
    EXPECT_EQ((Guilds{kMasterGuild, kMemberA}), teardown.guildsToNotify);
}

} // namespace
