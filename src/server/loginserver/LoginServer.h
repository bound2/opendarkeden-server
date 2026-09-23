//////////////////////////////////////////////////////////////////////
//
// Filename    : LoginServer.h
// Written By  : reiot@ewestsoft.com
// Description : Main class for the login server
//
//////////////////////////////////////////////////////////////////////

#ifndef __LOGIN_SERVER_H__
#define __LOGIN_SERVER_H__

// Including this module makes it the login server module.
#ifndef __LOGIN_SERVER__
#define __LOGIN_SERVER__
#endif

// include files
#include "Exception.h"
#include "Types.h"

class ClientManager;
class GameServerGroupInfoManager;
class GameServerManager;
class ItemDestroyer;
class PacketFactoryManager;
class PacketValidator;
class UserInfoManager;
class ZoneGroupInfoManager;
class ZoneInfoManager;

//////////////////////////////////////////////////////////////////////
//
// class LoginServer
//
// Class representing the login server itself.
//
//////////////////////////////////////////////////////////////////////

class LoginServer {
public:
    // constructor
    LoginServer();

    // destructor
    ~LoginServer() noexcept(false);

    // intialize game server
    void init();

    // start game server
    void start();

    // Request every worker to stop, then join them while the managers they
    // use are still alive. Idempotent: main and the startup catch both call it.
    void stop();

private:
    bool m_Stopped = false;

    // The managers the login server owns. Each is registered on
    // de::loginContext() as it is created, except the two nothing outside
    // this class reads and the packet factory table and the validator, which
    // go on de::KernelContext because every binary fills it with a set of
    // its own.
    GameServerGroupInfoManager* m_pGameServerGroupInfoManager = nullptr;
    ZoneInfoManager* m_pZoneInfoManager = nullptr;
    ZoneGroupInfoManager* m_pZoneGroupInfoManager = nullptr;
    GameServerManager* m_pGameServerManager = nullptr;
    ClientManager* m_pClientManager = nullptr;
    ItemDestroyer* m_pItemDestroyer = nullptr;
    UserInfoManager* m_pUserInfoManager = nullptr;
    PacketFactoryManager* m_pPacketFactoryManager = nullptr;
    PacketValidator* m_pPacketValidator = nullptr;
};

#endif
