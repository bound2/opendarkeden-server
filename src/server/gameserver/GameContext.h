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

class DatabaseManager;
class ItemFactoryManager;
class PCFinder;
class Properties;
class StringPool;
class VariableManager;
class ZoneGroupManager;
class ZoneInfoManager;

namespace de {

class GameContext {
public:
    GameContext() = default;

    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;

    void setConfig(Properties* pConfig) {
        m_pConfig = pConfig;
    }
    void setDatabaseManager(DatabaseManager* pDatabaseManager) {
        m_pDatabaseManager = pDatabaseManager;
    }
    void setItemFactoryManager(ItemFactoryManager* pItemFactoryManager) {
        m_pItemFactoryManager = pItemFactoryManager;
    }
    void setPCFinder(PCFinder* pPCFinder) {
        m_pPCFinder = pPCFinder;
    }
    void setStringPool(StringPool* pStringPool) {
        m_pStringPool = pStringPool;
    }
    void setVariableManager(VariableManager* pVariableManager) {
        m_pVariableManager = pVariableManager;
    }
    void setZoneGroupManager(ZoneGroupManager* pZoneGroupManager) {
        m_pZoneGroupManager = pZoneGroupManager;
    }
    void setZoneInfoManager(ZoneInfoManager* pZoneInfoManager) {
        m_pZoneInfoManager = pZoneInfoManager;
    }

    Properties& config() const;
    DatabaseManager& databases() const;
    ItemFactoryManager& itemFactories() const;
    PCFinder& playerCreatures() const;
    StringPool& strings() const;
    VariableManager& variables() const;
    ZoneGroupManager& zoneGroups() const;
    ZoneInfoManager& zoneInfos() const;

private:
    Properties* m_pConfig = nullptr;
    DatabaseManager* m_pDatabaseManager = nullptr;
    ItemFactoryManager* m_pItemFactoryManager = nullptr;
    PCFinder* m_pPCFinder = nullptr;
    StringPool* m_pStringPool = nullptr;
    VariableManager* m_pVariableManager = nullptr;
    ZoneGroupManager* m_pZoneGroupManager = nullptr;
    ZoneInfoManager* m_pZoneInfoManager = nullptr;
};

// The process-wide context the startup code fills. A converted subsystem is
// handed the context and never calls this; the call belongs at the boundary
// where a subsystem is created from code that still reads globals.
GameContext& gameContext();

} // namespace de

#endif
