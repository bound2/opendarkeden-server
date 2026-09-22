//////////////////////////////////////////////////////////////////////////////
// Filename    : SharedContext.cpp
// Description : Accessors for the managers registered on a SharedContext.
//
//               The manager types stay incomplete here: an accessor only
//               binds a reference to the registered object, so nothing in
//               this file needs a manager's definition.
//////////////////////////////////////////////////////////////////////////////

#include "SharedContext.h"

#include "Assert.h"

namespace de {

GameServerManager& SharedContext::gameServers() const {
    Assert(m_pGameServerManager != nullptr);
    return *m_pGameServerManager;
}

GuildManager& SharedContext::guilds() const {
    Assert(m_pGuildManager != nullptr);
    return *m_pGuildManager;
}

StringPool& SharedContext::strings() const {
    Assert(m_pStringPool != nullptr);
    return *m_pStringPool;
}

SharedContext& sharedContext() {
    static SharedContext context;
    return context;
}

} // namespace de
