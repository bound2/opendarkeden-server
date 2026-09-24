//////////////////////////////////////////////////////////////////////
//
// Filename    : guild_retirement_test.cpp
// Description : Pins what a retired guild and a retired guild member
//               answer (src/server/gameserver/Guild.{h,cpp}).
//
//               A guild the sharedserver link tears down, and a member
//               it expels, are taken out of their map and parked rather
//               than freed, because a zone thread mid-tick may already
//               hold the pointer. The object therefore stays readable
//               with the rank, state and membership it had when it left.
//               These cases pin the answers that keep such a reader from
//               deciding on them: the member reports itself retired and
//               ranks as GUILDMEMBER_RANK_LEAVE, the guild reports itself
//               retired, reads as GUILD_STATE_BROKEN and hands out no
//               members, while the guild's own bookkeeping still sees the
//               rank the member actually carried.
//
//               Only Guild.cpp is linked; the guild tables are never
//               reached on these paths, so the repository accessor below
//               refuses every call.
//
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "FakeGuildRepository.h"
#include "Guild.h"

// The process-wide accessor Guild.cpp reads the guild tables through.
GuildRepository& defaultGuildRepository() {
    static FakeGuildRepository repository;
    return repository;
}

namespace {

GuildMember* newMember(const std::string& name, GuildMemberRank_t rank) {
    GuildMember* pMember = new GuildMember();
    pMember->setName(name);
    pMember->setRank(rank);
    return pMember;
}

// A member is born in a guild, not retired, and answers its own rank.
TEST(GuildRetirement, ALiveMemberAnswersItsRank) {
    Guild guild;
    guild.addMember(newMember("Reiot", GuildMember::GUILDMEMBER_RANK_MASTER));

    GuildMember* pMember = guild.getMember("Reiot");
    ASSERT_NE(pMember, nullptr);
    EXPECT_FALSE(pMember->isRetired());
    EXPECT_EQ(pMember->getRank(), GuildMember::GUILDMEMBER_RANK_MASTER);
    EXPECT_EQ(pMember->getStoredRank(), GuildMember::GUILDMEMBER_RANK_MASTER);
}

// The case the fix is for: the pointer a reader took before the removal
// stays valid, and from the removal on it ranks as someone who has left.
TEST(GuildRetirement, AnExpelledMemberRanksAsLeftForAReaderHoldingIt) {
    Guild guild;
    guild.addMember(newMember("Reiot", GuildMember::GUILDMEMBER_RANK_MASTER));

    GuildMember* pMember = guild.getMember("Reiot");
    ASSERT_NE(pMember, nullptr);

    guild.deleteMember("Reiot");

    EXPECT_EQ(guild.getMember("Reiot"), nullptr) << "the guild no longer holds the member";
    EXPECT_TRUE(pMember->isRetired()) << "the pointer a reader still holds says so";
    EXPECT_EQ(pMember->getRank(), GuildMember::GUILDMEMBER_RANK_LEAVE) << "a rank check on it fails closed";
    EXPECT_EQ(pMember->getStoredRank(), GuildMember::GUILDMEMBER_RANK_MASTER)
        << "the rank the member carried is still there for the guild's own use";
}

// The counters follow the rank the member actually carried, so retiring it
// cannot make a departure count against the wrong tally.
TEST(GuildRetirement, TheMemberCountersFollowTheStoredRank) {
    Guild guild;
    guild.addMember(newMember("Reiot", GuildMember::GUILDMEMBER_RANK_MASTER));
    guild.addMember(newMember("Sigi", GuildMember::GUILDMEMBER_RANK_NORMAL));
    guild.addMember(newMember("Bezz", GuildMember::GUILDMEMBER_RANK_WAIT));
    ASSERT_EQ(guild.getActiveMemberCount(), 2);
    ASSERT_EQ(guild.getWaitMemberCount(), 1);

    guild.deleteMember("Sigi");
    EXPECT_EQ(guild.getActiveMemberCount(), 1);
    EXPECT_EQ(guild.getWaitMemberCount(), 1);

    guild.deleteMember("Bezz");
    EXPECT_EQ(guild.getActiveMemberCount(), 1);
    EXPECT_EQ(guild.getWaitMemberCount(), 0) << "the applicant was counted as an applicant, not as gone";
}

// Guild teardown reports the rank each member left with -- the handler acts
// on it -- and retires every one of them.
TEST(GuildRetirement, RetireAllMembersReportsTheRanksAndRetiresTheMembers) {
    Guild guild;
    guild.addMember(newMember("Reiot", GuildMember::GUILDMEMBER_RANK_MASTER));
    guild.addMember(newMember("Sigi", GuildMember::GUILDMEMBER_RANK_NORMAL));

    GuildMember* pMaster = guild.getMember("Reiot");
    GuildMember* pNormal = guild.getMember("Sigi");
    ASSERT_NE(pMaster, nullptr);
    ASSERT_NE(pNormal, nullptr);

    std::vector<std::pair<std::string, GuildMemberRank_t>> members = guild.retireAllMembers();

    ASSERT_EQ(members.size(), 2u);
    std::sort(members.begin(), members.end());
    EXPECT_EQ(members[0].first, "Reiot");
    EXPECT_EQ(members[0].second, GuildMember::GUILDMEMBER_RANK_MASTER);
    EXPECT_EQ(members[1].first, "Sigi");
    EXPECT_EQ(members[1].second, GuildMember::GUILDMEMBER_RANK_NORMAL);

    EXPECT_TRUE(pMaster->isRetired());
    EXPECT_TRUE(pNormal->isRetired());
    EXPECT_EQ(pMaster->getRank(), GuildMember::GUILDMEMBER_RANK_LEAVE);
    EXPECT_EQ(guild.getActiveMemberCount(), 0);
    EXPECT_EQ(guild.getMember("Reiot"), nullptr);
}

// A guild taken out of GuildManager's table reads as disbanded and hands out
// no members, and the members it had are retired with it.
TEST(GuildRetirement, ARetiredGuildIsBrokenAndHasNoMembers) {
    Guild guild;
    guild.setState(Guild::GUILD_STATE_ACTIVE);
    guild.addMember(newMember("Reiot", GuildMember::GUILDMEMBER_RANK_MASTER));

    GuildMember* pMember = guild.getMember("Reiot");
    ASSERT_NE(pMember, nullptr);
    ASSERT_EQ(guild.getState(), Guild::GUILD_STATE_ACTIVE);
    ASSERT_FALSE(guild.isRetired());

    guild.retire();

    EXPECT_TRUE(guild.isRetired());
    EXPECT_EQ(guild.getState(), Guild::GUILD_STATE_BROKEN) << "every 'is this guild active' gate closes";
    EXPECT_EQ(guild.getMember("Reiot"), nullptr) << "a retired guild has no members";
    EXPECT_TRUE(pMember->isRetired()) << "the members go with the guild";
    EXPECT_EQ(pMember->getRank(), GuildMember::GUILDMEMBER_RANK_LEAVE);
}

// Retirement is one way: a guild that was never retired keeps answering for
// itself, so the flag cannot be mistaken for a state the server sets.
TEST(GuildRetirement, ALiveGuildKeepsItsState) {
    Guild guild;
    guild.setState(Guild::GUILD_STATE_WAIT);
    EXPECT_FALSE(guild.isRetired());
    EXPECT_EQ(guild.getState(), Guild::GUILD_STATE_WAIT);

    guild.setState(Guild::GUILD_STATE_ACTIVE);
    EXPECT_EQ(guild.getState(), Guild::GUILD_STATE_ACTIVE);
}

} // namespace
