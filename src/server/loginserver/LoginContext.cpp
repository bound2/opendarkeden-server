//////////////////////////////////////////////////////////////////////////////
// Filename    : LoginContext.cpp
// Description : Accessors for the managers registered on a LoginContext.
//
//               The manager types stay incomplete here: an accessor only
//               binds a reference to the registered object, so nothing in
//               this file needs a manager's definition.
//////////////////////////////////////////////////////////////////////////////

#include "LoginContext.h"

#include "Assert.h"

namespace de {

GameServerGroupInfoManager& LoginContext::gameServerGroups() const {
    Assert(m_pGameServerGroupInfoManager != nullptr);
    return *m_pGameServerGroupInfoManager;
}

GameServerManager& LoginContext::gameServers() const {
    Assert(m_pGameServerManager != nullptr);
    return *m_pGameServerManager;
}

LoginPlayerManager& LoginContext::loginPlayers() const {
    Assert(m_pLoginPlayerManager != nullptr);
    return *m_pLoginPlayerManager;
}

ReconnectLoginInfoManager& LoginContext::reconnectLogins() const {
    Assert(m_pReconnectLoginInfoManager != nullptr);
    return *m_pReconnectLoginInfoManager;
}

UserInfoManager& LoginContext::userInfos() const {
    Assert(m_pUserInfoManager != nullptr);
    return *m_pUserInfoManager;
}

ZoneGroupInfoManager& LoginContext::zoneGroupInfos() const {
    Assert(m_pZoneGroupInfoManager != nullptr);
    return *m_pZoneGroupInfoManager;
}

ZoneInfoManager& LoginContext::zoneInfos() const {
    Assert(m_pZoneInfoManager != nullptr);
    return *m_pZoneInfoManager;
}

LoginContext& loginContext() {
    static LoginContext context;
    return context;
}

} // namespace de
