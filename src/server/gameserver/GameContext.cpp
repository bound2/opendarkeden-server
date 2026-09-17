//////////////////////////////////////////////////////////////////////////////
// Filename    : GameContext.cpp
// Description : Accessors for the managers registered on a GameContext.
//
//               The manager types stay incomplete here: an accessor only
//               binds a reference to the registered object, so nothing in
//               this file needs a manager's definition.
//////////////////////////////////////////////////////////////////////////////

#include "GameContext.h"

#include "Assert.h"

namespace de {

Properties& GameContext::config() const {
    Assert(m_pConfig != nullptr);
    return *m_pConfig;
}

DatabaseManager& GameContext::databases() const {
    Assert(m_pDatabaseManager != nullptr);
    return *m_pDatabaseManager;
}

ItemFactoryManager& GameContext::itemFactories() const {
    Assert(m_pItemFactoryManager != nullptr);
    return *m_pItemFactoryManager;
}

PCFinder& GameContext::playerCreatures() const {
    Assert(m_pPCFinder != nullptr);
    return *m_pPCFinder;
}

StringPool& GameContext::strings() const {
    Assert(m_pStringPool != nullptr);
    return *m_pStringPool;
}

VariableManager& GameContext::variables() const {
    Assert(m_pVariableManager != nullptr);
    return *m_pVariableManager;
}

ZoneGroupManager& GameContext::zoneGroups() const {
    Assert(m_pZoneGroupManager != nullptr);
    return *m_pZoneGroupManager;
}

ZoneInfoManager& GameContext::zoneInfos() const {
    Assert(m_pZoneInfoManager != nullptr);
    return *m_pZoneInfoManager;
}

GameContext& gameContext() {
    static GameContext context;
    return context;
}

} // namespace de
