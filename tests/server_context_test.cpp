//----------------------------------------------------------------------
// server_context_test.cpp
//
// Pins src/server/ServerContext.h: the registry that carries the
// process-wide managers ServerCore defines -- the database connection
// table, the game-server table and the world table -- instead of the
// g_p* globals each server used to assign.
//
// The managers are never defined here. ServerContext.h forward-declares
// them and its accessors only bind a reference to the registered object,
// so a context can be built over stand-in pointers with none of the
// servers linked, and none of MySQL. The suite therefore links only
// de-kernel (for the Assert helper) and the context's own translation
// unit.
//
// A failing Assert appends to assertion_failed.log in the working
// directory, so the ctest entry runs this from the build tree.
//----------------------------------------------------------------------

#include <gtest/gtest.h>

#include "Exception.h"
#include "ServerContext.h"

namespace {

// Distinct addresses standing in for the managers. Nothing dereferences
// them: the context stores a pointer and hands back a reference to it.
char g_managerStorage[16];

template <class T> T* standIn(int slot) {
    return reinterpret_cast<T*>(&g_managerStorage[slot]);
}

} // namespace

TEST(ServerContextTest, RegisteredManagersAreReadBack) {
    de::ServerContext context;

    DatabaseManager* pDatabaseManager = standIn<DatabaseManager>(0);
    GameServerInfoManager* pGameServerInfoManager = standIn<GameServerInfoManager>(1);
    GameWorldInfoManager* pGameWorldInfoManager = standIn<GameWorldInfoManager>(2);

    context.setDatabaseManager(pDatabaseManager);
    context.setGameServerInfoManager(pGameServerInfoManager);
    context.setGameWorldInfoManager(pGameWorldInfoManager);

    EXPECT_EQ(&context.database(), pDatabaseManager);
    EXPECT_EQ(&context.serverInfos(), pGameServerInfoManager);
    EXPECT_EQ(&context.worldInfos(), pGameWorldInfoManager);
}

TEST(ServerContextTest, ReregisteringReplacesTheManager) {
    de::ServerContext context;

    context.setDatabaseManager(standIn<DatabaseManager>(0));
    context.setDatabaseManager(standIn<DatabaseManager>(1));

    EXPECT_EQ(&context.database(), standIn<DatabaseManager>(1));
}

TEST(ServerContextTest, UnregisteredManagerAsserts) {
    de::ServerContext context;

    // Reading a manager the startup code has not registered yet is a
    // startup-order bug, so every accessor asserts rather than returning
    // something the caller could test.
    EXPECT_THROW(context.database(), AssertionError);
    EXPECT_THROW(context.serverInfos(), AssertionError);
    EXPECT_THROW(context.worldInfos(), AssertionError);
}

TEST(ServerContextTest, RegisteringOneManagerLeavesTheOthersAsserting) {
    de::ServerContext context;

    // The shared server creates no game-server table, so a binary that
    // registers two of the three is the ordinary case, not a broken one.
    context.setDatabaseManager(standIn<DatabaseManager>(0));
    context.setGameWorldInfoManager(standIn<GameWorldInfoManager>(2));

    EXPECT_EQ(&context.database(), standIn<DatabaseManager>(0));
    EXPECT_EQ(&context.worldInfos(), standIn<GameWorldInfoManager>(2));
    EXPECT_THROW(context.serverInfos(), AssertionError);
}

TEST(ServerContextTest, ProcessWideContextIsOneInstance) {
    EXPECT_EQ(&de::serverContext(), &de::serverContext());
}
