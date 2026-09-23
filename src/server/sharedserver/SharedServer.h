//////////////////////////////////////////////////////////////////////
//
// Filename    : SharedServer.h
// Written By  : reiot@ewestsoft.com
// Description : Main class for the login server
//
//////////////////////////////////////////////////////////////////////

#ifndef __SHARED_SERVER_H__
#define __SHARED_SERVER_H__

// Including this module makes it the login server module.
#ifndef __SHARED_SERVER__
#define __SHARED_SERVER__
#endif

// include files
#include "Exception.h"
#include "Types.h"

class GameServerGroupInfoManager;
class GameServerManager;
class GuildManager;
class HeartbeatManager;
class ResurrectLocationManager;
class SharedGameServerInfoManager;
class StringPool;

//////////////////////////////////////////////////////////////////////
//
// class SharedServer
//
// Class representing the login server itself.
//
//////////////////////////////////////////////////////////////////////

class SharedServer {
public:
    // constructor
    SharedServer();

    // destructor
    ~SharedServer() noexcept(false);

    // intialize game server
    void init();

    // start game server
    void start();

    // Request every worker to stop, then join them while the managers they
    // use are still alive. Idempotent: main and the startup catch both call it.
    void stop();

private:
    bool m_Stopped = false;

    // The managers the shared server owns. Each is registered on
    // de::sharedContext() as it is created, except the four nothing outside
    // this class reads.
    GuildManager* m_pGuildManager = nullptr;
    SharedGameServerInfoManager* m_pGameServerInfoManager = nullptr;
    GameServerGroupInfoManager* m_pGameServerGroupInfoManager = nullptr;
    GameServerManager* m_pGameServerManager = nullptr;
    HeartbeatManager* m_pHeartbeatManager = nullptr;
    ResurrectLocationManager* m_pResurrectLocationManager = nullptr;
    StringPool* m_pStringPool = nullptr;
};

#endif
