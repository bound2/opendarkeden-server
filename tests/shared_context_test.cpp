//----------------------------------------------------------------------
// shared_context_test.cpp
//
// Pins src/server/sharedserver/SharedContext.h: the registry a
// sharedserver subsystem is handed instead of reading the g_p* globals.
//
// The managers are never defined here. SharedContext.h forward-declares
// them and its accessors only bind a reference to the registered object,
// so a context can be built over stand-in pointers with none of the
// sharedserver linked -- which is what makes a subsystem that takes a
// SharedContext& testable at all. The suite therefore links only
// de-kernel (for the Assert helper) and the context's own translation
// unit.
//
// A failing Assert appends to assertion_failed.log in the working
// directory, so the ctest entry runs this from the build tree.
//----------------------------------------------------------------------

#include <gtest/gtest.h>

#include "Exception.h"
#include "SharedContext.h"

namespace {

// Distinct addresses standing in for the managers. Nothing dereferences
// them: the context stores a pointer and hands back a reference to it.
char g_managerStorage[16];

template <class T> T* standIn(int slot) {
    return reinterpret_cast<T*>(&g_managerStorage[slot]);
}

} // namespace

TEST(SharedContextTest, RegisteredManagersAreReadBack) {
    de::SharedContext context;

    GameServerManager* pGameServerManager = standIn<GameServerManager>(0);
    GuildManager* pGuildManager = standIn<GuildManager>(1);
    StringPool* pStringPool = standIn<StringPool>(2);

    context.setGameServerManager(pGameServerManager);
    context.setGuildManager(pGuildManager);
    context.setStringPool(pStringPool);

    EXPECT_EQ(&context.gameServers(), pGameServerManager);
    EXPECT_EQ(&context.guilds(), pGuildManager);
    EXPECT_EQ(&context.strings(), pStringPool);
}

TEST(SharedContextTest, ReregisteringReplacesTheManager) {
    de::SharedContext context;

    context.setGuildManager(standIn<GuildManager>(0));
    context.setGuildManager(standIn<GuildManager>(1));

    EXPECT_EQ(&context.guilds(), standIn<GuildManager>(1));
}

TEST(SharedContextTest, UnregisteredManagerAsserts) {
    de::SharedContext context;

    // Reading a manager the startup code has not registered yet is a
    // startup-order bug, so every accessor asserts rather than returning
    // something the caller could test.
    EXPECT_THROW(context.gameServers(), AssertionError);
    EXPECT_THROW(context.guilds(), AssertionError);
    EXPECT_THROW(context.strings(), AssertionError);
}

TEST(SharedContextTest, RegisteringOneManagerLeavesTheOthersAsserting) {
    de::SharedContext context;

    context.setStringPool(standIn<StringPool>(2));

    EXPECT_EQ(&context.strings(), standIn<StringPool>(2));
    EXPECT_THROW(context.guilds(), AssertionError);
}

TEST(SharedContextTest, ProcessWideContextIsOneInstance) {
    EXPECT_EQ(&de::sharedContext(), &de::sharedContext());
}
