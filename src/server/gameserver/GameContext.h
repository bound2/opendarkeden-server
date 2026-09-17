//////////////////////////////////////////////////////////////////////////////
// Filename    : GameContext.h
// Description : The managers a gameserver subsystem works against, handed to
//               it explicitly instead of looked up through a global.
//
//               A subsystem that takes a GameContext& states in its
//               constructor which managers it needs, and can be built in a
//               test over stand-in ones. The context does NOT own the
//               managers: each is still created and destroyed by the startup
//               code that holds it (GameServer for the server-wide ones,
//               ObjectManager for the world ones), and registers itself here
//               as soon as it exists. A manager is therefore null until its
//               creation point is reached, and an accessor asserts on a null
//               one: reading a manager before it exists is a startup-order
//               bug, not a runtime condition to branch on.
//
//               Only forward declarations live here, so the header costs a
//               caller nothing and can be included where none of the
//               managers are linked.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GAME_CONTEXT_H__
#define __GAME_CONTEXT_H__

class ActionFactoryManager;
class ConditionFactoryManager;
class DatabaseManager;
class DynamicZoneFactoryManager;
class ItemFactoryManager;
class MonsterNameManager;
class PCFinder;
class Properties;
class ScriptManager;
class ShopTemplateManager;
class StringPool;
class VariableManager;
class WeatherInfoManager;
class ZoneGroupManager;
class ZoneInfoManager;

namespace de {

class GameContext {
public:
    GameContext() = default;

    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;

    void setActionFactoryManager(ActionFactoryManager* pActionFactoryManager) {
        m_pActionFactoryManager = pActionFactoryManager;
    }
    void setConditionFactoryManager(ConditionFactoryManager* pConditionFactoryManager) {
        m_pConditionFactoryManager = pConditionFactoryManager;
    }
    void setConfig(Properties* pConfig) {
        m_pConfig = pConfig;
    }
    void setDatabaseManager(DatabaseManager* pDatabaseManager) {
        m_pDatabaseManager = pDatabaseManager;
    }
    void setDynamicZoneFactoryManager(DynamicZoneFactoryManager* pDynamicZoneFactoryManager) {
        m_pDynamicZoneFactoryManager = pDynamicZoneFactoryManager;
    }
    void setItemFactoryManager(ItemFactoryManager* pItemFactoryManager) {
        m_pItemFactoryManager = pItemFactoryManager;
    }
    void setMonsterNameManager(MonsterNameManager* pMonsterNameManager) {
        m_pMonsterNameManager = pMonsterNameManager;
    }
    void setPCFinder(PCFinder* pPCFinder) {
        m_pPCFinder = pPCFinder;
    }
    void setPublicScriptManager(ScriptManager* pPublicScriptManager) {
        m_pPublicScriptManager = pPublicScriptManager;
    }
    void setShopTemplateManager(ShopTemplateManager* pShopTemplateManager) {
        m_pShopTemplateManager = pShopTemplateManager;
    }
    void setStringPool(StringPool* pStringPool) {
        m_pStringPool = pStringPool;
    }
    void setVariableManager(VariableManager* pVariableManager) {
        m_pVariableManager = pVariableManager;
    }
    void setWeatherInfoManager(WeatherInfoManager* pWeatherInfoManager) {
        m_pWeatherInfoManager = pWeatherInfoManager;
    }
    void setZoneGroupManager(ZoneGroupManager* pZoneGroupManager) {
        m_pZoneGroupManager = pZoneGroupManager;
    }
    void setZoneInfoManager(ZoneInfoManager* pZoneInfoManager) {
        m_pZoneInfoManager = pZoneInfoManager;
    }

    ActionFactoryManager& actionFactories() const;
    ConditionFactoryManager& conditionFactories() const;
    Properties& config() const;
    DatabaseManager& databases() const;
    DynamicZoneFactoryManager& dynamicZoneFactories() const;
    ItemFactoryManager& itemFactories() const;
    MonsterNameManager& monsterNames() const;
    PCFinder& playerCreatures() const;
    ScriptManager& publicScripts() const;
    ShopTemplateManager& shopTemplates() const;
    StringPool& strings() const;
    VariableManager& variables() const;
    WeatherInfoManager& weatherInfos() const;
    ZoneGroupManager& zoneGroups() const;
    ZoneInfoManager& zoneInfos() const;

private:
    ActionFactoryManager* m_pActionFactoryManager = nullptr;
    ConditionFactoryManager* m_pConditionFactoryManager = nullptr;
    Properties* m_pConfig = nullptr;
    DatabaseManager* m_pDatabaseManager = nullptr;
    DynamicZoneFactoryManager* m_pDynamicZoneFactoryManager = nullptr;
    ItemFactoryManager* m_pItemFactoryManager = nullptr;
    MonsterNameManager* m_pMonsterNameManager = nullptr;
    PCFinder* m_pPCFinder = nullptr;
    ScriptManager* m_pPublicScriptManager = nullptr;
    ShopTemplateManager* m_pShopTemplateManager = nullptr;
    StringPool* m_pStringPool = nullptr;
    VariableManager* m_pVariableManager = nullptr;
    WeatherInfoManager* m_pWeatherInfoManager = nullptr;
    ZoneGroupManager* m_pZoneGroupManager = nullptr;
    ZoneInfoManager* m_pZoneInfoManager = nullptr;
};

// The process-wide context the startup code fills. A converted subsystem is
// handed the context and never calls this; the call belongs at the boundary
// where a subsystem is created from code that still reads globals.
GameContext& gameContext();

} // namespace de

#endif
