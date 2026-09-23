//////////////////////////////////////////////////////////////////////////////
// Filename    : ServerContext.cpp
// Description : Accessors for the managers registered on a ServerContext.
//
//               The manager types stay incomplete here: an accessor only
//               binds a reference to the registered object, so nothing in
//               this file needs a manager's definition.
//////////////////////////////////////////////////////////////////////////////

#include "ServerContext.h"

#include "Assert.h"

namespace de {

DatabaseManager& ServerContext::database() const {
    Assert(m_pDatabaseManager != nullptr);
    return *m_pDatabaseManager;
}

GameServerInfoManager& ServerContext::serverInfos() const {
    Assert(m_pGameServerInfoManager != nullptr);
    return *m_pGameServerInfoManager;
}

GameWorldInfoManager& ServerContext::worldInfos() const {
    Assert(m_pGameWorldInfoManager != nullptr);
    return *m_pGameWorldInfoManager;
}

ServerContext& serverContext() {
    static ServerContext context;
    return context;
}

} // namespace de
