//----------------------------------------------------------------------
// login_context_test.cpp
//
// Pins src/server/loginserver/LoginContext.h: the registry a loginserver
// subsystem is handed instead of reading the g_p* globals.
//
// The managers are never defined here. LoginContext.h forward-declares
// them and its accessors only bind a reference to the registered object,
// so a context can be built over stand-in pointers with none of the
// loginserver linked -- which is what makes a subsystem that takes a
// LoginContext& testable at all. The suite therefore links only de-kernel
// (for the Assert helper) and the context's own translation unit.
//
// A failing Assert appends to assertion_failed.log in the working
// directory, so the ctest entry runs this from the build tree.
//----------------------------------------------------------------------

#include <gtest/gtest.h>

#include "Exception.h"
#include "LoginContext.h"

namespace {

// Distinct addresses standing in for the managers. Nothing dereferences
// them: the context stores a pointer and hands back a reference to it.
char g_managerStorage[16];

template <class T> T* standIn(int slot) {
    return reinterpret_cast<T*>(&g_managerStorage[slot]);
}

} // namespace

TEST(LoginContextTest, StartupTableManagersAreReadBack) {
    de::LoginContext context;

    GameServerGroupInfoManager* pGameServerGroupInfoManager = standIn<GameServerGroupInfoManager>(0);
    UserInfoManager* pUserInfoManager = standIn<UserInfoManager>(1);
    ZoneGroupInfoManager* pZoneGroupInfoManager = standIn<ZoneGroupInfoManager>(2);
    ZoneInfoManager* pZoneInfoManager = standIn<ZoneInfoManager>(3);

    context.setGameServerGroupInfoManager(pGameServerGroupInfoManager);
    context.setUserInfoManager(pUserInfoManager);
    context.setZoneGroupInfoManager(pZoneGroupInfoManager);
    context.setZoneInfoManager(pZoneInfoManager);

    EXPECT_EQ(&context.gameServerGroups(), pGameServerGroupInfoManager);
    EXPECT_EQ(&context.userInfos(), pUserInfoManager);
    EXPECT_EQ(&context.zoneGroupInfos(), pZoneGroupInfoManager);
    EXPECT_EQ(&context.zoneInfos(), pZoneInfoManager);
}

TEST(LoginContextTest, SessionManagersAreReadBack) {
    de::LoginContext context;

    GameServerManager* pGameServerManager = standIn<GameServerManager>(4);
    LoginPlayerManager* pLoginPlayerManager = standIn<LoginPlayerManager>(5);
    ReconnectLoginInfoManager* pReconnectLoginInfoManager = standIn<ReconnectLoginInfoManager>(6);

    context.setGameServerManager(pGameServerManager);
    context.setLoginPlayerManager(pLoginPlayerManager);
    context.setReconnectLoginInfoManager(pReconnectLoginInfoManager);

    EXPECT_EQ(&context.gameServers(), pGameServerManager);
    EXPECT_EQ(&context.loginPlayers(), pLoginPlayerManager);
    EXPECT_EQ(&context.reconnectLogins(), pReconnectLoginInfoManager);
}

TEST(LoginContextTest, ReregisteringReplacesTheManager) {
    de::LoginContext context;

    context.setUserInfoManager(standIn<UserInfoManager>(0));
    context.setUserInfoManager(standIn<UserInfoManager>(1));

    EXPECT_EQ(&context.userInfos(), standIn<UserInfoManager>(1));
}

TEST(LoginContextTest, UnregisteredManagerAsserts) {
    de::LoginContext context;

    // Reading a manager the startup code has not registered yet is a
    // startup-order bug, so every accessor asserts rather than returning
    // something the caller could test.
    EXPECT_THROW(context.gameServerGroups(), AssertionError);
    EXPECT_THROW(context.gameServers(), AssertionError);
    EXPECT_THROW(context.loginPlayers(), AssertionError);
    EXPECT_THROW(context.reconnectLogins(), AssertionError);
    EXPECT_THROW(context.userInfos(), AssertionError);
    EXPECT_THROW(context.zoneGroupInfos(), AssertionError);
    EXPECT_THROW(context.zoneInfos(), AssertionError);
}

TEST(LoginContextTest, RegisteringOneManagerLeavesTheOthersAsserting) {
    de::LoginContext context;

    context.setZoneInfoManager(standIn<ZoneInfoManager>(3));

    EXPECT_EQ(&context.zoneInfos(), standIn<ZoneInfoManager>(3));
    EXPECT_THROW(context.userInfos(), AssertionError);
}

TEST(LoginContextTest, ProcessWideContextIsOneInstance) {
    EXPECT_EQ(&de::loginContext(), &de::loginContext());
}
