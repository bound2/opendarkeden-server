//////////////////////////////////////////////////////////////////////////////
// Filename    : GameServer.h
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GAME_SERVER_H__
#define __GAME_SERVER_H__

#include "Exception.h"
#include "Properties.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GameServer
//////////////////////////////////////////////////////////////////////////////

class ClientManager;
class ObjectManager;
class SharedServerManager;
class ThreadManager;

class GameServer {
public:
    GameServer();
    ~GameServer();

public:
    void init();

    void start();

    void stop();

private:
    bool m_Stopped = false; // Lifecycle operations are owned by the main thread.
    void sysinit();
    void goBackground();

    // Managers this class creates and deletes. The client manager and the
    // shared server link are registered on de::GameContext; the other two
    // are reached only from here.
    ClientManager* m_pClientManager = nullptr;
    ObjectManager* m_pObjectManager = nullptr;
    SharedServerManager* m_pSharedServerManager = nullptr;
    ThreadManager* m_pThreadManager = nullptr;
};

#endif
