//////////////////////////////////////////////////////////////////////
//
// Filename    : guild_union_registry_test.cpp
// Description : Pins which union a guild resolves to and what a retired
//               union answers
//               (src/server/gameserver/guild/GuildUnionRegistry.{h,cpp}).
//
//               The union manager is changed from the zone threads, the
//               sharedserver link and the loginserver link, and read from
//               all of them, so a union it takes away is retired rather
//               than freed: a reader may still hold the pointer. These
//               cases pin that a retired union resolves from nothing, keeps
//               its id and master, and names no guild as a member; that a
//               guild resolves to one union at a time; and that a reload
//               retires the whole previous set in one step.
//
//               Only GuildUnionRegistry.cpp is linked; ServerCore carries
//               Mutex.
//
//////////////////////////////////////////////////////////////////////

#include <atomic>
#include <list>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GuildUnionRegistry.h"

namespace {

std::unique_ptr<GuildUnion> makeUnion(uint unionID, GuildID_t master, std::list<GuildID_t> members = {}) {
    return std::make_unique<GuildUnion>(unionID, master, members);
}

TEST(GuildUnionRegistry, AGuildResolvesToItsUnionAsMasterAndAsMember) {
    GuildUnionRegistry registry;
    GuildUnion* pUnion = registry.publish(makeUnion(7, 100, {200, 300}));

    EXPECT_EQ(registry.unionOfGuild(100), pUnion) << "the master guild";
    EXPECT_EQ(registry.unionOfGuild(200), pUnion) << "a member guild";
    EXPECT_EQ(registry.unionOfGuild(300), pUnion);
    EXPECT_EQ(registry.unionByID(7), pUnion);

    EXPECT_EQ(registry.unionOfGuild(400), nullptr) << "a guild in no union";
    EXPECT_EQ(registry.unionByID(8), nullptr);
}

TEST(GuildUnionRegistry, ALiveUnionAnswersItsMembership) {
    GuildUnionRegistry registry;
    GuildUnion* pUnion = registry.publish(makeUnion(7, 100, {200}));

    EXPECT_FALSE(pUnion->isRetired());
    EXPECT_EQ(pUnion->getUnionID(), 7u);
    EXPECT_EQ(pUnion->getMasterGuildID(), 100);
    EXPECT_TRUE(pUnion->hasGuild(100));
    EXPECT_TRUE(pUnion->hasGuild(200));
    EXPECT_FALSE(pUnion->hasGuild(300));
    EXPECT_EQ(pUnion->getGuildList(), (std::list<GuildID_t>{200})) << "members only, not the master";
}

// The case the retirement is for: the pointer a reader took before the union
// was dissolved stays valid, and from then on names no guild.
TEST(GuildUnionRegistry, ARetiredUnionResolvesFromNothingButStaysReadable) {
    GuildUnionRegistry registry;
    GuildUnion* pUnion = registry.publish(makeUnion(7, 100, {200, 300}));

    ASSERT_TRUE(registry.retire(7));

    EXPECT_EQ(registry.unionOfGuild(100), nullptr);
    EXPECT_EQ(registry.unionOfGuild(200), nullptr);
    EXPECT_EQ(registry.unionOfGuild(300), nullptr);
    EXPECT_EQ(registry.unionByID(7), nullptr);

    EXPECT_TRUE(pUnion->isRetired());
    EXPECT_EQ(pUnion->getUnionID(), 7u) << "the id is kept: handing it back to the manager finds nothing";
    EXPECT_EQ(pUnion->getMasterGuildID(), 100) << "the master is kept";
    EXPECT_FALSE(pUnion->hasGuild(100)) << "no guild is in a retired union, not even its master";
    EXPECT_FALSE(pUnion->hasGuild(200));
    EXPECT_TRUE(pUnion->getGuildList().empty());
}

TEST(GuildUnionRegistry, AUnionIsRetiredOnce) {
    GuildUnionRegistry registry;
    registry.publish(makeUnion(7, 100, {200}));

    EXPECT_TRUE(registry.retire(7));
    EXPECT_FALSE(registry.retire(7));
    EXPECT_FALSE(registry.retire(8)) << "an id no union has";
}

TEST(GuildUnionRegistry, AnAddedMemberResolvesToTheUnion) {
    GuildUnionRegistry registry;
    GuildUnion* pUnion = registry.publish(makeUnion(7, 100));

    EXPECT_TRUE(registry.addMember(7, 200));

    EXPECT_EQ(registry.unionOfGuild(200), pUnion);
    EXPECT_TRUE(pUnion->hasGuild(200));
    EXPECT_EQ(pUnion->getGuildList(), (std::list<GuildID_t>{200}));
}

// A guild is in one union at a time, so a join that lost a race to another
// join is refused rather than recorded twice.
TEST(GuildUnionRegistry, AGuildAlreadyInAUnionIsNotAdded) {
    GuildUnionRegistry registry;
    GuildUnion* pFirst = registry.publish(makeUnion(7, 100, {200}));
    GuildUnion* pSecond = registry.publish(makeUnion(8, 300));

    EXPECT_FALSE(registry.addMember(8, 200)) << "a member of another union";
    EXPECT_FALSE(registry.addMember(8, 100)) << "the master of another union";
    EXPECT_FALSE(registry.addMember(7, 200)) << "already a member of this one";
    EXPECT_FALSE(registry.addMember(8, 300)) << "the union's own master";

    EXPECT_EQ(registry.unionOfGuild(200), pFirst);
    EXPECT_TRUE(pSecond->getGuildList().empty());
}

TEST(GuildUnionRegistry, NothingIsAddedToAUnionThatIsNotLive) {
    GuildUnionRegistry registry;
    GuildUnion* pUnion = registry.publish(makeUnion(7, 100));
    registry.retire(7);

    EXPECT_FALSE(registry.addMember(7, 200)) << "a retired union's id";
    EXPECT_FALSE(registry.addMember(9, 200)) << "an id no union has";
    EXPECT_EQ(registry.unionOfGuild(200), nullptr);
    EXPECT_TRUE(pUnion->getGuildList().empty());
}

TEST(GuildUnionRegistry, ARemovedMemberStopsResolving) {
    GuildUnionRegistry registry;
    GuildUnion* pUnion = registry.publish(makeUnion(7, 100, {200, 300}));

    EXPECT_TRUE(registry.removeMember(7, 200));

    EXPECT_EQ(registry.unionOfGuild(200), nullptr);
    EXPECT_EQ(registry.unionOfGuild(300), pUnion) << "the other member stays";
    EXPECT_EQ(registry.unionOfGuild(100), pUnion) << "and so does the master";
    EXPECT_EQ(pUnion->getGuildList(), (std::list<GuildID_t>{300}));
    EXPECT_FALSE(pUnion->isRetired()) << "removing a member does not dissolve the union";
}

TEST(GuildUnionRegistry, OnlyAListedMemberIsRemoved) {
    GuildUnionRegistry registry;
    GuildUnion* pUnion = registry.publish(makeUnion(7, 100, {200}));

    EXPECT_FALSE(registry.removeMember(7, 100)) << "the master is not a member";
    EXPECT_FALSE(registry.removeMember(7, 300)) << "a guild the union does not list";
    EXPECT_FALSE(registry.removeMember(9, 200)) << "an id no union has";

    EXPECT_EQ(registry.unionOfGuild(100), pUnion);
    EXPECT_EQ(registry.unionOfGuild(200), pUnion);

    registry.retire(7);
    EXPECT_FALSE(registry.removeMember(7, 200)) << "a retired union's id";
}

// A reload swaps the whole set: every union read before it is retired, and
// the same ids and guilds resolve to the objects loaded now.
TEST(GuildUnionRegistry, AReloadRetiresEveryPreviousUnion) {
    GuildUnionRegistry registry;
    GuildUnion* pOld = registry.publish(makeUnion(7, 100, {200}));
    GuildUnion* pGone = registry.publish(makeUnion(8, 300, {400}));

    std::vector<std::unique_ptr<GuildUnion>> fresh;
    fresh.push_back(makeUnion(7, 100, {200, 500}));
    GuildUnion* pNew = fresh.back().get();
    registry.replaceAll(std::move(fresh));

    EXPECT_TRUE(pOld->isRetired());
    EXPECT_TRUE(pGone->isRetired());
    EXPECT_FALSE(pNew->isRetired());

    EXPECT_EQ(registry.unionByID(7), pNew);
    EXPECT_EQ(registry.unionOfGuild(200), pNew);
    EXPECT_EQ(registry.unionOfGuild(500), pNew);
    EXPECT_EQ(registry.unionByID(8), nullptr) << "a union the tables no longer hold";
    EXPECT_EQ(registry.unionOfGuild(400), nullptr);
}

// Two unions listing one guild -- the tables do not forbid it -- leave it
// with the one published last; retiring the earlier one leaves that answer
// alone.
TEST(GuildUnionRegistry, RetiringAUnionLeavesTheEntriesAnotherTookOver) {
    GuildUnionRegistry registry;
    registry.publish(makeUnion(7, 100, {200}));
    GuildUnion* pLater = registry.publish(makeUnion(8, 300, {200}));

    EXPECT_EQ(registry.unionOfGuild(200), pLater);

    ASSERT_TRUE(registry.retire(7));
    EXPECT_EQ(registry.unionOfGuild(200), pLater);
    EXPECT_EQ(registry.unionOfGuild(100), nullptr);
}

// Readers look unions up and read them while another thread dissolves,
// joins and reloads. Every pointer a reader got stays readable, and a union
// it finds retired names no guild.
TEST(GuildUnionRegistry, ReadersSurviveConcurrentChanges) {
    GuildUnionRegistry registry;
    registry.publish(makeUnion(1, 100, {200}));

    std::atomic<bool> stop{false};
    std::atomic<long> reads{0};

    auto reader = [&] {
        while (!stop.load()) {
            GuildUnion* pUnion = registry.unionOfGuild(100);
            if (pUnion != NULL) {
                const std::list<GuildID_t> guilds = pUnion->getGuildList();
                if (pUnion->isRetired()) {
                    EXPECT_FALSE(pUnion->hasGuild(100));
                } else {
                    EXPECT_EQ(pUnion->getMasterGuildID(), 100);
                }
                reads.fetch_add(1 + static_cast<long>(guilds.size()));
            }
        }
    };

    std::thread first(reader);
    std::thread second(reader);

    // The changes start once a reader is in.
    while (reads.load() == 0)
        std::this_thread::yield();

    for (uint round = 2; round < 2000; round++) {
        registry.addMember(round - 1, 300);
        registry.removeMember(round - 1, 300);

        if (round % 3 == 0) {
            std::vector<std::unique_ptr<GuildUnion>> fresh;
            fresh.push_back(makeUnion(round, 100, {200}));
            registry.replaceAll(std::move(fresh));
        } else {
            registry.retire(round - 1);
            registry.publish(makeUnion(round, 100, {200}));
        }
    }

    stop.store(true);
    first.join();
    second.join();
}

} // namespace
