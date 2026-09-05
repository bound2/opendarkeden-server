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
};

// global variable
extern GameServer* g_pGameServer;

#endif
