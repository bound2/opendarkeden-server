//----------------------------------------------------------------------
// game_context_test.cpp
//
// Pins src/server/gameserver/GameContext.h: the registry a converted
// subsystem is handed instead of reading the g_p* globals.
//
// The managers are never defined here. GameContext.h forward-declares
// them and its accessors only bind a reference to the registered object,
// so a context can be built over stand-in pointers with none of the
// gameserver linked -- which is what makes a subsystem that takes a
// GameContext& testable at all. The suite therefore links only de-kernel
// (for the Assert helper) and the context's own translation unit.
//
// A failing Assert appends to assertion_failed.log in the working
// directory, so the ctest entry runs this from the build tree.
//----------------------------------------------------------------------

#include <gtest/gtest.h>

#include "Exception.h"
#include "GameContext.h"

namespace {

// Distinct addresses standing in for the managers. Nothing dereferences
// them: the context stores a pointer and hands back a reference to it.
char g_managerStorage[12];

template <class T> T* standIn(int slot) {
    return reinterpret_cast<T*>(&g_managerStorage[slot]);
}

} // namespace

TEST(GameContextTest, AccessorReturnsTheRegisteredManager) {
    de::GameContext context;

    Properties* pConfig = standIn<Properties>(0);
    DatabaseManager* pDatabaseManager = standIn<DatabaseManager>(1);
    ItemFactoryManager* pItemFactoryManager = standIn<ItemFactoryManager>(2);
    PCFinder* pPCFinder = standIn<PCFinder>(3);
    StringPool* pStringPool = standIn<StringPool>(4);
    VariableManager* pVariableManager = standIn<VariableManager>(5);
    ZoneGroupManager* pZoneGroupManager = standIn<ZoneGroupManager>(6);
    ZoneInfoManager* pZoneInfoManager = standIn<ZoneInfoManager>(7);

    context.setConfig(pConfig);
    context.setDatabaseManager(pDatabaseManager);
    context.setItemFactoryManager(pItemFactoryManager);
    context.setPCFinder(pPCFinder);
    context.setStringPool(pStringPool);
    context.setVariableManager(pVariableManager);
    context.setZoneGroupManager(pZoneGroupManager);
    context.setZoneInfoManager(pZoneInfoManager);

    EXPECT_EQ(&context.config(), pConfig);
    EXPECT_EQ(&context.databases(), pDatabaseManager);
    EXPECT_EQ(&context.itemFactories(), pItemFactoryManager);
    EXPECT_EQ(&context.playerCreatures(), pPCFinder);
    EXPECT_EQ(&context.strings(), pStringPool);
    EXPECT_EQ(&context.variables(), pVariableManager);
    EXPECT_EQ(&context.zoneGroups(), pZoneGroupManager);
    EXPECT_EQ(&context.zoneInfos(), pZoneInfoManager);
}

TEST(GameContextTest, QuestScriptingManagersAreReadBack) {
    de::GameContext context;

    ActionFactoryManager* pActionFactoryManager = standIn<ActionFactoryManager>(8);
    ConditionFactoryManager* pConditionFactoryManager = standIn<ConditionFactoryManager>(9);
    ScriptManager* pPublicScriptManager = standIn<ScriptManager>(10);
    ShopTemplateManager* pShopTemplateManager = standIn<ShopTemplateManager>(11);

    context.setActionFactoryManager(pActionFactoryManager);
    context.setConditionFactoryManager(pConditionFactoryManager);
    context.setPublicScriptManager(pPublicScriptManager);
    context.setShopTemplateManager(pShopTemplateManager);

    EXPECT_EQ(&context.actionFactories(), pActionFactoryManager);
    EXPECT_EQ(&context.conditionFactories(), pConditionFactoryManager);
    EXPECT_EQ(&context.publicScripts(), pPublicScriptManager);
    EXPECT_EQ(&context.shopTemplates(), pShopTemplateManager);
}

TEST(GameContextTest, ReregisteringReplacesTheManager) {
    de::GameContext context;

    context.setConfig(standIn<Properties>(0));
    context.setConfig(standIn<Properties>(1));

    EXPECT_EQ(&context.config(), standIn<Properties>(1));
}

TEST(GameContextTest, UnregisteredManagerAsserts) {
    de::GameContext context;

    // Reading a manager the startup code has not registered yet is a
    // startup-order bug, so every accessor asserts rather than returning
    // something the caller could test.
    EXPECT_THROW(context.actionFactories(), AssertionError);
    EXPECT_THROW(context.conditionFactories(), AssertionError);
    EXPECT_THROW(context.config(), AssertionError);
    EXPECT_THROW(context.databases(), AssertionError);
    EXPECT_THROW(context.itemFactories(), AssertionError);
    EXPECT_THROW(context.playerCreatures(), AssertionError);
    EXPECT_THROW(context.publicScripts(), AssertionError);
    EXPECT_THROW(context.shopTemplates(), AssertionError);
    EXPECT_THROW(context.strings(), AssertionError);
    EXPECT_THROW(context.variables(), AssertionError);
    EXPECT_THROW(context.zoneGroups(), AssertionError);
    EXPECT_THROW(context.zoneInfos(), AssertionError);
}

TEST(GameContextTest, RegisteringOneManagerLeavesTheOthersAsserting) {
    de::GameContext context;

    context.setZoneGroupManager(standIn<ZoneGroupManager>(6));

    EXPECT_EQ(&context.zoneGroups(), standIn<ZoneGroupManager>(6));
    EXPECT_THROW(context.config(), AssertionError);
}

TEST(GameContextTest, ProcessWideContextIsOneInstance) {
    EXPECT_EQ(&de::gameContext(), &de::gameContext());
}
